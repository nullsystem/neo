#include "cbase.h"
#include "neo_hud_ammo.h"

#include "c_neo_player.h"

#include "iclientmode.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/IScheme.h>

#include <engine/ivdebugoverlay.h>
#include "ienginevgui.h"

#include "ammodef.h"

#include "weapon_ghost.h"
#include "weapon_grenade.h"
#include "weapon_neobasecombatweapon.h"
#include "weapon_smokegrenade.h"
#include "weapon_supa7.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using vgui::surface;

ConVar cl_neo_hud_ammo_enabled("cl_neo_hud_ammo_enabled", "1", FCVAR_ARCHIVE,
	"Whether the HUD ammo is enabled or not.", true, 0, true, 1);

DECLARE_NAMED_HUDELEMENT(CNEOHud_Ammo, NHudWeapon);

NEO_HUD_ELEMENT_DECLARE_FREQ_CVAR(Ammo, 0.00695);

CNEOHud_Ammo::CNEOHud_Ammo()
	: CHudElement("NHudWeapon"), EditablePanel(nullptr, "NHudWeapon")
{
	SetAutoDelete(false);
	vgui::surface()->GetScreenSize(m_resX, m_resY);
	SetBounds(0, 0, m_resX, m_resY);
	SetVisible(true);
}

CNEOHud_Ammo::CNEOHud_Ammo(const char* pElementName, vgui::Panel* parent)
	: CHudElement(pElementName), EditablePanel(parent, pElementName)
{
	SetAutoDelete(true);
	m_iHideHudElementNumber = NEO_HUD_ELEMENT_AMMO;

	if (parent)
	{
		SetParent(parent);
	}
	else
	{
		SetParent(g_pClientMode->GetViewport());
	}

	surface()->GetScreenSize(m_resX, m_resY);
	SetBounds(0, 0, m_resX, m_resY);

	SetVisible(cl_neo_hud_ammo_enabled.GetBool());

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
}

void CNEOHud_Ammo::Paint()
{
	PaintNeoElement();
	BaseClass::Paint();
}

void CNEOHud_Ammo::UpdateStateForNeoHudElementDraw()
{
	Assert(C_NEO_Player::GetLocalNEOPlayer());
}

void CNEOHud_Ammo::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("scripts/HudLayout.res");
	LoadControlSettings("scripts/UserHudLayout.res");

	m_hSmallTextFont = pScheme->GetFont("NHudOCRSmall");
	m_hBulletFont = pScheme->GetFont("NHudBullets");
	m_hTextFont = pScheme->GetFont("NHudOCR");

	surface()->GetScreenSize(m_resX, m_resY);
	SetBounds(0, 0, m_resX, m_resY);
	SetFgColor(COLOR_TRANSPARENT);
	SetBgColor(COLOR_TRANSPARENT);
}

void CNEOHud_Ammo::DrawAmmo() const
{
	Assert(C_NEO_Player::GetLocalNEOPlayer());
	C_NEOBaseCombatWeapon *activeWep = dynamic_cast<C_NEOBaseCombatWeapon *>(
			C_NEO_Player::GetLocalNEOPlayer()->GetActiveWeapon());
	if (activeWep)
	{
		const WeaponInfos activeWepInfos = {
			.pszPrintName = activeWep->GetPrintName(),
			.pszBulletChar = activeWep->GetNEOWpnData().szBulletCharacter,
			.wepBits = activeWep->GetNeoWepBits(),
			.bMelee = activeWep->IsMeleeWeapon(),
			.bAutomatic = activeWep->IsAutomatic(),
			.bUsesClipsForAmmo1 = activeWep->UsesClipsForAmmo1(),
			.bSlugLoaded = (activeWep->GetNeoWepBits() & NEO_WEP_SUPA7)
					? static_cast<CWeaponSupa7 *>(activeWep)->SlugLoaded()
					: false,
			.iPrimaryAmmoCount = activeWep->m_iPrimaryAmmoCount,
			.iSecondaryAmmoCount = activeWep->m_iSecondaryAmmoCount,
			.iMaxClip1 = activeWep->GetMaxClip1(),
			.iDefaultClip1 = activeWep->GetDefaultClip1(),
			.iClip1 = activeWep->Clip1(),
		};
		MainDraw(activeWepInfos);
	}
}

void CNEOHud_Ammo::DrawNeoHudElement()
{
	auto *localPlayer = C_NEO_Player::GetLocalNEOPlayer();
	if (!ShouldDraw() || (!localPlayer || localPlayer->IsObserver()))
	{
		return;
	}

	if (cl_neo_hud_ammo_enabled.GetBool())
	{
		DrawAmmo();
	}
}

