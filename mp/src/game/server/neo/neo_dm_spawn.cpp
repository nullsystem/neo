#include "neo_dm_spawn.h"

#include <dbg.h>
#include "cbase.h"
#include "inetchannelinfo.h"
#include "neo_player.h"
#include "neo_misc.h"
#include "util_shared.h"
#include "filesystem.h"
#include "utlbuffer.h"

#include "tier0/memdbgon.h"

static inline CUtlVector<Vector> gDMSpawnLocs;
static inline int gDMSpawnCur = 0;

namespace DMSpawn
{
bool HasDMSpawn()
{
	return !gDMSpawnLocs.IsEmpty();
}

Vector GiveNextSpawn()
{
	if (gDMSpawnLocs.IsEmpty()) return Vector{};
	gDMSpawnCur = LoopAroundInArray(gDMSpawnCur + 1, gDMSpawnLocs.Size());
	return gDMSpawnLocs[gDMSpawnCur];
}
}

static CNEO_Player *LoopbackPlayer()
{
	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		auto *pPlayer = static_cast<CNEO_Player *>(UTIL_PlayerByIndex(i));
		if (!pPlayer || pPlayer->IsBot())
		{
			continue;
		}
		INetChannelInfo *nci = engine->GetPlayerNetInfo(i);
		if (nci->IsLoopback())
		{
			return pPlayer;
		}
	}
	return nullptr;
}

static void DMSpawnComCallbackCreate()
{
	if (auto *player = LoopbackPlayer())
	{
		const Vector newLoc = player->GetAbsOrigin();
		bool bAlreadyApplied = false;
		for (const auto &loc : gDMSpawnLocs)
		{
			if (loc == newLoc)
			{
				bAlreadyApplied = true;
				break;
			}
		}
		if (bAlreadyApplied)
		{
			Msg("DM Spawn already applied\n");
		}
		else
		{
			gDMSpawnLocs.AddToTail(newLoc);
			Msg("DM Spawn: %.2f %.2f %.2f created\n", newLoc.x, newLoc.y, newLoc.z);
		}
	}
}

static void DMSpawnComCallbackRemoveAll()
{
	gDMSpawnLocs.RemoveAll();
}

static void DMSpawnComCallbackPrintLocs()
{
	for (const auto &loc : gDMSpawnLocs)
	{
		Msg("DM Spawn XYZ: %.2f %.2f %.2f\n", loc.x, loc.y, loc.z);
	}
}

static constexpr unsigned int MAGIC_NUMBER = 0x15AF73DA;
static constexpr int DMSPAWN_SERIAL_CURRENT = 1;

static void DMSpawnComCallbackSave(const CCommand &command)
{
	if (gDMSpawnLocs.IsEmpty())
	{
		return;
	}

	CUtlBuffer buf;
	buf.PutUnsignedInt(MAGIC_NUMBER);
	buf.PutInt(DMSPAWN_SERIAL_CURRENT);
	buf.PutInt(gDMSpawnLocs.Size());
	for (const Vector &vec : gDMSpawnLocs)
	{
		buf.PutFloat(vec.x);
		buf.PutFloat(vec.y);
		buf.PutFloat(vec.z);
	}

	char szCurrentMapName[MAX_MAP_NAME + 1];
	V_strcpy_safe(szCurrentMapName, STRING(gpGlobals->mapname));
	filesystem->CreateDirHierarchy("maps/dm_locs");

	char szFName[512];
	V_sprintf_safe(szFName, "maps/dm_locs/%s.loc", szCurrentMapName);
	if (!filesystem->WriteFile(szFName, nullptr, buf))
	{
		Msg("Failed to write file: %s", szFName);
	}
}

static void DMSpawnComCallbackLoad(const CCommand &command)
{
	char szCurrentMapName[MAX_MAP_NAME + 1];
	V_strcpy_safe(szCurrentMapName, STRING(gpGlobals->mapname));

	CUtlBuffer buf(0, 0, CUtlBuffer::READ_ONLY);
	char szFName[512];
	V_sprintf_safe(szFName, "maps/dm_locs/%s.loc", szCurrentMapName);
	if (!filesystem->ReadFile(szFName, nullptr, buf))
	{
		Msg("Failed to read file: %s", szFName);
	}

	if (buf.GetUnsignedInt() != MAGIC_NUMBER)
	{
		return;
	}
	gDMSpawnLocs.RemoveAll();

	const int version = buf.GetInt();
	const int locsSize = buf.GetInt();
	for (int i = 0; i < locsSize && buf.IsValid(); ++i)
	{
		Vector vec;
		vec.x = buf.GetFloat();
		vec.y = buf.GetFloat();
		vec.z = buf.GetFloat();
		gDMSpawnLocs.AddToTail(vec);
	}
}

static void DMSpawnComCallbackTeleportNext()
{
	if (gDMSpawnLocs.IsEmpty()) return;
	gDMSpawnCur = LoopAroundInArray(gDMSpawnCur + 1, gDMSpawnLocs.Size());
	if (auto *pPlayer = LoopbackPlayer())
	{
		pPlayer->SetAbsOrigin(gDMSpawnLocs[gDMSpawnCur]);
	}
}

static void DMSpawnComCallbackMapinfo()
{
	char szCurrentMapName[MAX_MAP_NAME + 1];
	V_strcpy_safe(szCurrentMapName, STRING(gpGlobals->mapname));

	int iEntCount = 0;
	CBaseEntity *entDMSpawn = nullptr;
	while ((entDMSpawn = gEntList.FindEntityByClassname(entDMSpawn, "info_player_deathmatch")))
	{
		++iEntCount;
	}

	Msg("Deathmatch spawns for: %s\ndmspawn spawns count: %d\ninfo_player_deathmatch count: %d\nAllow deathmatch: %s",
		szCurrentMapName, gDMSpawnLocs.Size(), iEntCount, (iEntCount > 0 || !gDMSpawnLocs.IsEmpty()) ? "YES" : "NO");
}

ConCommand neo_sv_dmspawn_create("neo_sv_dmspawn_create", &DMSpawnComCallbackCreate, "", FCVAR_USERINFO | FCVAR_CHEAT);
ConCommand neo_sv_dmspawn_removeall("neo_sv_dmspawn_removeall", &DMSpawnComCallbackRemoveAll, "", FCVAR_USERINFO | FCVAR_CHEAT);
ConCommand neo_sv_dmspawn_printlocs("neo_sv_dmspawn_printlocs", &DMSpawnComCallbackPrintLocs, "", FCVAR_USERINFO | FCVAR_CHEAT);
ConCommand neo_sv_dmspawn_save("neo_sv_dmspawn_save", &DMSpawnComCallbackSave, "", FCVAR_USERINFO | FCVAR_CHEAT);
ConCommand neo_sv_dmspawn_load("neo_sv_dmspawn_load", &DMSpawnComCallbackLoad, "", FCVAR_USERINFO | FCVAR_CHEAT);
ConCommand neo_sv_dmspawn_teleportnext("neo_sv_dmspawn_teleportnext", &DMSpawnComCallbackTeleportNext, "", FCVAR_USERINFO | FCVAR_CHEAT);
ConCommand neo_sv_dmspawn_mapinfo("neo_sv_dmspawn_mapinfo", &DMSpawnComCallbackMapinfo, "", FCVAR_USERINFO);
ConVar neo_sv_dmspawn_useent("neo_sv_dmspawn_useent", "0", FCVAR_USERINFO);
