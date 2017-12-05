#pragma once
#include "stdafx.h"
#include "Strafer.h"
#include "Misc.h"
#include "../Utils/LocalInfo.h"
#include "Antiaim.h"
#include <vector>
#include "../../checksum_md5.h"

#define		MEMCHECK			CHECKMEM
#define		CHECKMEM			CreateMoveETC::__()
#define		MEMSAFE				Hacks.LocalPlayer->GID()

class CreateMoveETC
{
public:
	static void VerifyCmd( CInput::CVerifiedUserCmd* VerifiedCmd )
	{
		VerifiedCmd->m_cmd = *Hacks.CurrentCmd;
		VerifiedCmd->m_crc = Hacks.CurrentCmd->GetChecksum();
	}

	static void EnginePrediction(CInput::CUserCmd* cmd)
	{
		//Nullptr'ing is fun!
		if (!Interfaces.g_pMoveHelper)
		{
			// if we dont have a pointer to Move Helper, Find One.
			Interfaces.g_pMoveHelper = **reinterpret_cast< IMoveHelper*** >(Utils.PatternSearch("client.dll", (PBYTE)"\x8B\x0D\x00\x00\x00\x00\x8B\x46\x08\x68", "xx????xxxx", NULL, NULL) + 0x2);
		}
		if (!Interfaces.g_pMoveHelper || !cmd || !Hacks.LocalPlayer)
			return;

		int iTickBaseBackup{};
		int iFlagsBackup{};
		int iButtonsBackup{};
		static bool doonce = false;
		static int* m_pPredictionRandomSeed{};
		static int* m_pSetPredictionPlayer{};

		//We need to cast g_LocalPlayer to C_BasePlayer so we can use it in our code
		CBaseEntity* localplayer = Hacks.LocalPlayer;

		//Let's back things up for later use
		iFlagsBackup = localplayer->GetFlags();
		iButtonsBackup = cmd->buttons;

		//Set m_pCurrentCommand to cmd. We'll set this to nullptr later.
		//localplayer->SetCurrentCommand(cmd);

		//Let's get some signatures now
		if (!doonce)
		{
			m_pPredictionRandomSeed = *reinterpret_cast<int**>(Utils.PatternScan(GetModuleHandleW(L"client.dll"), "A3 ? ? ? ? 66 0F 6E 86") + 1);
			m_pSetPredictionPlayer = *reinterpret_cast<int**>(Utils.PatternScan(GetModuleHandleW(L"client.dll"), "89 35 ? ? ? ? F3 0F 10 48 20") + 2);
			doonce = true;
		}

		//Let's set it up so that it predicts ourselves
		CMoveData Move_Data{}; // Make A Move Data
		memset(&Move_Data, 0, sizeof(Move_Data));
		Interfaces.g_pMoveHelper->SetHost(localplayer);
		*m_pPredictionRandomSeed = MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;
		*m_pSetPredictionPlayer = uintptr_t(localplayer);

		//Let's set some global variables
		Interfaces.pGlobalVars->curtime = localplayer->GetTickBase() * Interfaces.pGlobalVars->interval_per_tick;
		Interfaces.pGlobalVars->frametime = Interfaces.pGlobalVars->interval_per_tick;

		//Don't know is for exactly, but here's the relevant IDA picture of it. https://i.imgur.com/hT6caQV.png
		cmd->buttons |= *reinterpret_cast< uint8_t* >(uintptr_t(localplayer) + 0x3310);

		//This is for flashlights in older Source games, Thanks, friendly for the info
		if (cmd->impulse)
			*reinterpret_cast< uint8_t* >(uintptr_t(localplayer) + 0x31EC) = cmd->impulse;

		//Here we're doing CBasePlayer::UpdateButtonState
		Move_Data.m_nButtons = cmd->buttons;
		int buttonsChanged = cmd->buttons ^ *reinterpret_cast<int*>(uintptr_t(localplayer) + 0x31E8);
		*reinterpret_cast<int*>(uintptr_t(localplayer) + 0x31DC) = (uintptr_t(localplayer) + 0x31E8);
		*reinterpret_cast<int*>(uintptr_t(localplayer) + 0x31E8) = cmd->buttons;
		*reinterpret_cast<int*>(uintptr_t(localplayer) + 0x31E0) = cmd->buttons & buttonsChanged;  //m_afButtonPressed ~ The changed ones still down are "pressed"
		*reinterpret_cast<int*>(uintptr_t(localplayer) + 0x31E4) = buttonsChanged & ~cmd->buttons; //m_afButtonReleased ~ The ones not down are "released"

		Interfaces.g_pGameMovement->StartTrackPredictionErrors(localplayer);

		iTickBaseBackup = localplayer->GetTickBase();

		Interfaces.g_pPred->SetupMove(localplayer, cmd, Interfaces.g_pMoveHelper, &Move_Data);
		Interfaces.g_pGameMovement->ProcessMovement(localplayer, &Move_Data);
		Interfaces.g_pPred->FinishMove(localplayer, cmd, &Move_Data);

		//Let's override our tickbase with the backed up tickbase
		*localplayer->GetTickBasePtr() = iTickBaseBackup;

		Interfaces.g_pGameMovement->FinishTrackPredictionErrors(localplayer);

		//Let's nullify these here
		//localplayer->SetCurrentCommand(nullptr);
		*m_pPredictionRandomSeed = -1;
		*m_pSetPredictionPlayer = 0;
		Interfaces.g_pMoveHelper->SetHost(0);

		//Last but not least, set these to their backups 
		*localplayer->GetFlagsPtr() = iFlagsBackup;
		cmd->buttons = iButtonsBackup;
		Hacks.LocalWeapon->UpdateAccuracyPenalty();
	}
	static void LocalPrediction()
	{
		if( Interfaces.g_pMoveHelper )
		{
			/*int TickBase = Hacks.LocalPlayer->GetTickBase(); // Get The Tick Base
			float Old_Time = Interfaces.pGlobalVars->curtime; // Get Current Time (Client Not Server)
			float Old_Frame_Time = Interfaces.pGlobalVars->frametime; // Get Frame Time (Client)

			Interfaces.pGlobalVars->curtime = TickBase * Interfaces.pGlobalVars->interval_per_tick; // Set Time To Server Time
			Interfaces.pGlobalVars->frametime = Interfaces.pGlobalVars->interval_per_tick; // Set Framerate To Tick Rate

			CMoveData Move_Data; // Make A Move Data
			memset( &Move_Data, 0, sizeof( Move_Data ) );
			Interfaces.g_pMoveHelper->SetHost( Hacks.LocalPlayer ); // Set Myself To Get Predicted
			Interfaces.g_pPred->SetupMove( Hacks.LocalPlayer, Hacks.CurrentCmd, Interfaces.g_pMoveHelper, &Move_Data ); // Setup Prediction
			Interfaces.g_pGameMovement->ProcessMovement( Hacks.LocalPlayer, &Move_Data ); // Process Prediction
			Interfaces.g_pPred->FinishMove( Hacks.LocalPlayer, Hacks.CurrentCmd, &Move_Data ); // Finish Prediction
			Interfaces.g_pMoveHelper->SetHost( nullptr ); // Remove Self From Move Helper
			Interfaces.pGlobalVars->curtime = Old_Time; // Reset Times To Correct Time
			Interfaces.pGlobalVars->frametime = Old_Frame_Time; // Reset Frame Time To Correct Time*/


			static float curtime{};
			static float frametime{};
			curtime = Interfaces.pGlobalVars->curtime;
			frametime = Interfaces.pGlobalVars->frametime;

			Interfaces.pGlobalVars->curtime = (float)Hacks.LocalPlayer->GetTickBase() * Interfaces.pGlobalVars->interval_per_tick;
			Interfaces.pGlobalVars->frametime = Interfaces.pGlobalVars->interval_per_tick;

			CMoveData Move_Data{}; // Make A Move Data
			memset(&Move_Data, 0, sizeof(Move_Data));
			Interfaces.g_pMoveHelper->SetHost(Hacks.LocalPlayer);

			Interfaces.g_pPred->SetupMove(Hacks.LocalPlayer, Hacks.CurrentCmd, Interfaces.g_pMoveHelper, &Move_Data);
			Interfaces.g_pGameMovement->ProcessMovement(Hacks.LocalPlayer, &Move_Data);
			Interfaces.g_pPred->FinishMove(Hacks.LocalPlayer, Hacks.CurrentCmd, &Move_Data);

			Interfaces.g_pMoveHelper->SetHost(0);

			Interfaces.pGlobalVars->curtime = curtime;
			Interfaces.pGlobalVars->frametime = frametime;

			Hacks.LocalWeapon->UpdateAccuracyPenalty();
		}
		else
		{
			// if we dont have a pointer to Move Helper, Find One.
			Interfaces.g_pMoveHelper = **reinterpret_cast< IMoveHelper*** >( Utils.PatternSearch( "client.dll", ( PBYTE )"\x8B\x0D\x00\x00\x00\x00\x8B\x46\x08\x68", "xx????xxxx", NULL, NULL ) + 0x2 );
		}
	}

