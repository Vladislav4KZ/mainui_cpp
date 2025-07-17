/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2025 APAmk2

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
#include "PicButton.h"
#include "Slider.h"
#include "CheckBox.h"

#define ART_BANNER "gfx/shell/head_xhair"

class CMenuCrosshair : public CMenuFramework
{
public:
	CMenuCrosshair() :
	    CMenuFramework( "CMenuCrosshair" ) { }

private:
	void _Init() override;
	void _VidInit() override;

public:
	void SaveButton();
	void SaveCrosshair();

	static void ExitMenuCb( CMenuBaseItem *pSelf, void *pExtra );

	CMenuSlider red;
	CMenuSlider green;
	CMenuSlider blue;
	CMenuSlider alpha;

	CMenuSlider gap;
	CMenuSlider pad;
	CMenuSlider size;
	CMenuSlider thick;
	CMenuSlider dynScale;

	CMenuCheckBox newXhair;
	CMenuCheckBox dot;
	CMenuCheckBox tShape;
	CMenuCheckBox dynMove;
	CMenuCheckBox weapGap;

	CMenuPicButton save;
	CMenuPicButton done;

	class CMenuCrosshairPreview : public CMenuBaseItem
	{
	public:
		void Draw() override;
		void DrawCrosshairPadding( int _pad, int _x0, int _y0, int _x1, int _y1, CMenuCrosshair *parent );
		void DrawCrosshairSection( int _x0, int _y0, int _x1, int _y1, CMenuCrosshair *parent );
		int ScaleForRes( float value, int height );

		HIMAGE hImage;
	} crosshair;
};

ADD_MENU3( menu_crosshair, CMenuCrosshair, UI_Crosshair_Menu );

void CMenuCrosshair::SaveCrosshair()
{
	EngFuncs::ClientCmdF( true, "xhair_color \"%u %u %u %u\"\n",
	                      (uint)red.GetCurrentValue(),
	                      (uint)green.GetCurrentValue(),
	                      (uint)blue.GetCurrentValue(),
	                      (uint)alpha.GetCurrentValue() );

	gap.WriteCvar();
	pad.WriteCvar();
	size.WriteCvar();
	thick.WriteCvar();
	dynScale.WriteCvar();

	newXhair.WriteCvar();
	dot.WriteCvar();
	tShape.WriteCvar();
	dynMove.WriteCvar();
	weapGap.WriteCvar();

	Hide();
}

/*
=================
UI_Crosshair_Init
=================
*/
void CMenuCrosshair::_Init( void )
{
	banner.SetPicture( ART_BANNER );

	red.eFocusAnimation = QM_PULSEIFFOCUS;
	red.SetNameAndStatus( L( "Red:" ), L( "Texture red channel" ) );
	red.Setup( 0, 255, 1 );

	green.eFocusAnimation = QM_PULSEIFFOCUS;
	green.SetNameAndStatus( L( "Green:" ), L( "Texture green channel" ) );
	green.Setup( 0, 255, 1 );

	blue.eFocusAnimation = QM_PULSEIFFOCUS;
	blue.SetNameAndStatus( L( "Blue:" ), L( "Texture blue channel" ) );
	blue.Setup( 0, 255, 1 );

	alpha.eFocusAnimation = QM_PULSEIFFOCUS;
	alpha.SetNameAndStatus( L( "Alpha:" ), L( "Texture alpha channel" ) );
	alpha.Setup( 0, 255, 1 );

	newXhair.SetNameAndStatus( L( "Enhanced crosshair" ), L( "Enables enhanced crosshair" ) );
	newXhair.LinkCvar( "xhair_enable" );

	dynMove.SetNameAndStatus( L( "Dynamic move" ), L( "Jumping, crouching and moving will affect the dynamic crosshair" ) );
	dynMove.LinkCvar( "xhair_dynamic_move" );

	gap.eFocusAnimation = QM_PULSEIFFOCUS;
	gap.SetNameAndStatus( L( "Gap:" ), L( "Space between crosshair's lines" ) );
	gap.Setup( 0, 13, 1 );

	pad.eFocusAnimation = QM_PULSEIFFOCUS;
	pad.SetNameAndStatus( L( "Padding:" ), L( "Border around crosshair" ) );
	pad.Setup( 0, 16, 1 );

	size.eFocusAnimation = QM_PULSEIFFOCUS;
	size.SetNameAndStatus( L( "Size:" ), L( "Crosshair size" ) );
	size.Setup( 0, 32, 1 );

	thick.eFocusAnimation = QM_PULSEIFFOCUS;
	thick.SetNameAndStatus( L( "Thickness:" ), L( "Crosshair thickness" ) );
	thick.Setup( 0, 11, 1 );

	dynScale.eFocusAnimation = QM_PULSEIFFOCUS;
	dynScale.SetNameAndStatus( L( "Dynamic scale:" ), L( "Scale of the dynamic crosshair movement" ) );
	dynScale.Setup( 0, 2.0f, 0.1f );

	dot.SetNameAndStatus( L( "Dot" ), L( "Enables crosshair dot" ) );
	dot.LinkCvar( "xhair_dot" );

	tShape.SetNameAndStatus( L( "T-Shape" ), L( "Enables T-shaped crosshair" ) );
	tShape.LinkCvar( "xhair_t" );

	weapGap.SetNameAndStatus( L( "Weapon Gap" ), L( "Makes the crosshair gap scale depend on the active weapon" ) );
	weapGap.LinkCvar( "xhair_gap_useweaponvalue" );

	save.szName = L( "Done" );
	save.SetPicture( PC_DONE );
	save.onReleased = VoidCb( &CMenuCrosshair::SaveCrosshair );

	done.szName = L( "GameUI_Cancel" );
	done.SetPicture( PC_CANCEL );
	done.onReleased = VoidCb( &CMenuCrosshair::Hide );

	crosshair.szName = L( "GameUI_CrosshairDescription" );
	crosshair.SetRect( 700, 230, 200, 200 );
	crosshair.hImage = EngFuncs::PIC_Load( "gfx/vgui/crosshair.tga", 0 );

	AddItem( banner );
	AddItem( red );
	AddItem( green );
	AddItem( blue );
	AddItem( alpha );
	AddItem( newXhair );
	AddItem( dynMove );

	AddItem( gap );
	AddItem( pad );
	AddItem( size );
	AddItem( thick );
	AddItem( dynScale );

	AddItem( dot );
	AddItem( tShape );
	AddItem( weapGap );

	AddItem( save );
	AddItem( done );
	AddItem( crosshair );
}

