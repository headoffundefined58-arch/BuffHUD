#include "PCH.h"

#include "Loc.h"
#include "Paths.h"

#include <fstream>

namespace BuffHUD::Loc
{
	namespace
	{
		std::unordered_map<std::string, std::string> g_strings;
		int                                          g_language{ 2 };

		bool LoadFile(const std::filesystem::path& a_path)
		{
			std::ifstream file(a_path, std::ios::binary);
			if (!file) {
				return false;
			}

			std::string line;
			while (std::getline(file, line)) {
				if (!line.empty() && line.back() == '\r') {
					line.pop_back();
				}
				if (line.empty() || line.front() == '#' || line.front() == ';') {
					continue;
				}
				const auto tab = line.find('\t');
				if (tab == std::string::npos || tab == 0) {
					continue;
				}
				g_strings[line.substr(0, tab)] = line.substr(tab + 1);
			}
			return true;
		}
	}

	void Load(int a_languageMode, bool a_systemChinese)
	{
		g_strings.clear();

		int wanted = a_languageMode;
		if (wanted == 0) {
			wanted = a_systemChinese ? 1 : 2;
		}
		g_language = wanted == 1 ? 1 : 2;

		const auto base = Paths::PluginData() / "lang";
		const char* primary = g_language == 1 ? "zh.tsv" : "en.tsv";
		const char* other = g_language == 1 ? "en.tsv" : "zh.tsv";

		if (!LoadFile(base / primary)) {
			spdlog::warn("[BuffHUD] language pack {} missing, falling back", primary);
			g_language = g_language == 1 ? 2 : 1;
			if (!LoadFile(base / other)) {
				spdlog::error("[BuffHUD] no language pack found under {}", base.string());
			}
		}

		spdlog::info("[BuffHUD] language: {} ({} strings)", LanguageTag(), g_strings.size());
	}

	std::string_view Get(std::string_view a_key)
	{
		const auto it = g_strings.find(std::string{ a_key });
		if (it == g_strings.end()) {
			return {};
		}
		return it->second;
	}

	int ActiveLanguage()
	{
		return g_language;
	}

	const char* LanguageTag()
	{
		return g_language == 1 ? "zh" : "en";
	}
}