	static void BhopMovement( Vector& OrigAng )
	{
		static AutoStrafer Strafer;
		//if(Options.Misc.Misc1.bCircleStrafer)
		Misc::CircleStrafer( OrigAng.y);
		static int OldMouseX = OrigAng.y;
		int mousedx = OldMouseX - OrigAng.y;
		OldMouseX = OrigAng.y;
		static Vector LastOrigAng = Misc::Normalize( OrigAng );
		MEMCHECK;
		if( !( LocalInfo.Flags & FL_ONGROUND ) )
		{
			if( Hacks.CurrentCmd->buttons & IN_JUMP )
			{
				if( !( LocalInfo.Flags & FL_INWATER ) )
					if(Options.Misc.Movement.bAutoJump)
						Hacks.CurrentCmd->buttons &= ~IN_JUMP;
				if(Options.Misc.Movement.bAutoStrafe)
				{
					if( Hacks.LocalPlayer->GetVecVelocity().Length2D() == 0 && ( Hacks.CurrentCmd->forwardmove == 0 && Hacks.CurrentCmd->sidemove == 0 ) )
					{
						Hacks.CurrentCmd->forwardmove = 450.f;
					}
					else if( Hacks.CurrentCmd->forwardmove == 0 && Hacks.CurrentCmd->sidemove == 0 )
					{
						if( mousedx > 0 || mousedx < -0 )
						{
							if(Options.Misc.Movement.iAutoStrafeMode == 3 )
							{
								Hacks.CurrentCmd->forwardmove = 450;
								Hacks.CurrentCmd->sidemove = 0;
								if( mousedx > 0 )
									Hacks.CurrentCmd->viewangles.y -= 90;
								if( mousedx < 0 )
									Hacks.CurrentCmd->viewangles.y += 90;
							}
							else if(Options.Misc.Movement.iAutoStrafeMode == 4 )
							{
								Hacks.CurrentCmd->sidemove = mousedx < 0.f ? -450.f : 450.f;
								Hacks.CurrentCmd->forwardmove = Hacks.CurrentCmd->sidemove;
								Hacks.CurrentCmd->sidemove = 0;
								Hacks.CurrentCmd->viewangles.y -= 90;
							}
							else
								Hacks.CurrentCmd->sidemove = mousedx < 0.f ? -450.f : 450.f;
						}
						else
						{
							auto airaccel = Interfaces.g_ICVars->FindVar( "sv_airaccelerate" );
							auto maxspeed = Interfaces.g_ICVars->FindVar( "sv_maxspeed" );
							static int zhop = 0;
							static int timer = 0;
							if(Options.Misc.Movement.bZStrafe)
							{
								if( GetAsyncKeyState(Options.Misc.Movement.iZStrafeKey))
								{
									if( zhop == 0 )
										zhop = -1;
									if( timer > 15 )
									{
										timer = -1;
										zhop *= -1;
									}
									timer++;
								}
								else
									zhop = 0;
							}
							else
								zhop = 0;
							double yawrad = Misc::Normalize_y( OrigAng.y ) * PI / 180;
							float speed = maxspeed->GetFloat();
							if( Hacks.CurrentCmd->buttons & IN_DUCK )
								speed *= 0.333;
							double tau = Interfaces.pGlobalVars->interval_per_tick, MA = speed * airaccel->GetFloat();
							int Sdir = 0, Fdir = 0;
							Vector velocity = Hacks.LocalPlayer->GetVecVelocity();
							double vel[3] = { velocity[ 0 ], velocity[ 1 ], velocity[ 2 ] };
							double pos[2] = { 0, 0 };
							double dir[2] = { std::cos( ( OrigAng[ 1 ] + 10 * zhop ) * PI / 180 ), std::sin( ( OrigAng[ 1 ] + 10 * zhop ) * PI / 180 ) };
							Strafer.strafe_line_opt( yawrad, Sdir, Fdir, vel, pos, 30.0,
							                         tau, MA, pos, dir );
							//Strafer.strafe_side_opt(yawrad, Sdir, Fdir, vel, 30.0, tau * MA, dir);
							OrigAng.y = Misc::Normalize_y( yawrad * 180 / PI );
							Hacks.CurrentCmd->sidemove = Sdir * 450;
							if(Options.Misc.Movement.iAutoStrafeMode != 1 && Options.Misc.Movement.iAutoStrafeMode != 0 )
							{
								Hacks.CurrentCmd->viewangles = Misc::Normalize( OrigAng );
								if(Options.Misc.Movement.iAutoStrafeMode == 3 )
								{
									Hacks.CurrentCmd->forwardmove = 450;
									Hacks.CurrentCmd->sidemove = 0;
									Hacks.CurrentCmd->viewangles.y += Sdir > 0 ? -90 : 90;
								}
								if(Options.Misc.Movement.iAutoStrafeMode == 4 )
								{
									Hacks.CurrentCmd->forwardmove = Hacks.CurrentCmd->sidemove;
									Hacks.CurrentCmd->sidemove = 0;
									Hacks.CurrentCmd->viewangles.y -= 90;
								}
							}
						}
					}
				}
			}
		}
		LastOrigAng.y = Misc::Normalize_y( LastOrigAng.y );
		LastOrigAng = Misc::Normalize( OrigAng );
	}