void CMenuCrosshair::_VidInit()
{
	red.SetCoord( 72, 230 );
	green.SetCoord( 72, 290 );
	blue.SetCoord( 72, 350 );
	alpha.SetCoord( 72, 410 );
	newXhair.SetCoord( 72, 460 );
	dynMove.SetCoord( 72, 510 );

	int cvarColor[3] = { 0, 255, 0 };
	float cvarAlpha = 255.0f;
	if ( sscanf( EngFuncs::GetCvarString( "xhair_color" ), "%d %d %d %f", &cvarColor[0], &cvarColor[1], &cvarColor[2], &cvarAlpha ) == 4 )
	{
		red.SetCurrentValue( bound( 0, cvarColor[0], 255 ) );
		green.SetCurrentValue( bound( 0, cvarColor[1], 255 ) );
		blue.SetCurrentValue( bound( 0, cvarColor[2], 255 ) );
		alpha.SetCurrentValue( bound( 0.0f, cvarAlpha, 255.0f ) );
	}

	gap.SetCoord( 404, 230 );
	gap.LinkCvar( "xhair_gap" );

	pad.SetCoord( 404, 290 );
	pad.LinkCvar( "xhair_pad" );

	size.SetCoord( 404, 350 );
	size.LinkCvar( "xhair_size" );

	thick.SetCoord( 404, 410 );
	thick.LinkCvar( "xhair_thick" );

	dynScale.SetCoord( 404, 470 );
	dynScale.LinkCvar( "xhair_dynamic_scale" );

	dot.SetCoord( 404, 520 );
	tShape.SetCoord( 404, 570 );
	weapGap.SetCoord( 404, 620 );

	save.SetCoord( 72, 630 );
	done.SetCoord( 72, 680 );
}

/*
=================
UI_Crosshair_Menu
=================
*/
void UI_Crosshair_Menu( void )
{
	menu_crosshair->Show();
}

int CMenuCrosshair::CMenuCrosshairPreview::ScaleForRes( float value, int height )
{
	/* "default" resolution is 640x480 */
	return rint( value * ( (float)height / 480.0f ) );
}

void CMenuCrosshair::CMenuCrosshairPreview::DrawCrosshairSection( int _x0, int _y0, int _x1, int _y1, CMenuCrosshair *parent )
{
	float x0 = (float)_x0;
	float y0 = (float)_y0;
	float x1 = (float)_x1 - _x0;
	float y1 = (float)_y1 - _y0;

	uint color = ( (uint)parent->blue.GetCurrentValue() ) | ( ( (uint)parent->green.GetCurrentValue() ) << 8 ) | ( ( (uint)parent->red.GetCurrentValue() ) << 16 ) | ( ( (uint)parent->alpha.GetCurrentValue() ) << 24 );

	UI_FillRect( x0, y0, x1, y1, color );
}

