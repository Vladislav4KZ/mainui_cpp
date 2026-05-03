#include "BaseMenu.h"
#include "ClientWindow.h"
#include "ScrollView.h"

static class CClientJoinGame : public CClientWindow
{
public:
	typedef CClientWindow BaseClass;
	CClientJoinGame() : BaseClass( "CClientJoinGame" ) {}

	void _Init();
	void Reload();

	bool hasSpectator;
	bool hasVIP;
	bool hasCancel;
	CMenuAction *spectate;
	CMenuAction *vipbutton;
	CMenuAction *cancel;
	CMenuAction text;
	CMenuScrollView scroll;

	CEventCallback MakeCb( const char *cmd )
	{
		return CEventCallback( MenuCb( &CClientJoinGame::teamPressedCb ), (void*)cmd );
	}

	CEventCallback MakeFocusCb( const char *cmd )
	{
		return CEventCallback( MenuCb( &CClientJoinGame::teamFocusCb ), (void*)cmd );
	}

private:
	const char *command;
	char textbuffer[1024];

	void ConfirmSelection()
	{
		Hide();
		EngFuncs::ClientCmd( false, command );
	}

	void teamFocusCb( void *pExtra )
	{
		command = (const char*)pExtra;
	}

	void teamPressedCb( void *pExtra )
	{
		teamFocusCb( pExtra );
		ConfirmSelection();
	}

} uiJoinGame;

void CClientJoinGame::_Init()
{
	CMenuAction *btn;
	btn = AddButton( '1', L( "Cstrike_Terrorist_Forces" ),
		Point( 100, 180 ), MakeCb( "jointeam 1" ));
	btn->onGotFocus = MakeFocusCb( "jointeam 1" );

	btn = AddButton( '2', L( "Cstrike_CT_Forces" ),
		Point( 100, 230 ), MakeCb( "jointeam 2" ));
	btn->onGotFocus = MakeFocusCb( "jointeam 2" );

	vipbutton = AddButton( '3', L( "Cstrike_VIP_Team" ),
		Point( 100, 280 ), MakeCb( "jointeam 3" ));
	vipbutton->onGotFocus = MakeFocusCb( "jointeam 3" );

	btn = AddButton( '5', L( "Cstrike_Team_AutoAssign" ),
		Point( 100, 380 ), MakeCb( "jointeam 5" ));
	btn->onGotFocus = MakeFocusCb( "jointeam 5" );

	spectate = AddButton( '6', L( "Cstrike_Menu_Spectate" ),
		Point( 100, 430 ), MakeCb( "jointeam 6" ));
	spectate->onGotFocus = MakeFocusCb( "jointeam 6" );

	cancel = AddButton( '0', L( "Cstrike_Cancel" ),
		Point( 100, 530 ), CEventCallback( VoidCb( &CMenuBaseWindow::Hide ) ) );

	scroll.SetRect( 400, 180, 400, 200 );
	scroll.bDrawStroke = true;
	scroll.colorStroke = uiInputTextColor;
	scroll.iStrokeWidth = 1;

	text.SetRect( 0, 0, 0, 0);
	text.SetBackground( 0U );
	text.SetInactive( true );
	text.SetCharSize( QM_SMALLERFONT );
	text.szName = textbuffer;
	text.m_bLimitBySize = false;

	scroll.AddItem( text );
	AddItem( scroll );

	szName = L("Cstrike_Join_Team");
}

void CClientJoinGame::Reload()
{
	if( hasSpectator )
	{
		spectate->Show();
		keys[6] = spectate->onPressed;
	}
	else
	{
		keys[6].Reset();
		spectate->Hide();
	}

	if( hasVIP )
	{
		keys[3] = vipbutton->onPressed;
		vipbutton->Show();
	}
	else
	{
		keys[3].Reset();
		vipbutton->Hide();
	}

	if( hasCancel )
	{
		keys[0] = cancel->onPressed;
		cancel->Show();
	}
	else
	{
		keys[0].Reset();
		cancel->Hide();
	}

	const char *mapname = EngFuncs::GetCvarString( "cscl_currentmap" );

	if( mapname && mapname[0] )
	{
		char buf[256];
		snprintf( buf, 256, "maps/%s.txt", mapname );

		char *txt = (char*)EngFuncs::COM_LoadFile( buf );

		if( txt )
		{
			Q_strncpy( textbuffer, txt, sizeof( textbuffer ));

			EngFuncs::COM_FreeFile( txt );
		}
		else
		{
			Q_strncpy( textbuffer, L( "Cstrike_TitlesTXT_Map_Description_not_available" ), sizeof( textbuffer ));
		}
	}
	else
	{
		Q_strncpy( textbuffer, L( "Cstrike_TitlesTXT_Map_Description_not_available" ), sizeof( textbuffer ));
	}

	text.size.h = 0; // recalc
	text.size.w = 0; // recalc
	scroll.VidInit();
}
void UI_JoinGame_Show( int param1, int param2 )
{
	uiJoinGame.hasSpectator = param1 & 1;
	uiJoinGame.hasVIP		= param1 & 2;
	uiJoinGame.hasCancel    = param1 & 4;

	uiJoinGame.Show();
}
