// This was made from Valve's skeleton NPC

#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "activitylist.h"
#include "ai_basenpc.h"
#include "engine/IEngineSound.h"
#include "ai_senses.h"
#include "neo_gamerules.h"
#include "weapon_neobasecombatweapon.h"
#include "weapon_ghost.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// --- This stuff is pretty useless for the Dummy. But it seems like its required
int	ACT_DUMMY = -1;

enum
{
	SCHED_MYCUSTOMSCHEDULE = LAST_SHARED_SCHEDULE,
};

enum
{
	TASK_MYCUSTOMTASK = LAST_SHARED_TASK,
};

enum
{
	COND_MYCUSTOMCONDITION = LAST_SHARED_CONDITION,
};

class CNEO_NPCDummy : public CAI_BaseNPC
{
	DECLARE_CLASS(CNEO_NPCDummy, CAI_BaseNPC);
	DECLARE_SERVERCLASS();

public:
	virtual void			Precache(void) override;
	virtual void			Spawn(void) override;
	virtual Class_T			Classify(void) override;
	virtual int				ShouldTransmit(const CCheckTransmitInfo* pInfo) override;
	virtual int				UpdateTransmitState() override;

	DECLARE_DATADESC();

	DEFINE_CUSTOM_AI;

private:
	int m_iKeyHealth;
	string_t m_strKeyModel;
	string_t m_strKeyWeaponModel;
	bool m_bKeyDistortEffect;

	void WeaponBonemerge(const char* weaponModelDir);
};

IMPLEMENT_SERVERCLASS_ST(CNEO_NPCDummy, DT_NEO_NPCDummy)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(neo_npc_dummy, CNEO_NPCDummy);
IMPLEMENT_CUSTOM_AI(npc_citizen, CNEO_NPCDummy);

BEGIN_DATADESC(CNEO_NPCDummy)
	DEFINE_KEYFIELD(m_iKeyHealth, FIELD_INTEGER, "Health"),
	DEFINE_KEYFIELD(m_strKeyModel, FIELD_STRING, "Model"),
	DEFINE_KEYFIELD(m_strKeyWeaponModel, FIELD_STRING, "Weapon"),
	DEFINE_KEYFIELD(m_bKeyDistortEffect, FIELD_BOOLEAN, "DistortEffect"),
END_DATADESC()

// -- This is also useless
void CNEO_NPCDummy::InitCustomSchedules(void)
{
	INIT_CUSTOM_AI(CNEO_NPCDummy);

	ADD_CUSTOM_TASK(CNEO_NPCDummy, TASK_MYCUSTOMTASK);

	ADD_CUSTOM_SCHEDULE(CNEO_NPCDummy, SCHED_MYCUSTOMSCHEDULE);

	ADD_CUSTOM_ACTIVITY(CNEO_NPCDummy, ACT_DUMMY);

	ADD_CUSTOM_CONDITION(CNEO_NPCDummy, COND_MYCUSTOMCONDITION);
}
// ---

void CNEO_NPCDummy::Precache(void)
{
	if (m_strKeyModel != NULL_STRING)
	{
		PrecacheModel(STRING(m_strKeyModel));
	}

	if (m_strKeyWeaponModel != NULL_STRING)
	{
		PrecacheModel(STRING(m_strKeyWeaponModel));
	}

	BaseClass::Precache();
}

void CNEO_NPCDummy::Spawn(void)
{
	Precache();

	if (m_strKeyModel == NULL_STRING || STRING(m_strKeyModel)[0] == '\0')
	{
		Warning("You need to specify a model, or else the dummy will not spawn.\n");
		UTIL_Remove(this);
		return;
	}

	SetModel(STRING(m_strKeyModel));

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(DONT_BLEED);

	GetSenses()->AddSensingFlags(SENSING_FLAGS_DONT_LOOK | SENSING_FLAGS_DONT_LISTEN); // The dummy is deaf and blind

	m_iHealth = (m_iKeyHealth > 0) ? m_iKeyHealth : 20;

	m_flFieldOfView = 0.5;
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesClear();
	// CapabilitiesAdd( bits_CAP_NONE );

	if (m_strKeyWeaponModel != NULL_STRING)
	{
		WeaponBonemerge(STRING(m_strKeyWeaponModel));
	}

	NPCInit();
}

void CNEO_NPCDummy::WeaponBonemerge(const char* weaponModelDir)
{
	CBaseEntity* pWeaponModel = CreateEntityByName("prop_dynamic");
	if (pWeaponModel)
	{
		pWeaponModel->SetModel(weaponModelDir);

		pWeaponModel->SetParent(this);
		pWeaponModel->AddEffects(EF_BONEMERGE);
		pWeaponModel->AddEffects(EF_BONEMERGE_FASTCULL);
		// Rendering issue with bonemerge. When added, weapons will be visible through invisible surfaces that seal the map, like skybox or nodraw.
		// Not a big problem, but I can't figure out why it's happening. They should inherit the visibility of their parent

		if (m_bKeyDistortEffect)
		{
			pWeaponModel->SetRenderMode(kRenderTransTexture);
			pWeaponModel->m_nRenderFX = kRenderFxDistort;
		}
	}
}

// Purpose: Good question
Class_T	CNEO_NPCDummy::Classify(void)
{
	return	CLASS_NONE;
}

int CNEO_NPCDummy::ShouldTransmit(const CCheckTransmitInfo* pInfo)
{
	if (pInfo)
	{
		auto otherNeoPlayer = assert_cast<CNEO_Player*>(Instance(pInfo->m_pClientEnt));
		// If other is spectator or same team
		if (otherNeoPlayer->GetTeamNumber() == TEAM_SPECTATOR ||
#ifdef GLOWS_ENABLE
			otherNeoPlayer->IsDead() || // ...or dead spec, for xray purposes
#endif
			GetTeamNumber() == otherNeoPlayer->GetTeamNumber())
		{
			return FL_EDICT_ALWAYS;
		}

		// Ghoster should receive beacons regardless of PVS
		auto otherWep = static_cast<CNEOBaseCombatWeapon*>(otherNeoPlayer->GetActiveWeapon());
		if (otherWep && otherWep->IsGhost() &&
			static_cast<CWeaponGhost*>(otherWep)->IsPosWithinViewDistance(GetAbsOrigin()))
		{
			return FL_EDICT_ALWAYS;
		}

		// NEO TODO (Adam) Check if weapon attached to this npc is of type weapon_ghost? can we draw more than one ghost beacon at once?
	}

	return BaseClass::ShouldTransmit(pInfo);
}

int CNEO_NPCDummy::UpdateTransmitState()
{
	return FL_EDICT_FULLCHECK;
}
