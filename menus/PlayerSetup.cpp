/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "Framework.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "CheckBox.h"
#include "Slider.h"
#include "Field.h"
#include "SpinControl.h"
#include "YesNoMessageBox.h"
#include "PlayerModelView.h"
#include "StringArrayModel.h"
#include "StringVectorModel.h"

#define ART_BANNER		"gfx/shell/head_customize"

#define MAX_PLAYERMODELS	100

static struct
{
	const char *name;
	int r, g, b;
} g_LogoColors[] =
{
{ "FullColor",     -1,  -1,  -1  },
{ "#Valve_Orange", 255, 120, 24  }, // L( "#Valve_Orange" )
{ "#Valve_Yellow", 225, 180, 24  }, // L( "#Valve_Yellow" )
{ "#Valve_Blue",   0,   60,  255 }, // L( "#Valve_Blue" )
{ "#Valve_Ltblue", 0,   167, 255 }, // L( "#Valve_Ltblue" )
{ "#Valve_Green",  0,   167, 0   }, // L( "#Valve_Green" )
{ "#Valve_Red",    255, 43,  0   }, // L( "#Valve_Red" )
{ "#Valve_Brown",  123, 73,  0   }, // L( "#Valve_Brown" )
{ "#Valve_Ltgray", 100, 100, 100 }, // L( "#Valve_Ltgray" )
{ "#Valve_Dkgray", 36,  36,  36  }, // L( "#Valve_Dkgray" )
};

static struct
{
	const char *name;
	int r, g, b;
} g_CrosshairColors[] =
{
{ "#Valve_Green",	50,		250,	50 	},
{ "#Valve_Red",		250,	50,		50 	},
{ "#Valve_Blue",	50,		50,		250 },
{ "#Valve_Yellow",	250,	250,	50 	},
{ "#Valve_Ltblue",	50,		250,	250	},
};

class CMenuPlayerSetup : public CMenuFramework
{
private:
	void _Init() override;
	void Reload() override;
public:
	CMenuPlayerSetup() : CMenuFramework( "CMenuPlayerSetup" ), msgBox( true ) { }

	void SetConfig();
	void UpdateModel();
	void UpdateLogo();
	void ApplyColorToImagePreview();
	void ApplyColorToLogoPreview();
	void WriteNewLogo();
	void SaveAndPopMenu() override;

	class CModelListModel : public CStringArrayModel
	{
	public:
		CModelListModel() : CStringArrayModel( (const char *)models, CS_SIZE, 0 ) {}
		void Update();

	private:
		char models[MAX_PLAYERMODELS][CS_SIZE];
	} modelsModel;

	class CLogosListModel : public CStringVectorModel
	{
	public:
		void Update() override;

		int GetFullPath( char *buf, size_t size, int pos )
		{
			const char *file, *ext;

			file = Element( pos ).String();
			ext = IsPng( pos ) ? "png" : "bmp";

			return snprintf( buf, size, "logos/%s.%s", file, ext );
		}

		bool IsPng( int pos )
		{
			return m_isPngs[pos];
		}

	private:
		CUtlVector<bool> m_isPngs;
	} logosModel;

	CMenuPlayerModelView	view;

	CMenuCheckBox	showModels;
	CMenuCheckBox	hiModels;
	CMenuSlider	topColor;
	CMenuSlider	bottomColor;

	CMenuField	name;
	CMenuSpinControl	model;

	class CMenuLogoPreview : public CMenuBaseItem
	{
	public:
		virtual void Draw();
		int r, g, b;
		HIMAGE hImage;
	} logoImage;

	CMenuSpinControl	logo;
	CMenuSpinControl	logoColor;

	CMenuYesNoMessageBox msgBox;

	class CMenuCrosshairPreview : public CMenuBaseItem
	{
	public:
		virtual void Draw();
		int r, g, b;
		HIMAGE hImage;
	} crosshairPreview;

	CMenuSpinControl crosshairSize;
	CMenuSpinControl crosshairColor;
	CMenuCheckBox crosshairTranslucent;

	bool hideModels, hideLogos;
};

