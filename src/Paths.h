#pragma once

#include <filesystem>

namespace BuffHUD::Paths
{
	// Directory holding SkyrimSE.exe. REL::Module knows it even under MO2's VFS.
	[[nodiscard]] const std::filesystem::path& GameRoot();

	// <game>\Data
	[[nodiscard]] std::filesystem::path Data();

	// <game>\Data\SKSE\Plugins\BuffHUD  -- the plugin's own assets
	[[nodiscard]] std::filesystem::path PluginData();

	// <game>\Data\SKSE\Plugins\BuffHUD\BuffHUD.ini -- owned by the DLL itself
	[[nodiscard]] std::filesystem::path SettingsIni();
}
