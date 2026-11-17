#pragma once

namespace BuffHUD::Framework
{
	// Registers BuffHUD with the SkyUI SKSE Menu Framework: the settings panel
	// becomes a page in the Mod Control Panel (under the "BuffHUD" section) and
	// the buff list is drawn as a HUD element on SMF's overlay layer.
	//
	// SMF owns the ImGui context, the D3D11 device and the render loop, so this
	// plugin no longer spins up its own overlay. When SMF is not installed the
	// call is a no-op (with a log warning) and the plugin simply does nothing
	// visible, instead of crashing on a missing Present hook.
	void Register();

	// True once Register() succeeded and SMF is present in the process.
	[[nodiscard]] bool Ready();

	// OS UI language probe used to pick the default localisation. Lives here
	// because Framework.cpp is the translation unit allowed to see <windows.h>.
	[[nodiscard]] bool SystemIsChinese();

	// Human-readable name for a DirectInput scan code (e.g. "F", "Enter", "A"),
	// used by the settings key-binding row and the hint line.
	[[nodiscard]] const char* KeyName(int a_scan);
}
