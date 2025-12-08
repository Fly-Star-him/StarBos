#include <Ext/Scenario/Body.h>
#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

DEFINE_HOOK(0x6870D7, ReadScenario_MissionINI, 0x5)
{
	enum { SkipGameCode = 0x6873AB };

	LEA_STACK(CCINIClass*, pINI, STACK_OFFSET(0x174, -0x158));

	auto const pScenario = ScenarioClass::Instance;
	auto const pScenarioExt = ScenarioExt::Global();
	auto const scenarioName = pScenario->FileName;
	auto const defaultsSection = "Defaults";

	CCINIClass ini_missionmd {};
	ini_missionmd.LoadFromFile(GameStrings::MISSIONMD_INI);

	pScenarioExt->DefaultLS640BkgdName.Read(&ini_missionmd, defaultsSection, "DefaultLS640BkgdName");
	pScenarioExt->DefaultLS800BkgdName.Read(&ini_missionmd, defaultsSection, "DefaultLS800BkgdName");
	pScenarioExt->DefaultLS800BkgdPal.Read(&ini_missionmd, defaultsSection, "DefaultLS800BkgdPal");

	pScenarioExt->ShowBriefing = pINI->ReadBool(scenarioName, "ShowBriefing", pScenarioExt->ShowBriefing);
	pScenarioExt->BriefingTheme = pINI->ReadTheme(scenarioName, "BriefingTheme", pScenarioExt->BriefingTheme);

	pScenario->LS640BriefLocX = pINI->ReadInteger(scenarioName, "LS640BriefLocX", ini_missionmd.ReadInteger(defaultsSection, "DefaultLS640BriefLocX", 0));
	pScenario->LS640BriefLocY = pINI->ReadInteger(scenarioName, "LS640BriefLocY", ini_missionmd.ReadInteger(defaultsSection, "DefaultLS640BriefLocY", 0));
	pScenario->LS800BriefLocX = pINI->ReadInteger(scenarioName, "LS800BriefLocX", ini_missionmd.ReadInteger(defaultsSection, "DefaultLS800BriefLocX", 0));
	pScenario->LS800BriefLocY = pINI->ReadInteger(scenarioName, "LS800BriefLocY", ini_missionmd.ReadInteger(defaultsSection, "DefaultLS800BriefLocY", 0));

	pINI->ReadString(scenarioName, "LS640BkgdName", pScenarioExt->DefaultLS640BkgdName, pScenario->LS640BkgdName, 64);
	pINI->ReadString(scenarioName, "LS800BkgdName", pScenarioExt->DefaultLS800BkgdName, pScenario->LS800BkgdName, 64);
	pINI->ReadString(scenarioName, "LS800BkgdPal", pScenarioExt->DefaultLS800BkgdPal, pScenario->LS800BkgdPal, 64);

	return SkipGameCode;
}

#pragma region PlayerAtX

// Map <Player @ X> as object owner name to correct HouseClass index.
DEFINE_HOOK(0x50C186, GetHouseIndexFromName_PlayerAtX, 0x6)
{
	enum { ReturnFromFunction = 0x50C203 };

	GET(const char*, name, ECX);

	// Bail out early in campaign mode or if the name does not start with <
	if (SessionClass::IsCampaign() || *name != '<')
		return 0;

	const int playerAtIndex = HouseClass::GetPlayerAtFromString(name);

	if (playerAtIndex != -1)
	{
		auto const pHouse = HouseClass::FindByPlayerAt(playerAtIndex);

		if (pHouse)
		{
			R->EDX(pHouse->ArrayIndex);
			return ReturnFromFunction;
		}
	}

	return 0;
}

// Skip check that prevents buildings from being created for local player.
DEFINE_JUMP(LJMP, 0x44F8D5, 0x44F8E1);

#pragma endregion

// score options
// score music for single player missions
DEFINE_HOOK(0x6C924F, ScoreDialog_Handle_ScoreThemeA, 0x5)
{
	GET(char*, pTitle, ECX);
	GET(char*, pMessage, ESI);
	CSFText& Title = ScenarioExt::Global()->ParTitle;
	CSFText& Message = ScenarioExt::Global()->ParMessage;

	strcpy(pTitle, Title.Label);
	strcpy(pMessage, Message.Label);

	return 0;
}

DEFINE_HOOK(0x6C935C, ScoreDialog_Handle_ScoreThemeB, 0x5)
{
	REF_STACK(char*, pTheme, 0x0);

	const char* theme = ScenarioExt::Global()->ScoreCampaignTheme.data();

	if (strcmp(theme, "") && strcmp(theme, "0"))
		strcpy(pTheme, theme);

	return 0;
}

DEFINE_HOOK(0x5AE192, SelectNextMission, 0x6)
{
	const char* nextMission = ScenarioExt::Global()->NextMission.data();

	if (strcmp(nextMission, "") && strcmp(nextMission, "0"))
		R->EAX(nextMission);

	return 0;
}

DEFINE_HOOK(0x689EB0, ScenarioClass_ReadMap_SkipHeader, 0x6)
{
	if (SessionClass::Instance.IsCampaign())
		return 0x689FC0;

	return 0;
}