void CNEOHud_Ammo::DrawHeatMeter(const WeaponInfos &activeWepInfos) const
{
	float flHeatAmount = (1.0f - (activeWepInfos.iPrimaryAmmoCount / (float)activeWepInfos.iDefaultClip1));
	Color heatColorLerp = LerpColor(ammo_color,heat_color, flHeatAmount);
	
	if (activeWepInfos.iPrimaryAmmoCount == 0)
	{
		surface()->DrawSetTextFont(m_hSmallTextFont);
		surface()->DrawSetTextPos(heatbar_xpos + xpos, (heatbar_ypos + ypos) - 22);
		surface()->DrawPrintText(L"OVERHEAT", 8);
	}

	surface()->DrawSetColor(heatColorLerp);
	surface()->DrawFilledRect(
		heatbar_xpos + xpos,
		heatbar_ypos + ypos,
		heatbar_xpos + xpos + (heatbar_w * flHeatAmount),
		heatbar_ypos + ypos + heatbar_h);

	surface()->DrawSetColor(ammo_text_color);
	surface()->DrawOutlinedRect(
		heatbar_xpos + xpos,
		heatbar_ypos + ypos,
		heatbar_xpos + xpos + heatbar_w,
		heatbar_ypos + ypos + heatbar_h);
}