void CMenuCrosshair::CMenuCrosshairPreview::DrawCrosshairPadding( int _pad, int _x0, int _y0, int _x1, int _y1, CMenuCrosshair *parent )
{
	float pad = (float)_pad;
	float x0 = (float)_x0;
	float y0 = (float)_y0;
	float x1 = (float)_x1 - _x0;
	float y1 = (float)_y1 - _y0;

	uint color = PackRGBA( 0, 0, 0, (uint)parent->alpha.GetCurrentValue() );

	UI_FillRect( x0 - pad, y0 - pad, x1 + 2 * pad, pad, color ); // top part
	UI_FillRect( x0 - pad, y0 + y1, x1 + 2 * pad, pad, color ); // bottom part
	UI_FillRect( x0 - pad, y0, pad, y1, color ); // left part
	UI_FillRect( x0 + x1, y0, pad, y1, color ); // right part
}

void CMenuCrosshair::CMenuCrosshairPreview::Draw()
{
	CMenuCrosshair *parent = (CMenuCrosshair *)Parent();

	int center_x, center_y;
	int gap, length, thickness;
	int y0, y1, x0, x1;
	wrect_t inner;
	wrect_t outer;

	/* calculate the coordinates */
	center_x = m_scPos.x + ( m_scSize.w / 2 );
	center_y = m_scPos.y + ( m_scSize.h / 2 );

	gap = ScaleForRes( 4 + parent->gap.GetCurrentValue(), ScreenHeight );
	length = ScaleForRes( parent->size.GetCurrentValue(), ScreenHeight );
	thickness = ScaleForRes( parent->thick.GetCurrentValue(), ScreenHeight );
	thickness = Q_max( 1, thickness );

	inner.left = ( center_x - gap - thickness / 2 );
	inner.right = ( inner.left + 2 * gap + thickness );
	inner.top = ( center_y - gap - thickness / 2 );
	inner.bottom = ( inner.top + 2 * gap + thickness );

	outer.left = ( inner.left - length );
	outer.right = ( inner.right + length );
	outer.top = ( inner.top - length );
	outer.bottom = ( inner.bottom + length );

	y0 = ( center_y - thickness / 2 );
	x0 = ( center_x - thickness / 2 );
	y1 = ( y0 + thickness );
	x1 = ( x0 + thickness );

	// APAMk2 - Draw Crosshair's BG before Crosshair itself
	if ( !hImage )
	{
		UI_FillRect( m_scPos, m_scSize, uiPromptBgColor );
	}
	else
	{
		EngFuncs::PIC_Set( hImage, 255, 255, 255 );
		EngFuncs::PIC_DrawTrans( m_scPos, m_scSize );
	}
	int textHeight = m_scPos.y - ( m_scChSize * 1.5f );
	uint textflags = ( iFlags & QMF_DROPSHADOW ) ? ETF_SHADOW : 0;
	UI_DrawString( font, m_scPos.x, textHeight, m_scSize.w, m_scChSize, szName, uiColorHelp, m_scChSize, QM_LEFT, textflags | ETF_FORCECOL );

	/* draw padding if wanted */
	if ( parent->pad.GetCurrentValue() )
	{
		/* don't scale this */
		int pad = (int)parent->pad.GetCurrentValue();

		if ( parent->dot.bChecked )
			DrawCrosshairPadding( pad, x0, y0, x1, y1, parent );

		if ( !parent->tShape.bChecked )
			DrawCrosshairPadding( pad, x0, outer.top, x1, inner.top, parent );

		DrawCrosshairPadding( pad, x0, inner.bottom, x1, outer.bottom, parent );
		DrawCrosshairPadding( pad, outer.left, y0, inner.left, y1, parent );
		DrawCrosshairPadding( pad, inner.right, y0, outer.right, y1, parent );
	}

	if ( parent->dot.bChecked )
		DrawCrosshairSection( x0, y0, x1, y1, parent );

	if ( !parent->tShape.bChecked )
		DrawCrosshairSection( x0, outer.top, x1, inner.top, parent );

	DrawCrosshairSection( x0, inner.bottom, x1, outer.bottom, parent );
	DrawCrosshairSection( outer.left, y0, inner.left, y1, parent );
	DrawCrosshairSection( inner.right, y0, outer.right, y1, parent );
}