void CMenuPlayerSetup::CMenuLogoPreview::Draw()
{
	if( !hImage )
	{
		// draw the background
		UI_FillRect( m_scPos, m_scSize, uiPromptBgColor );

		UI_DrawString( font, m_scPos, m_scSize, L( "No logo" ), colorBase, m_scChSize, QM_CENTER, ETF_SHADOW );
	}
	else
	{
		if( r != -1 && g != -1 && b != -1 )
			EngFuncs::PIC_Set( hImage, r, g, b );
		else
			EngFuncs::PIC_Set( hImage, 255, 255, 255 );
		EngFuncs::PIC_DrawTrans( m_scPos, m_scSize );
	}

	// draw the rectangle
	if( eFocusAnimation == QM_HIGHLIGHTIFFOCUS && IsCurrentSelected() )
		UI_DrawRectangle( m_scPos, m_scSize, uiInputTextColor );
	else
		UI_DrawRectangle( m_scPos, m_scSize, uiInputFgColor );

}

void CMenuPlayerSetup::CMenuCrosshairPreview::Draw()
{
	if( !hImage )
	{
		UI_FillRect( m_scPos, m_scSize, uiPromptBgColor );
	}
	else
	{
		EngFuncs::PIC_Set( hImage, 255, 255, 255 );
		EngFuncs::PIC_DrawTrans( m_scPos, m_scSize );
	}

	// draw the rectangle
	if( eFocusAnimation == QM_HIGHLIGHTIFFOCUS && IsCurrentSelected() )
		UI_DrawRectangle( m_scPos, m_scSize, uiInputTextColor );
	else
		UI_DrawRectangle( m_scPos, m_scSize, uiInputFgColor );

}

/*
=================
UI_PlayerSetup_FindModels
=================
*/
void CMenuPlayerSetup::CModelListModel::Update( void )
{
	char	name[256];
	char	**filenames;
	int numFiles, i;
	
	m_iCount = 0;

	// Get file list
	// search in basedir too, because that's how GoldSrc does this
	filenames = EngFuncs::GetFilesList(  "models/player/*", &numFiles, FALSE );

#if 0
	// add default singleplayer model
	strcpy( models[num_models], "player" );
	modelsPtr[num_models] = models[num_models];
	num_models++;
#endif

	// build the model list
	for( i = 0; i < numFiles; i++ )
	{
		COM_FileBase( filenames[i], name );
		Q_strncpy( models[m_iCount], name, sizeof( models[0] ) );
		
		// check if the path is a valid model
		snprintf( name, sizeof( name ), "models/player/%s/%s.mdl", models[m_iCount], models[m_iCount] );
		if( !EngFuncs::FileExists( name ) )
			continue;
		
		m_iCount++;
	}
}

/*
=================
CMenuPlayerSetup::FindLogos

=================
*/
void CMenuPlayerSetup::CLogosListModel::Update( )
{
	char	**filenames;
	int numFiles, i;

	// Get file list
	filenames = EngFuncs::GetFilesList( "logos/*.*", &numFiles, FALSE );

	if( !filenames || !numFiles )
	{
		m_isPngs.RemoveAll();
		RemoveAll();
		return;
	}

	// build the model list
	for( i = 0; i < numFiles; i++ )
	{
		CUtlString logoFileName = filenames[i];
		char temp[256];
		bool png;

		if(( png = logoFileName.BEndsWithCaseless( ".png" )) ||
			 logoFileName.BEndsWithCaseless( ".bmp" ))
		{
			COM_FileBase( logoFileName.String(), temp );

			if( !stricmp( temp, "remapped" ))
				continue;

			AddToTail( temp );
			m_isPngs.AddToTail( png );
		}
	}
}

/*
=================
UI_PlayerSetup_SetConfig
=================
*/
void CMenuPlayerSetup::SetConfig( void )
{
	name.WriteCvar();
	model.WriteCvar();
	topColor.WriteCvar();
	bottomColor.WriteCvar();
	hiModels.WriteCvar();
	showModels.WriteCvar();
	crosshairSize.WriteCvar();
	crosshairColor.WriteCvar();
	crosshairTranslucent.WriteCvar();
	WriteNewLogo();
}

void CMenuPlayerSetup::SaveAndPopMenu()
{
	if( !UI::Names::CheckIsNameValid( name.GetBuffer() ) )
	{
		msgBox.Show();
		return;
	}

	SetConfig();
	CMenuFramework::SaveAndPopMenu();
}