void CNEOHud_Ammo::MainDraw(const WeaponInfos &activeWepInfos) const
{
	wchar_t wszWepName[64] = {};
	Q_UTF8ToUnicode(activeWepInfos.pszPrintName, wszWepName, sizeof(wszWepName));
	V_wcsupr(wszWepName);

	DrawNeoHudRoundedBox(xpos, ypos, xpos + wide, ypos + tall,
			box_color,
			top_left_corner, top_right_corner, bottom_left_corner, bottom_right_corner);

	vgui::surface()->DrawSetTextFont(m_hSmallTextFont);
	vgui::surface()->DrawSetTextColor(ammo_text_color);
	int fontWidth, fontHeight;
	vgui::surface()->GetTextSize(m_hSmallTextFont, wszWepName, fontWidth, fontHeight);
	vgui::surface()->DrawSetTextPos((text_xpos + xpos) - fontWidth, text_ypos + ypos);
	vgui::surface()->DrawPrintText(wszWepName, V_wcslen(wszWepName));

	if ((activeWepInfos.wepBits & NEO_WEP_GHOST)
			|| activeWepInfos.bMelee
			|| activeWepInfos.iMaxClip1 == 0)
	{
		return;
	}

	const int ammoCount = activeWepInfos.iPrimaryAmmoCount;
	// abs because grenades return negative values (???)
	// casting division to float in case we have a half-empty mag, rounding up
	// to show the half mag as one more mag
	const int numClips = ceil(abs((float)ammoCount / activeWepInfos.iMaxClip1));
	const bool isSupa = activeWepInfos.wepBits & NEO_WEP_SUPA7;
		
	if (activeWepInfos.bUsesClipsForAmmo1 && !(activeWepInfos.wepBits & NEO_WEP_DETPACK))
	{
		wchar_t wszClipsText[5] = {};
		if (isSupa)
		{
			V_swprintf_safe(wszClipsText, L"%d+%d",
					ammoCount, activeWepInfos.iSecondaryAmmoCount);
		}
		else
		{
			V_swprintf_safe(wszClipsText, L"%d",
					numClips);
		}

		surface()->DrawSetTextFont(m_hTextFont);
		surface()->GetTextSize(m_hTextFont, wszClipsText, fontWidth, fontHeight);
		surface()->DrawSetTextPos(digit2_xpos + xpos - fontWidth, digit2_ypos + ypos);
		surface()->DrawPrintText(wszClipsText, V_wcslen(wszClipsText));
	}

	const char *ammoChar = nullptr;
	int magSizeMax = 0;
	int magSizeCurrent = 0;
		
	if ((activeWepInfos.bUsesClipsForAmmo1 && !(activeWepInfos.wepBits & NEO_WEP_THROWABLE)) || (activeWepInfos.wepBits & NEO_WEP_BALC))
	{
		wchar_t wszFireMode[2] = {};

		ammoChar = activeWepInfos.pszBulletChar;
		magSizeMax = activeWepInfos.iMaxClip1;
		magSizeCurrent = activeWepInfos.iClip1;
			
		if (activeWepInfos.bAutomatic)
		{
			wszFireMode[0] = L'j';
		}
		else if (isSupa)
		{
			if (activeWepInfos.bSlugLoaded)
			{
				wszFireMode[0] = L'h';
			}
			else
			{
				wszFireMode[0] = L'l';
			}
		}
		else
		{
			wszFireMode[0] = L'h';
		}

		vgui::surface()->DrawSetTextFont(m_hBulletFont);
		vgui::surface()->DrawSetTextPos(icon_xpos + xpos, icon_ypos + ypos);
		vgui::surface()->DrawPrintText(wszFireMode, V_wcslen(wszFireMode));
	}
	else 
	{
		if (activeWepInfos.wepBits & NEO_WEP_SMOKE_GRENADE)
		{
			ammoChar = "f";
			magSizeMax = magSizeCurrent = ammoCount;
		}
		else if (activeWepInfos.wepBits & NEO_WEP_FRAG_GRENADE)
		{
			ammoChar = "g";
			magSizeMax = magSizeCurrent = ammoCount;
		}			
	}

	if (activeWepInfos.wepBits & NEO_WEP_BALC)
	{
		DrawHeatMeter(activeWepInfos);
		return;
	}

	vgui::surface()->DrawSetTextColor(ammo_color);
	// Draw bullets in magazine in number form
	if (digit_as_number && activeWepInfos.bUsesClipsForAmmo1)
	{
		surface()->DrawSetTextFont(m_hBulletFont);
		surface()->DrawSetTextPos(digit_xpos + xpos, digit_ypos + ypos);
		wchar_t bullets[22];
		V_swprintf_safe(bullets, L"%i/%i", magSizeCurrent, magSizeMax);
		surface()->DrawPrintText(bullets, (int)(magSizeCurrent == 0 ? 1 : log10(magSizeCurrent) + 1) + (int)(log10(magSizeMax) + 1) + 1);
		return;
	}

	if (ammoChar == nullptr)
	{
		return;
	}

	const int maxSpaceAvailableForBullets = digit_max_width;
	const int bulletWidth = surface()->GetCharacterWidth(m_hBulletFont, *ammoChar);
	const int plusWidth = surface()->GetCharacterWidth(m_hBulletFont, '+');
	const int maxBulletsWeCanDisplay = bulletWidth == 0 ? 0 : (maxSpaceAvailableForBullets / bulletWidth);

	if (maxBulletsWeCanDisplay == 0)
	{
		return;
	}

	const int maxBulletsWeCanDisplayWithPlus = bulletWidth == 0 ? 0 : ((maxSpaceAvailableForBullets - plusWidth) / bulletWidth);
	const bool bulletsOverflowing = maxBulletsWeCanDisplay < magSizeMax;

	if (bulletsOverflowing)
	{
		magSizeMax = maxBulletsWeCanDisplayWithPlus + 1;
	}

	constexpr auto maxBullets = 100; // PZ Mag Size

	char bullets[maxBullets + 1];
	magSizeMax = Min(magSizeMax, narrow_cast<int>(sizeof(bullets) - 1));
	int i;
	for (i = 0; i < magSizeMax; i++)
	{
		bullets[i] = *ammoChar;
	}
	bullets[i] = '\0';

	int magAmountToDrawFilled = magSizeCurrent;
		
	if (bulletsOverflowing)
	{
		if (magSizeMax > 0)
		{
			bullets[magSizeMax - 1] = '+';
		}

		if (activeWepInfos.iMaxClip1 == magSizeCurrent)
		{
			magAmountToDrawFilled = magSizeMax;
		}
		else if (magSizeMax - 1 < magSizeCurrent)
		{
			magAmountToDrawFilled = magSizeMax - 1;
		}
		else
		{
			magAmountToDrawFilled = magSizeCurrent;
		}
	}
		
	wchar_t wszBullets[maxBullets + 1];
	Q_UTF8ToUnicode(bullets, wszBullets, sizeof(wszBullets));
		
	if (magAmountToDrawFilled > 0)
	{
		vgui::surface()->DrawSetTextFont(m_hBulletFont);
		vgui::surface()->DrawSetTextPos(digit_xpos + xpos, digit_ypos + ypos);
		vgui::surface()->DrawPrintText(wszBullets, magAmountToDrawFilled);
	}

	if (activeWepInfos.iMaxClip1 > 0)
	{
		if (magSizeMax > 0)
		{
			vgui::surface()->DrawSetTextColor(emptied_ammo_color);
			vgui::surface()->DrawSetTextPos(digit_xpos + xpos + (bulletWidth * magAmountToDrawFilled), digit_ypos + ypos);
			vgui::surface()->DrawPrintText(&wszBullets[magAmountToDrawFilled], magSizeMax - magAmountToDrawFilled);
		}
	}
}

