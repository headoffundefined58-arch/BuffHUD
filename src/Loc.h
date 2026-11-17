#pragma once

#include <string_view>

namespace BuffHUD::Loc
{
	// a_languageMode: 0 follow the game, 1 Chinese, 2 English.
	// a_systemChinese is the OS UI hint supplied by the platform layer; the
	// game does not expose its own language through a stable API.
	void Load(int a_languageMode, bool a_systemChinese);

	// Returns an empty view when the key is missing, so callers can skip a
	// label instead of drawing the raw key.
	[[nodiscard]] std::string_view Get(std::string_view a_key);

	// 1 = Chinese, 2 = English, after mode resolution.
	[[nodiscard]] int ActiveLanguage();

	[[nodiscard]] const char* LanguageTag();
}