void CMenuPlayerSetup::UpdateModel()
{
	char image[256];
	const char *mdl = model.GetCurrentString();

	// seems we DON'T have this model locally
	// just force display string and do nothing
	if( !mdl )
	{
		model.ForceDisplayString( EngFuncs::GetCvarString( "model" ) );
		return;
	}

	snprintf( image, 256, "models/player/%s/%s.bmp", mdl, mdl );
	view.hPlayerImage = EngFuncs::PIC_Load( image, PIC_KEEP_SOURCE );

	ApplyColorToImagePreview();
	EngFuncs::CvarSetString( "model", mdl );
	if( !strcmp( mdl, "player" ) )
		strcpy( image, "models/player.mdl" );
	else
		snprintf( image, sizeof(image), "models/player/%s/%s.mdl", mdl, mdl );
	if( view.ent )
		EngFuncs::SetModel( view.ent, image );
}

void CMenuPlayerSetup::UpdateLogo()
{
	char filename[1024];
	int pos = logo.GetCurrentValue();

	if( pos < 0 )
		return;

	logosModel.GetFullPath( filename, sizeof( filename ), pos );
	logoImage.hImage = EngFuncs::PIC_Load( filename, 0 );
	if( logosModel.IsPng( pos ))
	{
		logoImage.r = logoImage.g = logoImage.b = -1;
		logoColor.SetGrayed( true );
	}
	else
	{
		CBMP *bmpFile = CBMP::LoadFile( filename );
		if( bmpFile->GetBitmapHdr()->bitsPerPixel == 8 )
		{
			ApplyColorToLogoPreview();
			logoColor.SetGrayed( false );
		}
		else
		{
			logoImage.r = logoImage.g = logoImage.b = -1;
			logoColor.SetGrayed( true );
		}
		delete bmpFile;
	}

	EngFuncs::CvarSetString( "cl_logofile", logo.GetCurrentString() );
	logoColor.WriteCvar();
}

void CMenuPlayerSetup::ApplyColorToImagePreview()
{
	EngFuncs::ProcessImage( view.hPlayerImage, -1,
		topColor.GetCurrentValue(), bottomColor.GetCurrentValue() );
}

void CMenuPlayerSetup::ApplyColorToLogoPreview()
{
	const char *logoColorStr = logoColor.GetCurrentString();

	for( size_t i = 0; i < V_ARRAYSIZE( g_LogoColors ) && logoColorStr; i++ )
	{
		if( !stricmp( logoColorStr, L( g_LogoColors[i].name )))
		{
			logoImage.r = g_LogoColors[i].r;
			logoImage.g = g_LogoColors[i].g;
			logoImage.b = g_LogoColors[i].b;
			return;
		}
	}

	logoColor.SetCurrentValue( L( g_LogoColors[0].name ) );
	logoImage.r = g_LogoColors[0].r;
	logoImage.g = g_LogoColors[0].g;
	logoImage.b = g_LogoColors[0].b;
}

void CMenuPlayerSetup::WriteNewLogo( void )
{
	char filename[1024];
	int pos = logo.GetCurrentValue();

	if( pos < 0 || hideLogos )
		return;

	EngFuncs::DeleteFile( "logos/remapped.png" );
	EngFuncs::DeleteFile( "logos/remapped.bmp" );

	logosModel.GetFullPath( filename, sizeof( filename ), pos );

	// TODO: check file size and throw a messagebox if it's too big?
	if( logosModel.IsPng( pos ))
	{
		int len;
		void *afile = EngFuncs::COM_LoadFile( filename, &len );

		// just copy file, nothing special
		EngFuncs::COM_SaveFile( "logos/remapped.png", afile, len );

		EngFuncs::COM_FreeFile( afile );

		EngFuncs::CvarSetString( "cl_logoext", "png" );
	}
	else
	{
		CBMP *bmpFile = CBMP::LoadFile( filename );

		// not valid logo BMP file
		if( !bmpFile )
			return;

		// remap logo if needed
		if( logoImage.r != -1 && logoImage.g != -1 && logoImage.b != -1 )
			bmpFile->RemapLogo( logoImage.r, logoImage.g, logoImage.b );

		EngFuncs::COM_SaveFile( "logos/remapped.bmp", bmpFile->GetBitmap(), bmpFile->GetBitmapHdr()->fileSize );
		EngFuncs::CvarSetString( "cl_logoext", "bmp" );

		delete bmpFile;
	}

	logo.WriteCvar();
	logoColor.WriteCvar();
}

