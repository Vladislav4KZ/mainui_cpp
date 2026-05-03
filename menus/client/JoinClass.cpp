#include "BaseMenu.h"
#include "ClientWindow.h"
#include "PlayerModelView.h"

class CClientJoinClass : public CClientWindow
{
public:
	typedef CClientWindow BaseClass;
	CClientJoinClass() : BaseClass( "CClientJoinClass" ) {}

	CEventCallback MakeCb( const char *cmd )
	{
		return CEventCallback( MenuCb( &CClientJoinClass::classPressedCb ), (void*)cmd );
	}

	CEventCallback MakeFocusCb( const char *cmd )
	{
		return CEventCallback( MenuCb( &CClientJoinClass::classFocusCb ), (void*)cmd );
	}

	void _Init();
	void Reload();

	CMenuAction *AddButton( int key, const char *name, const char *modelname, Point pos );

	bool hasCzero;
	CMenuPlayerModelView player;
	CMenuAction text;

	void ConfirmSelection()
	{
		UI_CloseClientMenu();
		EngFuncs::ClientCmd( false, command );
	}

protected:
	const char *m_szDefaultClass;

private:
	const char *command;
	char textbuffer[1024];

	void classFocusCb( void *pExtra )
	{
		player.Show();
		text.Show();
		const char *sz = (const char *)pExtra;
		const char *loctext = NULL;
		char model[256];
		static const char *table[5 * 6] =
		{
			"terror",   "urban",    "joinclass 1", "Cstrike_Terror_Label", "Cstrike_Urban_Label",
			"leet",     "gsg9",     "joinclass 2", "Cstrike_Leet_Label", "Cstrike_GSG9_Label",
			"arctic",   "sas",      "joinclass 3", "Cstrike_Arctic_Label", "Cstrike_SAS_Label",
			"guerilla", "gign",     "joinclass 4", "Cstrike_Guerilla_Label", "Cstrike_GIGN_Label",
			"militia",  "spetsnaz", "joinclass 5", "Cstrike_Militia_Label", "Cstrike_Spetsnaz_Label",
			"ct_random", "t_random", "joinclass 6", "Cstrike_AutoSelect_Label", "Cstrike_AutoSelect_Label",
		};


		bool showModel = true;
		int idx;
		for( idx = 0; idx < 5 * 6; idx += 5 )
		{
			if( !strcmp( sz, table[idx] ) )
			{
				command = table[idx + 2];
				loctext = L( table[idx + 3] );
				break;
			}

			if( !strcmp( sz, table[idx + 1] ) )
			{
				command = table[idx + 2];
				loctext = L( table[idx + 4] );
				break;
			}
		}

		if( idx >= 5 * 5 )
			showModel = false;

		if( !loctext )
			loctext = "";

		Q_strncpy( textbuffer, loctext, sizeof( textbuffer ) );

		if( showModel && ui_showclassmodels && ui_showclassmodels->value ) // try to load model
		{
			player.ent = EngFuncs::GetPlayerModel();
			if( player.ent )
			{
				snprintf( model, sizeof( model ), "models/player/%s/%s.mdl", sz, sz );
				player.hPlayerImage = 0;
				player.eOverrideMode = CMenuPlayerModelView::PMV_SHOWMODEL;
				EngFuncs::SetModel( player.ent, model );
				if( !player.ent->model )
				{
					showModel = false;
				}
			}
			else
			{
				showModel = false;
			}
		}
		else
		{
			showModel = false;
		}

		if( !showModel )
		{
			snprintf( model, sizeof( model ), "gfx/vgui/%s.tga", sz );

			player.hPlayerImage = EngFuncs::PIC_Load( model );
			player.eOverrideMode = CMenuPlayerModelView::PMV_SHOWIMAGE;
		}
	}

	void classPressedCb( void *pExtra )
	{
		classFocusCb( pExtra );
		ConfirmSelection();
	}
};

