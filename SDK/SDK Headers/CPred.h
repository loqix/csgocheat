#pragma once
#include "stdafx.h"
#define MAX_SPLITSCREEN_PLAYERS 1

class IMoveHelper;
class CBaseHandle;

class CMoveData
{
public:
	bool m_bFirstRunOfFunctions;
	bool m_bGameCodeMovedPlayer;
	bool m_bNoAirControl;
	unsigned long m_nPlayerHandle;
	int m_nImpulseCommand;
	Vector m_vecViewAngles;
	Vector m_vecAbsViewAngles;
	int m_nButtons;
	int m_nOldButtons;
	float m_flForwardMove;
	float m_flSideMove;
	float m_flUpMove;
	float m_flMaxSpeed;
	float m_flClientMaxSpeed;
	Vector m_vecVelocity;
	Vector m_vecOldVelocity;
	float somefloat;
	Vector m_vecAngles;
	Vector m_vecOldAngles;
	float m_outStepHeight;
	Vector m_outWishVel;
	Vector m_outJumpVel;
	Vector m_vecConstraintCenter;
	float m_flConstraintRadius;
	float m_flConstraintWidth;
	float m_flConstraintSpeedFactor;
	bool m_bConstraintPastRadius;
	Vector m_vecAbsOrigin;
};

class CPrediction
{
public:
	void SetupMove(CBaseEntity *player, CInput::CUserCmd *pCmd, IMoveHelper *pHelper, void *move)
	{
		typedef void(__thiscall* SetupMoveFn)(void*, CBaseEntity*, CInput::CUserCmd*, IMoveHelper*, void*);
		return getvfunc<SetupMoveFn>(this, 20)(this, player, pCmd, pHelper, move);
	}

	void FinishMove(CBaseEntity *player, CInput::CUserCmd *pCmd, void *move)
	{
		typedef void(__thiscall* FinishMoveFn)(void*, CBaseEntity*, CInput::CUserCmd*, void*);
		return getvfunc<FinishMoveFn>(this, 21)(this, player, pCmd, move);
	}
};