/*
=================
UI_PlayerSetup_Init
=================
*/
void CMenuPlayerSetup::_Init( void )
{
	int addFlags = 0;

	hideModels = hideLogos = false;

	// disable playermodel preview for HLRally to prevent crash
	if( !stricmp( gMenu.m_gameinfo.gamefolder, "hlrally" ))
		hideModels = true;

	if( gMenu.m_gameinfo.flags & GFL_NOMODELS )
		addFlags |= QMF_INACTIVE;

	banner.SetPicture(ART_BANNER);

	name.szStatusText = L( "Enter your multiplayer display name" );
	name.iMaxLength = 32;
	name.LinkCvar( "name" );
	name.SetRect( 320, 260, 256, 36 );

	modelsModel.Update();
	if( !modelsModel.GetRows() )
	{
		model.SetVisibility( false );
		hideModels = true;
	}
	else
	{
		model.Setup( &modelsModel );
		model.LinkCvar( "model", CMenuEditable::CVAR_STRING );
		model.onChanged = VoidCb( &CMenuPlayerSetup::UpdateModel );
		model.SetRect( 660, 580 + UI_OUTLINE_WIDTH, 260, 32 );
	}

	topColor.iFlags |= addFlags;
	topColor.SetNameAndStatus( L( "GameUI_PrimaryColor" ), L( "Set a player model top color" ) );
	topColor.Setup( 0.0, 255, 1 );
	topColor.LinkCvar( "topcolor" );
	topColor.onCvarChange = CMenuEditable::WriteCvarCb;
	topColor.onChanged = VoidCb( &CMenuPlayerSetup::ApplyColorToImagePreview );
	topColor.SetCoord( 340, 520 );
	topColor.size.w = 300;

	bottomColor.iFlags |= addFlags;
	bottomColor.SetNameAndStatus( L( "GameUI_SecondaryColor" ), L( "Set a player model bottom color" ) );
	bottomColor.Setup( 0.0, 255.0, 1 );
	bottomColor.LinkCvar( "bottomcolor" );
	bottomColor.onCvarChange = CMenuEditable::WriteCvarCb;
	bottomColor.onChanged = VoidCb( &CMenuPlayerSetup::ApplyColorToImagePreview );;
	bottomColor.SetCoord( 340, 590 );
	bottomColor.size.w = 300;

	showModels.iFlags |= addFlags;
	showModels.SetNameAndStatus( L( "Show 3D preview" ), L( "Show 3D player models instead of preview thumbnails" ) );
	showModels.LinkCvar( "ui_showmodels" );
	showModels.onCvarChange = CMenuEditable::WriteCvarCb;
	showModels.SetCoord( 340, 380 );

	hiModels.iFlags |= addFlags;
	hiModels.SetNameAndStatus( L( "GameUI_HighModels" ), L( "Show HD models in multiplayer" ) );
	hiModels.LinkCvar( "cl_himodels" );
	hiModels.onCvarChange = CMenuEditable::WriteCvarCb;
	hiModels.SetCoord( 340, 430 );

	view.iFlags |= addFlags;
	view.SetRect( 660, 260, 260, 320 );

	msgBox.SetMessage( L( "Please, choose another player name" ) );
	msgBox.Link( this );

	AddItem( background );
	AddItem( banner );

	AddButton( L( "Done" ), L( "Go back to the Multiplayer Menu" ), PC_DONE, VoidCb( &CMenuPlayerSetup::SaveAndPopMenu ) );
	CMenuPicButton *gameOpt = AddButton( L( "Game options" ), L( "Configure handness, fov and other advanced options" ), PC_GAME_OPTIONS );
	SET_EVENT_MULTI( gameOpt->onReleased,
	{
		((CMenuPlayerSetup*)pSelf->Parent())->SetConfig();
		UI_AdvUserOptions_Menu();
	});

	AddButton( L( "Adv. Options" ), "", PC_ADV_OPT, UI_GameOptions_Menu );
	gameOpt->SetGrayed( !UI_AdvUserOptions_IsAvailable() );


	if( !hideLogos )
	{
		logosModel.Update();
		if( !logosModel.GetRows() )
		{
			// don't add to framework
			hideLogos = true;
		}
		else
		{
			static const char *itemlist[V_ARRAYSIZE( g_LogoColors )];
			static CStringArrayModel colors( itemlist, V_ARRAYSIZE( g_LogoColors ) );
			for( size_t i = 0; i < V_ARRAYSIZE( g_LogoColors ); i++ )
				itemlist[i] = L( g_LogoColors[i].name );

			logoImage.SetRect( 72, 230 + m_iBtnsNum * 50 + 10, 200, 200 );

			logo.Setup( &logosModel );
			logo.LinkCvar( "cl_logofile", CMenuEditable::CVAR_STRING );
			logo.onChanged = VoidCb( &CMenuPlayerSetup::UpdateLogo );
			logo.SetRect( 72, logoImage.pos.y + logoImage.size.h + UI_OUTLINE_WIDTH, 200, 32 );

			logoColor.Setup( &colors );
			logoColor.LinkCvar( "cl_logocolor", CMenuEditable::CVAR_STRING );
			logoColor.onChanged = VoidCb( &CMenuPlayerSetup::ApplyColorToLogoPreview );;
			logoColor.SetRect( 72, logo.pos.y + logo.size.h + UI_OUTLINE_WIDTH, 200, 32 );
		}
	}

	AddItem( name );
	if( !hideLogos )
	{
		UpdateLogo();
		AddItem( logo );
		AddItem( logoColor );
		AddItem( logoImage );
	}

	static const char *itemlist[V_ARRAYSIZE( g_CrosshairColors )];
	static CStringArrayModel colors( itemlist, V_ARRAYSIZE( g_CrosshairColors ) );
	for( size_t i = 0; i < V_ARRAYSIZE( g_CrosshairColors ); i++ )
		itemlist[i] = L( g_CrosshairColors[i].name );

	static const char *sizelist[] = { "Auto-size", "Small", "Medium", "Large" };
	static CStringArrayModel sizes( sizelist, V_ARRAYSIZE( sizelist ));

	crosshairPreview.SetNameAndStatus( "Crosshair appearance", NULL );
	crosshairPreview.SetRect( 302, 230 + m_iBtnsNum * 50 + 10, 200, 200 );
	crosshairPreview.hImage = EngFuncs::PIC_Load( "gfx/vgui/crosshair.tga", 0 );

	crosshairSize.Setup( &sizes );
	crosshairSize.LinkCvar( "cl_crosshair_size", CMenuEditable::CVAR_VALUE );
	crosshairSize.SetRect( 302, crosshairPreview.pos.y + crosshairPreview.size.h + UI_OUTLINE_WIDTH, 200, 32 );

	crosshairColor.Setup( &colors );
	// crosshairColor.LinkCvar( "cl_crosshair_color", CMenuEditable::CVAR_STRING );
	crosshairColor.SetRect( 302, crosshairSize.pos.y + crosshairSize.size.h + UI_OUTLINE_WIDTH, 200, 32 );

	crosshairTranslucent.SetNameAndStatus( "Translucent", NULL );
	crosshairTranslucent.LinkCvar( "cl_crosshair_translucent" );
	crosshairTranslucent.SetCoord( 302, crosshairColor.pos.y + crosshairColor.size.h + UI_OUTLINE_WIDTH );

	AddItem( crosshairPreview );
	AddItem( crosshairSize );
	AddItem( crosshairColor );
	AddItem( crosshairTranslucent );

	if( !(gMenu.m_gameinfo.flags & GFL_NOMODELS) )
	{
		AddItem( topColor );
		AddItem( bottomColor );
		AddItem( showModels );
		AddItem( hiModels );
		AddItem( model );
		// disable playermodel preview for HLRally to prevent crash
		if( !hideModels )
		{
			AddItem( view );
		}
	}
}

void CMenuPlayerSetup::Reload()
{
	if( !hideLogos ) UpdateLogo();
	if( !hideModels ) UpdateModel();
}

ADD_MENU( menu_playersetup, CMenuPlayerSetup, UI_PlayerSetup_Menu );
