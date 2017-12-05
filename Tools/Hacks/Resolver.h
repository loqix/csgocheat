#pragma once
#include "../../stdafx.h"
#include "../Utils/Playerlist.h"
#include "../Utils/Hitbox.h"
#include "../../SDK/SDK.h"
#include "Backtrack.h"
#define TICK_INTERVAL			(Interfaces.pGlobalVars->interval_per_tick)
#define TIME_TO_TICKS2( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

//second: check if the animation state is 979, then do your flip however you want >120
//check if animation state is 973, then do your flip however you want <120

int CBaseEntity::GetSequenceActivity(int sequence)
{
	auto hdr = Interfaces.g_pModelInfo->GetStudioModel(this->GetModel());

	if (!hdr)
		return -1;

	// c_csplayer vfunc 242, follow calls to find the function.

	static auto client2 = GetModuleHandleW(L"client.dll");
	static auto getSequenceActivity = (DWORD)(Utils.PatternScan(client2, "55 8B EC 83 7D 08 FF 56 8B F1 74"));
	static auto GetSequenceActivity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(getSequenceActivity);

	return GetSequenceActivity(this, hdr, sequence);
}

bool isPartOf(char *a, char *b) {
	if (std::strstr(b, a) != NULL) {    //Strstr says does b contain a
		return true;
	}
	return false;
}

class CResolver {
public:
	void AntiAimResolver();
	void AntiAimCorrection();
private:
	bool IsFakeWalking();
};

struct ResolverData
{
	float simtime, flcycle[13], flprevcycle[13], flweight[13], flweightdatarate[13], fakewalkdetection[2], fakeanglesimtimedetection[2], fakewalkdetectionsimtime[2];
	float yaw, addyaw, lbycurtime;
	float shotsimtime, oldlby, lastmovinglby, balanceadjustsimtime, balanceadjustflcycle;
	int fakeanglesimtickdetectionaverage[4], amountgreaterthan2, amountequal1or2, amountequal0or1, amountequal1, amountequal0, resetmovetick, resetmovetick2;
	int tick, balanceadjusttick, missedshots, activity[13];
	bool bfakeangle, bfakewalk, playerhurtcalled, weaponfirecalled;
	Vector shotaimangles, hitboxPos, balanceadjustaimangles;
	uint32_t norder[13];
	char* resolvermode = "NONE", *fakewalk = "Not Moving";
};

extern ResolverData pResolverData[64];
extern CResolver* Resolver;

