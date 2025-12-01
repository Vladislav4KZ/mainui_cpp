/*
Copyright (C) 2025 Vladislav Sukhov

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
#include "YesNoMessageBox.h"
#include "Table.h"
#include "StringVectorModel.h"
#include "keydefs.h"
#include "enginecallback_menu.h"
#include <ctype.h>

#define ART_BANNER "gfx/shell/head_language"

class CMenuLanguage: public CMenuFramework
{
private:
	void _Init( void ) override;

	// callbacks
	void OnOk();
	void OnCancel();
	void OnConfirmRestart();
	void OnSelectionChanged();

	// helpers
	void FillLanguages();

	CStringVectorModel m_model;
	CMenuTable m_table;
	CUtlVector<CUtlString> m_tokens; // parallel array of language tokens
    CMenuPicButton *m_btnActivate;

public:
	CMenuYesNoMessageBox msgBox;

	CMenuLanguage() : CMenuFramework("CMenuLanguage") { }
};

void CMenuLanguage::_Init( void )
{
	banner.SetPicture( ART_BANNER );

	AddItem( banner );

	// language list
	m_table.iFlags |= QMF_DROPSHADOW;
	m_table.SetRect( 360, 210, -20, 465 );
	m_table.SetModel( &m_model );
	AddItem( m_table );

	m_btnActivate = AddButton( L( "Activate" ), nullptr, PC_ACTIVATE, VoidCb( &CMenuLanguage::OnOk ), QMF_NOTIFY );
	AddButton( L( "GameUI_Cancel" ), nullptr, PC_CANCEL, VoidCb( &CMenuLanguage::OnCancel ), QMF_NOTIFY );

	msgBox.SetMessage( L( "Changing language will cause the engine to restart.\nRestart now?" ) );
	msgBox.onPositive = VoidCb( &CMenuLanguage::OnConfirmRestart );
	msgBox.Link( this );

    m_table.onChanged = VoidCb( &CMenuLanguage::OnSelectionChanged );
	FillLanguages();
}

// check that directory basename (like "gamedir_language") contains resource/<prefix>_<lang>.txt for allowed prefixes
static bool HasLocalizationResourcesInDir( const char *dirBasename, const char *lang, const char *prefixes[], int prefixCount )
{
	if( !dirBasename || !dirBasename[0] || !lang || !lang[0] ) return false;

	char checkpath[256];
	for( int i = 0; i < prefixCount; i++ )
	{
		const char *pref = prefixes[i];
		if( !pref || !pref[0] ) continue;

		snprintf( checkpath, sizeof( checkpath ), "%s/resource/%s_%s.txt", dirBasename, pref, lang );
		if( EngFuncs::FileExists( checkpath, false ) )
			return true;
	}
	return false;
}

void CMenuLanguage::FillLanguages()
{
	m_model.RemoveAll();

	// scan localization files inside current gamedir with specific prefixes
	const char *gamedir = gMenu.m_gameinfo.gamefolder;
	const char *prefixes[] = { "valve", "gameui", "mainui", gamedir };
	int prefixCount = sizeof(prefixes) / sizeof(prefixes[0]);

	// temporary map to hold token->display
	struct langpair_t { CUtlString token; CUtlString display; };
	CUtlVector<langpair_t> langs;

	for( int prefixIndex = 0; prefixIndex < prefixCount; prefixIndex++ )
	{
		const char *pref = prefixes[prefixIndex];
		if( !pref || !pref[0] ) continue;

		int numFiles = 0;
		char pattern[128];
		snprintf( pattern, sizeof(pattern), "resource/%s_*.txt", pref );
		char **filenames = EngFuncs::GetFilesList( pattern, &numFiles, true ); // gamedir only

		for( int i = 0; i < numFiles; i++ )
		{
			const char *fname = filenames[i];
			// expect resource/<prefix>_<lang>.txt
			const char *p = strrchr( fname, '/' );
			if( !p ) p = fname; else p++;

			// ensure prefix matches
			size_t plen = strlen( pref );
			if( strnicmp( p, pref, plen ) ) continue;
			const char *us = strchr( p + plen, '_' );
			if( !us ) us = strchr( p + plen, '_' );
			// find dot
			const char *dot = strrchr( p, '.' );
			if( !dot || us >= dot ) continue;

			// token between underscore and dot
			char token[64];
			int len = dot - (us + 1);
			if( len <= 0 || len >= (int)sizeof(token) ) continue;
			Q_strncpy( token, us + 1, len + 1 );

			// avoid duplicates
			bool found = false;
			for( int k = 0; k < langs.Count(); k++ )
			{
				if( !stricmp( langs[k].token.Get(), token )) { found = true; break; }
			}
			if( found ) continue;

			// Use token-derived display name (capitalize first letter).
			char cap[64];
			Q_strncpy( cap, token, sizeof( cap ) );
			// normalize: make lowercase then uppercase first letter
			for( char *cc = cap; *cc; cc++ ) *cc = tolower( (unsigned char)*cc );
			if( cap[0] ) cap[0] = toupper( (unsigned char)cap[0] );
			CUtlString displayName( cap );

			langpair_t lp;
			lp.token = token;
			lp.display = displayName.Get();
			langs.AddToTail( lp );
		}
		// filenames returned by engine are managed by engine; don't free here
	}

	if( gamedir && gamedir[0] )
	{
		int numDirs = 0;
		char pattern2[128];
		// look for entries like <gamedir>_* (we only need to check the engine root directory)
		snprintf( pattern2, sizeof(pattern2), "%s_*", gamedir );
		char **dirnames = EngFuncs::GetFilesList( pattern2, &numDirs, false );
        // HACKHACK: check localization folders inside gamedir
		int numDirsGamedir = 0;
		char **dirnamesGamedir = EngFuncs::GetFilesList( pattern2, &numDirsGamedir, true );

		for( int i = 0; i < numDirs; i++ )
		{
			const char *dname = dirnames[i];

			// localization folders must not be inside the gamedir, so exclude them
			bool inGamedir = false;
			for( int gi = 0; gi < numDirsGamedir; gi++ )
			{
				const char *gname = dirnamesGamedir[gi];
				if( gname && dname && !stricmp( gname, dname ) ) { inGamedir = true; break; }
			}
			if( inGamedir )
				continue;

			const char *p = strrchr( dname, '/' );
			if( !p ) p = dname; else p++;
			// ensure prefix matches gamedir
			size_t plen = strlen( gamedir );
			if( strnicmp( p, gamedir, plen ) ) continue;
			const char *us = strchr( p + plen, '_' );
			if( !us ) continue;
			// token goes until end or dot
			const char *dot = strrchr( p, '.' );
			const char *end = dot ? dot : p + strlen( p );
			int len = end - (us + 1);
			if( len <= 0 || len >= 64 ) continue;
			char token[64];
			Q_strncpy( token, us + 1, len + 1 );

			// verify that inside this <gamedir>_<lang> directory there is at least
			// one resource/<prefix>_<lang>.txt file for allowed prefixes
			if( !HasLocalizationResourcesInDir( p, token, prefixes, prefixCount )) continue;

			// avoid duplicates
			bool found = false;
			for( int k = 0; k < langs.Count(); k++ )
			{
				if( !stricmp( langs[k].token.Get(), token )) { found = true; break; }
			}
			if( found ) continue;

			char cap[64];
			Q_strncpy( cap, token, sizeof( cap ) );
			for( char *cc = cap; *cc; cc++ ) *cc = tolower( (unsigned char)*cc );
			if( cap[0] ) cap[0] = toupper( (unsigned char)cap[0] );
			CUtlString displayName( cap );

			langpair_t lp;
			lp.token = token;
			lp.display = displayName.Get();
			langs.AddToTail( lp );
		}
		// engine manages memory for returned list
	}

	m_tokens.Purge();
	// ensure English is always available even if no *_english.txt exists
	bool haveEnglish = false;
	for( int i = 0; i < langs.Count(); i++ )
	{
		if( !stricmp( langs[i].token.Get(), "english" )) { haveEnglish = true; break; }
	}
	if( !haveEnglish )
	{
		langpair_t lp;
		lp.token = "english";
		lp.display = "English";
		// insert English at beginning of the list
		langs.InsertBefore( 0, lp );
	}

	for( int i = 0; i < langs.Count(); i++ )
	{
		m_model.AddToTail( langs[i].display.Get() );
		m_tokens.AddToTail( langs[i].token );
	}

	// select current language if present
	const char *cur = EngFuncs::GetCvarString( "ui_language" );
	if( cur && cur[0] )
	{
		for( int i = 0; i < m_model.GetRows(); i++ )
		{
			if( i < m_tokens.Count() )
			{
				const char *token = m_tokens[i].Get();
				if( token && !stricmp( token, cur ))
				{
					m_table.SetCurrentIndex( i );
					if( m_table.onChanged ) m_table.onChanged( &m_table );
					break;
				}
			}
		}
	}

	m_table.SetModel( &m_model );
}

void CMenuLanguage::OnSelectionChanged()
{
	int idx = m_table.GetCurrentIndex();
	const char *curLang = EngFuncs::GetCvarString( "ui_language" );

	if( !m_btnActivate ) return;

	if( idx < 0 || idx >= m_tokens.Count() )
	{
		m_btnActivate->SetGrayed( true );
		m_btnActivate->SetInactive( true );
		return;
	}

	const char *token = m_tokens[idx].Get();
	if( token && curLang && !stricmp( token, curLang ) )
	{
		m_btnActivate->SetGrayed( true );
		m_btnActivate->SetInactive( true );
	}
	else
	{
		m_btnActivate->SetGrayed( false );
		m_btnActivate->SetInactive( false );
	}
}

void CMenuLanguage::OnOk()
{
	// ask for confirmation
	msgBox.Show();
}

void CMenuLanguage::OnCancel()
{
	Hide();
}

void CMenuLanguage::OnConfirmRestart()
{
	int idx = m_table.GetCurrentIndex();
	if( idx < 0 || idx >= m_model.GetRows() )
		return;

	// token stored in m_tokens
	const char *token = NULL;
	if( idx < m_tokens.Count() ) token = m_tokens[idx].Get();
	if( !token || !token[0] ) return;

	// request engine to restart into same gamedir and with preserved args via console command 'language'
	char cmd[256];
	snprintf( cmd, sizeof( cmd ), "language %s\n", token );
	EngFuncs::ClientCmd( false, cmd );
}

ADD_MENU( menu_language, CMenuLanguage, UI_Language_Menu );