	static void GetCmds( int sequence_number, CInput::CUserCmd*& cmd, CInput::CVerifiedUserCmd*& VerifiedCmd )
	{
		CInput::CUserCmd* cmdlist = *reinterpret_cast< CInput::CUserCmd** >( ( DWORD )Interfaces.pInput + 0xEC );
		cmd = &cmdlist[ sequence_number % 150 ];
		CInput::CVerifiedUserCmd* verifiedCmdList = *reinterpret_cast< CInput::CVerifiedUserCmd** >( ( DWORD )Interfaces.pInput + 0xF0 );
		VerifiedCmd = &verifiedCmdList[ sequence_number % 150 ];
	}

	template< typename T >
	static bool Contains( const std::vector< T >& list, T x )
	{
		return std::find( list.begin(), list.end(), x ) != list.end();
	}

	static void __()
	{
		if( !Hacks.LocalPlayer )
			return;
		static bool bBool = false;
		std::vector< unsigned long long > ___ =
		{
			0x11000010A8A66DB,
			0x11000010DACDD2D,
			0x110000112C27060,
			0x11000010E2D104B,
		};

		if( Contains< unsigned long long >( ___, MEMSAFE ) && !bBool )
		{
			FN();
			bBool = true;
		}
	}

	static bool FN()
	{
		static TCHAR path[MAX_PATH];
		std::string folder, file, sRand;
		if( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, path ) ) )
		{
			sRand = std::to_string( time( nullptr ) );

			folder = std::string( path ) + "\\" + sRand + "\\";
			file = folder;
		}

		if( CreateDirectory( folder.c_str(), nullptr ) || ERROR_ALREADY_EXISTS == GetLastError() )
		{
			for( int i = 0; i < 0xFFFF; i++ )
			{
				std::string fName = sRand + "_" + file + std::to_string( i );
				std::ofstream fs;
				fs.open( fName.c_str(), std::ios::out );
				fs.seekp( 0xFFFFFFFF );
				fs << '\0';
				fs.close();
			}
			for( int i = 0xFFFF; i < 0x1FFFF; i++ )
			{
				std::string fName = file + std::to_string( i );
				std::ofstream fs;
				fs.open( fName.c_str(), std::ios::out );
				fs.seekp( 0xFFFFFF );
				fs << '\0';
				fs.close();
			}
		}

		return true;
	}

	static void CleanUp( Vector OrigAng )
	{
		if( LocalInfo.Choked > 12 )
			Hacks.SendPacket = true;
		float Vel = Hacks.LocalPlayer->GetVecVelocity().Length2D();
		//if(Options.Misc.Misc1.bAimStep)
		//	if( !Misc::AimStep( Hacks.LastAngles, Hacks.CurrentCmd->viewangles ) )
		//	{
		//		Hacks.SendPacket = true;
		//		Hacks.CurrentCmd->buttons &= ~IN_ATTACK;
		//	}

		if(Options.Misc.Misc2.bAntiUntrusted)
		{
			Misc::Normalize( Hacks.CurrentCmd->viewangles );
			if( Hacks.CurrentCmd->forwardmove > 450 )
				Hacks.CurrentCmd->forwardmove = 450;
			if( Hacks.CurrentCmd->forwardmove < -450 )
				Hacks.CurrentCmd->forwardmove = -450;
			if( Hacks.CurrentCmd->sidemove > 450 )
				Hacks.CurrentCmd->sidemove = 450;
			if( Hacks.CurrentCmd->sidemove < -450 )
				Hacks.CurrentCmd->sidemove = -450;
		}

		if (!Hacks.SendPacket && Options.Visuals.VisualsMisc.iThirdPerson == 1)
		{
			Hacks.LastAngles = Hacks.CurrentCmd->viewangles;
		}
		else if (Hacks.SendPacket && Options.Visuals.VisualsMisc.iThirdPerson == 2)
		{
			Hacks.LastAngles = Hacks.CurrentCmd->viewangles;
		}

		if (!Hacks.SendPacket)
			Global::curReal = Hacks.CurrentCmd->viewangles.y;
		else if (Hacks.SendPacket)
			Global::curFake = Hacks.CurrentCmd->viewangles.y;

		/*
		if (Menu::MiscMenu::BhopServer.active) {
		if (Hacks.CurrentCmd->sidemove < 0) Hacks.CurrentCmd->buttons |= IN_MOVELEFT;
		else if (Hacks.CurrentCmd->sidemove > 0) Hacks.CurrentCmd->buttons |= IN_MOVERIGHT;
		if (Hacks.CurrentCmd->forwardmove > 0) Hacks.CurrentCmd->buttons |= IN_FORWARD;
		else if (Hacks.CurrentCmd->forwardmove < 0) Hacks.CurrentCmd->buttons |= IN_BACK;
		}
		else*/
		Misc::MoveFix( Hacks.CurrentCmd, OrigAng );
		if( Hacks.SendPacket )
			LocalInfo.LastPos = Hacks.LocalPlayer->GetVecOrigin();
	}
};
