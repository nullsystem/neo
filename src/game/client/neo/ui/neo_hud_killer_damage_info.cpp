#include <cbase.h>
#include "neo_hud_killer_damage_info.h"

#include "iclientmode.h"

#include "c_neo_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_NAMED_HUDELEMENT(CNEOHud_KillerDamageInfo, neo_killer_damage_info)

NEO_HUD_ELEMENT_DECLARE_FREQ_CVAR(KillerDamageInfo, 0.1)

CNEOHud_KillerDamageInfo::CNEOHud_KillerDamageInfo(const char *pszName, vgui::Panel *parent)
	: CHudElement(pszName)
	, vgui::Panel(parent, pszName)
{
	SetAutoDelete(true);

	if (parent)
	{
		SetParent(parent);
	}
	else
	{
		SetParent(g_pClientMode->GetViewport());
	}

	int wide, tall;
	vgui::surface()->GetScreenSize(wide, tall);
	SetBounds(0, 0, wide, tall);

	SetVisible(false);

	SetFgColor(COLOR_TRANSPARENT);
	SetBgColor(COLOR_TRANSPARENT);
}

void CNEOHud_KillerDamageInfo::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	vgui::Panel::ApplySchemeSettings(pScheme);

	m_hFont = pScheme->GetFont("NHudOCRSmall", true);

	int wide, tall;
	vgui::surface()->GetScreenSize(wide, tall);
	SetBounds(0, 0, wide, tall);

	SetFgColor(COLOR_TRANSPARENT);
	SetBgColor(COLOR_TRANSPARENT);
}

void CNEOHud_KillerDamageInfo::resetHUDState()
{
	V_memset(&g_neoKDmgInfos, 0, sizeof(CNEOKillerDamageInfos));
}

void CNEOHud_KillerDamageInfo::UpdateStateForNeoHudElementDraw()
{
	// no-op?
}

void CNEOHud_KillerDamageInfo::DrawNeoHudElement()
{
	const C_NEO_Player *localPlayer = C_NEO_Player::GetLocalNEOPlayer();
	const bool bPlayerShownHud = ShouldDraw() && localPlayer
			&& const_cast<C_NEO_Player *>(localPlayer)->IsPlayerDead()
			&& (localPlayer->GetTeamNumber() == TEAM_JINRAI ||
				localPlayer->GetTeamNumber() == TEAM_NSF)
			&& (g_neoKDmgInfos.killerInfo.iEntIndex > 0);
	if (!bPlayerShownHud)
	{
		return;
	}

	int iScrWide, iScrTall;
	vgui::surface()->GetScreenSize(iScrWide, iScrTall);

	// Show info box while it just died spectating itself
	if (g_neoKDmgInfos.killerInfo.iEntIndex > 0 &&
			(!localPlayer->GetObserverTarget() || localPlayer->GetObserverTarget() == localPlayer))
	{
		vgui::IntRect infoRect = {
			.x0 = (iScrWide / 3),
			.y0 = iScrTall - ((iScrTall / 4) * 2),
			.x1 = iScrWide - (iScrWide / 3),
			.y1 = iScrTall - (iScrTall / 4),
		};
		int infoWide = infoRect.x1 - infoRect.x0;
		int infoTall = infoRect.y1 - infoRect.y0;

		vgui::surface()->DrawSetColor(COLOR_FADED_DARK);
		vgui::surface()->DrawFilledRectArray(&infoRect, 1);
		vgui::surface()->DrawSetTextPos(infoRect.x0 + (infoWide / 2), infoRect.y0 + (infoTall / 2));

		// Show a suicide/self kill if it matches the local player's entity index
		if (g_neoKDmgInfos.killerInfo.iEntIndex == localPlayer->entindex())
		{
			static constexpr wchar_t SELF_KILL_TEXT[] = L"YOU KILLED YOUSELF";
			vgui::surface()->DrawSetTextColor(COLOR_RED);
			vgui::surface()->DrawPrintText(SELF_KILL_TEXT, ARRAYSIZE(SELF_KILL_TEXT) - 1);
		}
		else
		{
			vgui::surface()->DrawSetTextColor(COLOR_WHITE);
			vgui::surface()->DrawPrintText(g_neoKDmgInfos.wszKillerName, g_neoKDmgInfos.iKillerNameSize);
		}
	}
}

void CNEOHud_KillerDamageInfo::Paint()
{
	SetFgColor(COLOR_TRANSPARENT);
	SetBgColor(COLOR_TRANSPARENT);
	vgui::Panel::Paint();
	PaintNeoElement();
}
