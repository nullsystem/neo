#ifndef NEO_WEAPON_JITTE_S_H
#define NEO_WEAPON_JITTE_S_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_neo_player.h"
#else
#include "neo_player.h"
#endif

#include "weapon_neobasecombatweapon.h"

#ifdef CLIENT_DLL
#define CWeaponJitteS C_WeaponJitteS
#endif

class CWeaponJitteS : public CNEOBaseCombatWeapon
{
	DECLARE_CLASS(CWeaponJitteS, CNEOBaseCombatWeapon);
public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifdef GAME_DLL
	DECLARE_ACTTABLE();
#endif

	CWeaponJitteS();

	virtual NEO_WEP_BITS_UNDERLYING_TYPE GetNeoWepBits(void) const OVERRIDE { return NEO_WEP_JITTE | NEO_WEP_SUPPRESSED; }
	virtual int GetNeoWepXPCost(const int neoClass) const OVERRIDE { return 0; }

	virtual float GetSpeedScale(void) const OVERRIDE { return 0.8f; }

protected:
	virtual float GetFastestDryRefireTime() const OVERRIDE { return 0.2f; }

private:
	CWeaponJitteS(const CWeaponJitteS &other);
};

#endif // NEO_WEAPON_JITTE_S_H