void CResolver::AntiAimResolver()
{
	CBaseEntity* pLocal = Hacks.LocalPlayer;
	player_info_t info{};
	if (!pLocal->isAlive())
		return;

	for (int i = 0; i < Interfaces.pEngine->GetMaxClients(); i++)
	{
		auto pEntity = Interfaces.pEntList->GetClientEntity(i);

		if (!pEntity || !pLocal)
			continue;

		if (pEntity == pLocal)
			continue;

		if (!Interfaces.pEngine->GetPlayerInfo(i, &info))
			continue;

		if (pEntity->IsDormant())
			continue;

		if (pEntity->GetTeam() == pLocal->GetTeam())
			continue;

		if (pEntity->isAlive())
		{
			if (pResolverData[i].addyaw > 135.f)
				pResolverData[i].addyaw = 0.f;
			// Add Spread Check here if you want pBruteForce
			if (pResolverData[Hacks.LocalPlayer->GetIndex()].weaponfirecalled)
			{
				if (!pResolverData[pEntity->GetIndex()].playerhurtcalled)
				{
					if (isPartOf("Brute", pResolverData[i].resolvermode))
					{
						//if (hitgroup >= 4 && hitgroup <= 7)
						//pResolverData[i].addyaw += 15;
						//else
						pResolverData[i].addyaw += 45.f;
					}
				}
				else
					pResolverData[i].playerhurtcalled = false;
				pResolverData[Hacks.LocalPlayer->GetIndex()].weaponfirecalled = false;
			}
			//Garbage Fake Angle Detection
			/*pResolverData[pEntity->GetIndex()].fakeanglesimtimedetection[Hacks.CurrentCmd->command_number % 2] = pEntity->GetSimulationTime();
			pResolverData[pEntity->GetIndex()].fakeanglesimtickdetectionaverage[Hacks.CurrentCmd->command_number % 4] = TIME_TO_TICKS2(fabs(pEntity->GetSimulationTime() - pEntity->GetOldSimulationTime()));
			pResolverData[pEntity->GetIndex()].amountgreaterthan2 = 0; pResolverData[pEntity->GetIndex()].amountequal1 = 0; pResolverData[pEntity->GetIndex()].amountequal0 = 0; pResolverData[pEntity->GetIndex()].amountequal1or2 = 0; pResolverData[pEntity->GetIndex()].amountequal0or1 = 0;
			for (int i = 0; i < 4; i++)
			{
				if (pResolverData[pEntity->GetIndex()].fakeanglesimtickdetectionaverage[i] >= 2)
				{
					pResolverData[pEntity->GetIndex()].amountgreaterthan2++;
					pResolverData[pEntity->GetIndex()].amountequal1or2++;
				}
				else if (pResolverData[pEntity->GetIndex()].fakeanglesimtickdetectionaverage[i] == 1)
				{
					pResolverData[pEntity->GetIndex()].amountequal1or2++;
					pResolverData[pEntity->GetIndex()].amountequal1++;
					pResolverData[pEntity->GetIndex()].amountequal0or1++;
				}
				else if (pResolverData[pEntity->GetIndex()].fakeanglesimtickdetectionaverage[i] == 0)
				{
					pResolverData[pEntity->GetIndex()].amountequal0++;
					pResolverData[pEntity->GetIndex()].amountequal0or1++;
				}
			}
			if (pResolverData[pEntity->GetIndex()].bfakewalk)
			{
				if (pResolverData[pEntity->GetIndex()].amountequal0 == 3 && pResolverData[pEntity->GetIndex()].amountgreaterthan2 == 1 ||
					pResolverData[pEntity->GetIndex()].amountequal0 == 2 && pResolverData[pEntity->GetIndex()].amountgreaterthan2 == 2 
					|| pResolverData[pEntity->GetIndex()].amountequal0 >= 3  /*this is to force
					)
					pResolverData[pEntity->GetIndex()].bfakeangle = true;
				else
					pResolverData[pEntity->GetIndex()].bfakeangle = false;
			}
			else
			{
				if (pResolverData[pEntity->GetIndex()].amountequal1or2 >= 4 ||
					pResolverData[pEntity->GetIndex()].amountequal0or1 >= 4 ||
					pResolverData[pEntity->GetIndex()].amountgreaterthan2 == 3 && pResolverData[pEntity->GetIndex()].amountequal1 == 1 ||
					pResolverData[pEntity->GetIndex()].amountequal1 >= 2)
					pResolverData[pEntity->GetIndex()].bfakeangle = false;
				else
					pResolverData[pEntity->GetIndex()].bfakeangle = true;
			}*/

			for (int w = 0; w < 13; w++)
			{
				AnimationLayer currentLayer = pEntity->GetAnimOverlay(w);
				const int activity = pEntity->GetSequenceActivity(currentLayer.m_nSequence);
				float flcycle = currentLayer.m_flCycle, flprevcycle = currentLayer.m_flPrevCycle, flweight = currentLayer.m_flWeight, flweightdatarate = currentLayer.m_flWeightDeltaRate;
				uint32_t norder = currentLayer.m_nOrder;
				Vector* pAngles = pEntity->GetEyeAnglesPointer();
				/*if (pEntity->GetSimulationTime() != pResolverData[pEntity->GetIndex()].simtime)
				{
				pResolverData[pEntity->GetIndex()].simtime = pEntity->GetSimulationTime();
				}*/
				// Fake Walk Detection NOrder 12 is activated when an animation starts and you can use this to detect fakewalk
				// Cause if you understand how fake walk works, they choke and then run and they stop at 0 velocity when they unchoke
				// causing the 0 velocity to be sent and lby not being upadted and it starts a new animation
				// Which is norder 12, if you can figure out how to instantly stop, you can go any velocity fucking up velocity checks.
				// This works against all fake walks.
				// To make this more accurate use more ticks
				if (norder == 12)
				{
					pResolverData[pEntity->GetIndex()].fakewalkdetection[Hacks.CurrentCmd->command_number % 2] = flweight;
					pResolverData[pEntity->GetIndex()].fakewalkdetectionsimtime[Hacks.CurrentCmd->command_number % 2] = pEntity->GetSimulationTime();
					for (int t = 0; t < 2; t++)
					{
						int resetmovetick2{};
						if (pResolverData[pEntity->GetIndex()].fakewalkdetection[t] > 0.f)
							pResolverData[pEntity->GetIndex()].resetmovetick = t;
						else if (t == 1)
						{
							if (pEntity->GetVecVelocity().Length2D() < 0.50 && flweight == 0.f)
							{
								pResolverData[pEntity->GetIndex()].fakewalk = "Not Moving";
								pResolverData[pEntity->GetIndex()].bfakewalk = false;
							}
						}
						else {
							if (pResolverData[pEntity->GetIndex()].resetmovetick > 0)
								resetmovetick2 = pResolverData[pEntity->GetIndex()].resetmovetick - 1;
							else
								resetmovetick2 = pResolverData[pEntity->GetIndex()].resetmovetick + 1;

							if (pResolverData[pEntity->GetIndex()].fakewalkdetection[resetmovetick2] == 0.f)
							{
								pResolverData[pEntity->GetIndex()].fakewalk = "Fake Walking";
								pResolverData[pEntity->GetIndex()].bfakewalk = true;
							}
						}
					}
				}
				// Removed fake angle check unreliable
				//if (pResolverData[pEntity->GetIndex()].bfakeangle || !pResolverData[pEntity->GetIndex()].bfakeangle)
				//{
					if (pEntity->GetVecVelocity().Length2D() >= 0.50 && norder == 6 && flweight >= 0.550000 || pEntity->GetVecVelocity().Length2D() >= 0.50 && norder == 5 && flweight >= 0.550000 || !pResolverData[pEntity->GetIndex()].bfakewalk && pEntity->GetVecVelocity().Length2D() >= 0.50)
					{
						float simtime = pEntity->GetSimulationTime();
						Hitbox box{};
						if (!box.GetHitbox(pEntity, 0))
							continue;
						Vector hitboxPos{};
						float damage = box.GetBestPoint(hitboxPos);
						pResolverData[pEntity->GetIndex()].lastmovinglby = pEntity->pelvisangs();
						RageBackData[pEntity->GetIndex()][Hacks.CurrentCmd->command_number % 16].damage = damage;
						RageBackData[pEntity->GetIndex()][Hacks.CurrentCmd->command_number % 16].simtime = simtime;
						RageBackData[pEntity->GetIndex()][Hacks.CurrentCmd->command_number % 16].hitboxPos = hitboxPos;
						pResolverData[pEntity->GetIndex()].resolvermode = "LBY Move";
						pResolverData[pEntity->GetIndex()].fakewalk = "No Fake Walk";
						pAngles->y = pEntity->pelvisangs();
					}
					else
					{
						/*if (Hacks.CurrentCmd->tick_count % 2 <= 1)
						{
						if (norder == 6 && flweight == 0.f || norder == 12 && flweight == 0.f)
						{
						pResolverData[pEntity->GetIndex()].fakewalk = "Not Moving";
						}
						else if (pEntity->GetVecVelocity().Length2D() >= 0.50 && norder == 6 && flweight >= 0.550000)
						pResolverData[pEntity->GetIndex()].fakewalk = "Fake Walking";
						}*/
						/*else if (backtracking->IsTickValid(RageBackData[pEntity->GetIndex()][Hacks.CurrentCmd->command_number % 16].simtime))
						{
						Hacks.CurrentCmd->tick_count = TIME_TO_TICKS2(RageBackData[pEntity->GetIndex()][Hacks.CurrentCmd->command_number % 16].simtime);
						pAngles->y = RageBackData[pEntity->GetIndex()][Hacks.CurrentCmd->command_number % 16].yaw;
						}*/
						/*if (activity == ACT_CSGO_IDLE_TURN_BALANCEADJUST && flweight > 0.f && flcycle > 0.f && flcycle <= 0.05f)
						{
							pResolverData[pEntity->GetIndex()].balanceadjustsimtime = pEntity->GetSimulationTime();
							if (backtracking->IsTickValid(pResolverData[pEntity->GetIndex()].balanceadjustsimtime))
							{
								Hitbox box{};
								if (!box.GetHitbox(pEntity, 0))
									continue;
								pResolverData[pEntity->GetIndex()].balanceadjustaimangles = box.GetCenter();
								//if (Hacks.CurrentCmd->buttons & IN_ATTACK)
								pResolverData[pEntity->GetIndex()].resolvermode = "BA Update";
								pAngles->y = pEntity->pelvisangs() - 17.5;
								if (Hacks.CurrentCmd && Hacks.CurrentCmd->buttons & IN_ATTACK)
									backtracking->BackTrackPlayer(Hacks.CurrentCmd, TIME_TO_TICKS2(pResolverData[pEntity->GetIndex()].balanceadjustsimtime));
							}
						}*/
						/*else if (activity == ACT_CSGO_FIRE_PRIMARY && flweight > 0.f && flcycle > 0.f && flcycle <= 0.05f)
						{	
							Hitbox box{};
							if (!box.GetHitbox(pEntity, 0))
								continue;
							pResolverData[pEntity->GetIndex()].resolvermode = "Fire Update";
							pResolverData[pEntity->GetIndex()].shotsimtime = pEntity->GetSimulationTime();
							pResolverData[pEntity->GetIndex()].shotaimangles = box.GetCenter();
						}
						else if (backtracking->IsTickValid(pResolverData[pEntity->GetIndex()].balanceadjustsimtime))
						{
							pResolverData[pEntity->GetIndex()].resolvermode = "BA";
							if (Hacks.CurrentCmd && Hacks.CurrentCmd->buttons & IN_ATTACK)
								backtracking->BackTrackPlayer(Hacks.CurrentCmd, TIME_TO_TICKS2(pResolverData[pEntity->GetIndex()].balanceadjustsimtime));
						}
						else if (backtracking->IsTickValid(pResolverData[pEntity->GetIndex()].shotsimtime))
						{
							pResolverData[pEntity->GetIndex()].resolvermode = "Fire";
							if (Hacks.CurrentCmd && Hacks.CurrentCmd->buttons & IN_ATTACK)
								backtracking->BackTrackPlayer(Hacks.CurrentCmd, TIME_TO_TICKS2(pResolverData[pEntity->GetIndex()].shotsimtime));
						}*/
						// Check how many times they triggered it in less than 1.1f time to make this more accurate
						if (activity == ACT_CSGO_IDLE_TURN_BALANCEADJUST && flweight >= 0.05f)
						{
							pResolverData[pEntity->GetIndex()].resolvermode = "Less BA Brute";
							pResolverData[pEntity->GetIndex()].addyaw > 0.f ? pAngles->y = pEntity->pelvisangs() - pResolverData[pEntity->GetIndex()].addyaw - 77.5 : pAngles->y = pEntity->pelvisangs() - 77.5;
						}
						else if (activity == ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING || activity == ACT_CSGO_IDLE_TURN_BALANCEADJUST && flweight == 0.f && flcycle >= 0.970000)
						{
							pResolverData[pEntity->GetIndex()].resolvermode = "LBY Brute";
							pResolverData[pEntity->GetIndex()].addyaw > 0.f ? pAngles->y = pEntity->pelvisangs() - pResolverData[pEntity->GetIndex()].addyaw : pAngles->y = pEntity->pelvisangs() - 17.5;
							//pEntity->pelvisangs() < 0.f ? pAngles->y = (pResolverData[pEntity->GetIndex()].lastmovinglby + (pResolverData[pEntity->GetIndex()].lastmovinglby - pEntity->pelvisangs())) : pAngles->y = (pResolverData[pEntity->GetIndex()].lastmovinglby - (pResolverData[pEntity->GetIndex()].lastmovinglby - pEntity->pelvisangs()));
							//activity == ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING ? pAngles->y = pEntity->pelvisangs() : pAngles->y = pEntity->pelvisangs() - 90;
						}
					}
				//}
				//else
					//pResolverData[pEntity->GetIndex()].resolvermode = "No Fake Angle";
				g_Math.AngleNormalise(*pAngles);
			}
		}
	}
}

ResolverData pResolverData[64];
CResolver* Resolver;