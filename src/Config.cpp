#include "PCH.h"

#include "Config.h"
#include "Paths.h"

#include <fstream>

namespace BuffHUD
{
	namespace
	{
		Config g_config{};
		std::filesystem::file_time_type g_stamp{};

		[[nodiscard]] std::string_view Trim(std::string_view a_text)
		{
			while (!a_text.empty() && (a_text.front() == ' ' || a_text.front() == '\t' || a_text.front() == '\r')) {
				a_text.remove_prefix(1);
			}
			while (!a_text.empty() && (a_text.back() == ' ' || a_text.back() == '\t' || a_text.back() == '\r')) {
				a_text.remove_suffix(1);
			}
			return a_text;
		}

		[[nodiscard]] bool ParseInt(std::string_view a_text, int& a_out)
		{
			// from_chars does not throw, which matters: an exception escaping
			// an SKSE callback takes the whole game down.
			const auto* first = a_text.data();
			const auto* last = first + a_text.size();
			int         value = 0;
			const auto  result = std::from_chars(first, last, value);
			if (result.ec != std::errc{} || result.ptr != last) {
				return false;
			}
			a_out = value;
			return true;
		}

		[[nodiscard]] bool ParseBool(std::string_view a_text, bool& a_out)
		{
			if (a_text == "1" || a_text == "true" || a_text == "True" || a_text == "yes") {
				a_out = true;
				return true;
			}
			if (a_text == "0" || a_text == "false" || a_text == "False" || a_text == "no") {
				a_out = false;
				return true;
			}
			int numeric = 0;
			if (ParseInt(a_text, numeric)) {
				a_out = numeric != 0;
				return true;
			}
			return false;
		}

		void Apply(Config& a_cfg, std::string_view a_key, std::string_view a_value)
		{
			if (a_key == "bEnabled") {
				ParseBool(a_value, a_cfg.enabled);
			} else if (a_key == "iMaxRows") {
				ParseInt(a_value, a_cfg.maxRows);
			} else if (a_key == "iAnchor") {
				ParseInt(a_value, a_cfg.anchor);
			} else if (a_key == "iSort") {
				ParseInt(a_value, a_cfg.sort);
			} else if (a_key == "bMerge") {
				ParseBool(a_value, a_cfg.merge);
			} else if (a_key == "bShowTime") {
				ParseBool(a_value, a_cfg.showTime);
			} else if (a_key == "bShowSource") {
				ParseBool(a_value, a_cfg.showSource);
			} else if (a_key == "bHideInactive") {
				ParseBool(a_value, a_cfg.hideInactive);
			} else if (a_key == "iScale") {
				ParseInt(a_value, a_cfg.scale);
			} else if (a_key == "iOpacity") {
				ParseInt(a_value, a_cfg.opacity);
			} else if (a_key == "iTheme") {
				ParseInt(a_value, a_cfg.theme);
			} else if (a_key == "bTint") {
				ParseBool(a_value, a_cfg.tint);
			} else if (a_key == "iKey") {
				ParseInt(a_value, a_cfg.key);
			} else if (a_key == "iHoldMs") {
				ParseInt(a_value, a_cfg.holdMs);
			} else if (a_key == "bCombatOnly") {
				ParseBool(a_value, a_cfg.combatOnly);
			} else if (a_key == "bShowHidden") {
				ParseBool(a_value, a_cfg.showHidden);
			} else if (a_key == "bDebug") {
				ParseBool(a_value, a_cfg.debug);
		} else if (a_key == "iLanguage") {
			ParseInt(a_value, a_cfg.language);
		}
	}
	}

	const Config& Cfg()
	{
		return g_config;
	}

	Config& Edit()
	{
		return g_config;
	}

	std::string ConfigPathForLog()
	{
		return Paths::SettingsIni().string();
	}

