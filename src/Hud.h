#pragma once

namespace BuffHUD::Hud
{
	// Draws the panel for one frame. Called by the SMF HUD-element callback
	// (Framework.cpp), which has already applied the visibility guards. Reads only
	// the collector snapshot and the config; renders into SMF's ImGui context.
	void __stdcall Render();
}
