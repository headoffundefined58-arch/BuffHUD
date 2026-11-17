#pragma once

namespace BuffHUD::Menu
{
	// Draws the settings page for one frame. Called by the SMF section-item
	// callback (Framework.cpp) while the BuffHUD page is active in the Mod
	// Control Panel. Draws widgets directly into SMF's panel window; the panel
	// itself (title bar, scroll, navigation) is owned by SMF.
	void __stdcall RenderSettings();
}