	void LoadConfig()
	{
		Config loaded{};

		const auto path = Paths::SettingsIni();
		std::error_code ec;
		if (!std::filesystem::exists(path, ec)) {
			spdlog::info("[BuffHUD] no settings ini at {}, using defaults", path.string());
			g_config = loaded;
			return;
		}

		std::ifstream file(path, std::ios::binary);
		if (!file) {
			spdlog::warn("[BuffHUD] settings ini exists but could not be opened");
			g_config = loaded;
			return;
		}

		std::string line;
		while (std::getline(file, line)) {
			std::string_view view{ line };
			view = Trim(view);
			if (view.empty() || view.front() == ';' || view.front() == '[' || view.front() == '#') {
				continue;
			}
			const auto eq = view.find('=');
			if (eq == std::string_view::npos) {
				continue;
			}
			Apply(loaded, Trim(view.substr(0, eq)), Trim(view.substr(eq + 1)));
		}

		// Clamp everything that feeds a size or a loop bound, so a typo in the
		// ini cannot produce a 4000-row panel or a negative window size.
		loaded.maxRows = std::clamp(loaded.maxRows, 1, 40);
		loaded.scale = std::clamp(loaded.scale, 70, 200);
		loaded.opacity = std::clamp(loaded.opacity, 0, 100);
		loaded.holdMs = std::clamp(loaded.holdMs, 100, 2000);
		loaded.anchor = std::clamp(loaded.anchor, 0, 4);
		loaded.sort = std::clamp(loaded.sort, 0, 2);
		loaded.theme = std::clamp(loaded.theme, 0, 2);
		loaded.language = std::clamp(loaded.language, 0, 2);

		g_config = loaded;
		std::error_code stampEc;
		g_stamp = std::filesystem::last_write_time(path, stampEc);

		spdlog::info("[BuffHUD] settings: lang={} hold={}ms key={} rows={} anchor={}",
			loaded.language, loaded.holdMs, loaded.key, loaded.maxRows, loaded.anchor);
	}

	void Save()
	{
		const auto path = Paths::SettingsIni();
		std::error_code ec;
		if (!std::filesystem::exists(path.parent_path(), ec)) {
			std::filesystem::create_directories(path.parent_path(), ec);
		}

		std::ofstream file(path, std::ios::binary);
		if (!file) {
			spdlog::warn("[BuffHUD] cannot write settings to {}", path.string());
			return;
		}

		const auto w = [&](std::string_view a_key, const auto& a_value) {
			file << a_key << " = " << a_value << '\n';
		};

		file << "; BuffHUD settings - written by the in-game menu.\n";
		file << "; Hand-editing is fine; sections are ignored on load, only the key names matter.\n\n";

		file << "[Display]\n";
		w("bEnabled", g_config.enabled ? 1 : 0);
		w("iMaxRows", g_config.maxRows);
		w("iAnchor", g_config.anchor);
		w("iSort", g_config.sort);
		w("bMerge", g_config.merge ? 1 : 0);
		w("bShowTime", g_config.showTime ? 1 : 0);
		w("bShowSource", g_config.showSource ? 1 : 0);
		w("bHideInactive", g_config.hideInactive ? 1 : 0);

		file << "\n[Appearance]\n";
		w("iScale", g_config.scale);
		w("iOpacity", g_config.opacity);
		w("iTheme", g_config.theme);
		w("bTint", g_config.tint ? 1 : 0);

		file << "\n[Controls]\n";
		w("iKey", g_config.key);
		w("iHoldMs", g_config.holdMs);
		w("bCombatOnly", g_config.combatOnly ? 1 : 0);

		file << "\n[Compat]\n";
		w("bShowHidden", g_config.showHidden ? 1 : 0);
		w("bDebug", g_config.debug ? 1 : 0);

		file << "\n[Localisation]\n";
		w("iLanguage", g_config.language);

		spdlog::info("[BuffHUD] settings saved to {}", path.string());
	}

	bool PollConfig()
	{
		std::error_code ec;
		const auto path = Paths::SettingsIni();
		if (!std::filesystem::exists(path, ec)) {
			return false;
		}
		const auto stamp = std::filesystem::last_write_time(path, ec);
		if (ec || stamp == g_stamp) {
			return false;
		}
		g_stamp = stamp;
		LoadConfig();
		return true;
	}
}
