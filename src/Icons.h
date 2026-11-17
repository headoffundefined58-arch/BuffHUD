#pragma once

#include <cstddef>
#include <string_view>

namespace BuffHUD::Icons
{
	enum class Tint : int
	{
		kBuff = 0,
		kDebuff,
		kNeutral,
		kVital,
		kDamage
	};

	struct IconDef
	{
		float u0{ 0.0f };
		float v0{ 0.0f };
		float u1{ 0.0f };
		float v1{ 0.0f };
		Tint  tint{ Tint::kNeutral };
	};

	// Reads <PluginData>\icons.tsv next to the atlas PNG. Flat table on
	// purpose: the runtime never parses JSON, so malformed authoring data can
	// only ever produce a missing icon, never a crash.
	bool Load();

	[[nodiscard]] const IconDef* Find(std::string_view a_id);

	// Fallback used when a rule points at an icon that is not in the table.
	[[nodiscard]] const IconDef* Unknown();

	[[nodiscard]] int   AtlasWidth();
	[[nodiscard]] int   AtlasHeight();
	[[nodiscard]] float CellSize();
	[[nodiscard]] std::size_t Count();
}
