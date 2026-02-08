#include "neo_root_gamehud.h"

#include "neo_hud_ammo.h"
#include "neo_hud_health_thermoptic_aux.h"

// game/neo/scripts/HudLayout.res
// void EditablePanel::LoadUserConfig(const char *configName, int dialogID)
// void EditablePanel::SaveUserConfig()

void NeoRootGamehud_Init(NeoRootGamehud *gamehud)
{
	gamehud->ammo = GET_NAMED_HUDELEMENT(CNEOHud_Ammo, NHudWeapon);
	gamehud->hta = GET_NAMED_HUDELEMENT(CNEOHud_HTA, NHudHealth);
}

void NeoRootGamehud_Paint(NeoRootGamehud *gamehud)
{
	//gamehud->ammo->Paint();
	gamehud->hta->DrawHTA();
}

