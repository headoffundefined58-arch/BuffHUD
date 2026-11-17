#pragma once

namespace BuffHUD
{
	struct Config
	{
		bool enabled{ true };
		int  maxRows{ 12 };
		int  anchor{ 0 };    // 0 TL, 1 TR, 2 BL, 3 BR, 4 centre
		int  sort{ 1 };      // 0 remaining, 1 debuffs first, 2 name
		bool merge{ true };
		bool showTime{ true };
		bool showSource{ true };
		bool hideInactive{ true };

		int  scale{ 100 };   // percent
		int  opacity{ 78 };  // percent
		int  theme{ 0 };     // 0 parchment, 1 cool grey, 2 high contrast
		bool tint{ true };

		int  key{ 33 };      // DirectInput scan code, 33 = F
		int  holdMs{ 350 };
		bool combatOnly{ false };

		bool showHidden{ false };
		bool debug{ false };

		int  language{ 0 };  // 0 auto, 1 chinese, 2 english
	};

	[[nodiscard]] const Config& Cfg();

	// Non-const accessor for the in-game menu to mutate and persist settings.
	[[nodiscard]] Config& Edit();

	// Reads the ini (now under Data/SKSE/Plugins/BuffHUD/BuffHUD.ini). Never
	// throws: a missing or broken file simply leaves the defaults in place.
	void LoadConfig();

	// Cheap stat() call; returns true (and reloads) when the file changed.
	bool PollConfig();

	// Writes the current configuration back to the ini. Called by the menu after
	// any change so settings survive a restart.
	void Save();

	// Absolute path of the ini actually in use, for the log.
	[[nodiscard]] std::string ConfigPathForLog();
}