static class CClientJoinClassT: public CClientJoinClass
{
	typedef CClientJoinClass BaseClass;
public:
	void _Init();
} uiJoinClassT;

static class CClientJoinClassCT: public CClientJoinClass
{
	typedef CClientJoinClass BaseClass;
public:
	void _Init();
} uiJoinClassCT;

void CClientJoinClass::_Init()
{

	player.SetRect( 400, 180, 400, 284 );
	player.backgroundColor = uiColorBlack;
	player.colorStroke = uiPromptTextColor;
	player.iStrokeWidth = 1;
	player.eFocusAnimation = QM_NOFOCUSANIMATION;
	player.bDrawAsPlayer = false;

	text.SetRect( 400, 500, 500, 200 );
	text.SetBackground( 0U );
	text.iFlags |= QMF_INACTIVE;
	text.SetCharSize( QM_TINYFONT );
	text.szName = textbuffer;

	szName = L( "Cstrike_Join_Class" );
	AddItem( player );
	AddItem( text );
}

void CClientJoinClass::Reload()
{
	keys[0].Reset();

	textbuffer[0] = 0;
	player.eOverrideMode = CMenuPlayerModelView::PMV_SHOWIMAGE;
	player.hPlayerImage = 0;
	player.ent = EngFuncs::GetPlayerModel();
	if( player.ent )
	{
		player.ent->angles[1] += 15;
	}

	if( m_szDefaultClass )
		classFocusCb( (void*)m_szDefaultClass );
}


CMenuAction *CClientJoinClass::AddButton( int key, const char *name, const char *modelname, Point pos )
{
	CMenuAction *act = CClientWindow::AddButton( key, name, pos, MakeCb( modelname ) );
	act->onGotFocus = MakeFocusCb( modelname );
	return act;
}

void CClientJoinClassT::_Init()
{
	m_szDefaultClass = "terror";
	AddButton( '1', L( "Cstrike_Terror" ), "terror",
		Point( 100, 180 ));
	AddButton( '2', L( "Cstrike_L337_Krew" ), "leet",
		Point( 100, 230 ));
	AddButton( '3', L( "Cstrike_Arctic" ), "arctic",
		Point( 100, 280 ));
	AddButton( '4', L( "Cstrike_Guerilla" ), "guerilla",
		Point( 100, 330 ));
	if( hasCzero )
		AddButton( '5', L( "Cstrike_Militia" ), "militia",
			Point( 100, 380 ));
	AddButton( '6', L( "Cstrike_Auto_Select" ), "t_random",
		Point( 100, 430 ));

	BaseClass::_Init();
}

void CClientJoinClassCT::_Init()
{
	m_szDefaultClass = "urban";
	AddButton( '1', L( "Cstrike_Urban" ), "urban",
		Point( 100, 180 ));
	AddButton( '2', L( "Cstrike_GSG9" ), "gsg9",
		Point( 100, 230 ));
	AddButton( '3', L( "Cstrike_SAS" ), "sas",
		Point( 100, 280 ));
	AddButton( '4', L( "Cstrike_GIGN" ), "gign",
		Point( 100, 330 ));
	if( hasCzero )
		AddButton( '5', L( "Cstrike_Spetsnaz" ), "spetsnaz",
			Point( 100, 380 ));
	AddButton( '6', L( "Cstrike_Auto_Select" ), "ct_random",
		Point( 100, 430 ));

	BaseClass::_Init();
}

void UI_JoinClassT_Show( int param1, int param2 )
{
	uiJoinClassT.hasCzero = param1;
	EngFuncs::KEY_SetDest( KEY_MENU );
	uiJoinClassT.Show();
}

void UI_JoinClassCT_Show( int param1, int param2 )
{
	uiJoinClassCT.hasCzero = param1;
	EngFuncs::KEY_SetDest( KEY_MENU );
	uiJoinClassCT.Show();
}
