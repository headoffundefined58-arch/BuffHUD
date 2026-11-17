#include "PCH.h"

#include "Paths.h"

namespace BuffHUD::Paths
{
	const std::filesystem::path& GameRoot()
	{
		// Module::filePath is the running executable, so its parent is the
		// install root regardless of how MO2 mounted the mod folder.
		static const std::filesystem::path root = [] {
			std::filesystem::path p{ REL::Module::get().filePath() };
			return p.parent_path();
		}();
		return root;
	}

	std::filesystem::path Data()
	{
		return GameRoot() / "Data";
	}

	std::filesystem::path PluginData()
	{
		return Data() / "SKSE" / "Plugins" / "BuffHUD";
	}

	std::filesystem::path SettingsIni()
	{
		return PluginData() / "BuffHUD.ini";
	}
}
