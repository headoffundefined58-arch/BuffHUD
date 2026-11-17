#include "PCH.h"

#include "Icons.h"
#include "Paths.h"

#include <fstream>
#include <sstream>

namespace BuffHUD::Icons
{
	namespace
	{
		std::unordered_map<std::string, IconDef> g_icons;
		IconDef                                  g_unknown{};
		int                                      g_atlasW{ 0 };
		int                                      g_atlasH{ 0 };
		float                                    g_cell{ 256.0f };

		[[nodiscard]] Tint ParseTint(std::string_view a_text)
		{
			if (a_text == "buff") {
				return Tint::kBuff;
			}
			if (a_text == "debuff") {
				return Tint::kDebuff;
			}
			if (a_text == "vital") {
				return Tint::kVital;
			}
			if (a_text == "damage") {
				return Tint::kDamage;
			}
			return Tint::kNeutral;
		}
	}

	bool Load()
	{
		g_icons.clear();

		const auto path = Paths::PluginData() / "icons.tsv";
		std::ifstream file(path, std::ios::binary);
		if (!file) {
			spdlog::error("[BuffHUD] icons.tsv missing at {}", path.string());
			return false;
		}

		std::string line;
		while (std::getline(file, line)) {
			if (line.empty() || line.front() == '#') {
				continue;
			}

			std::istringstream stream(line);
			std::string        tag;
			if (!(stream >> tag)) {
				continue;
			}

			if (tag == "cell") {
				stream >> g_cell;
			} else if (tag == "atlas") {
				stream >> g_atlasW >> g_atlasH;
			} else {
				std::string tint;
				int         col = 0;
				int         row = 0;
				if (!(stream >> tint >> col >> row)) {
					continue;
				}
				IconDef def{};
				def.tint = ParseTint(tint);
				if (g_atlasW > 0 && g_atlasH > 0) {
					def.u0 = static_cast<float>(col) * g_cell / static_cast<float>(g_atlasW);
					def.v0 = static_cast<float>(row) * g_cell / static_cast<float>(g_atlasH);
					def.u1 = def.u0 + g_cell / static_cast<float>(g_atlasW);
					def.v1 = def.v0 + g_cell / static_cast<float>(g_atlasH);
				}
				g_icons[tag] = def;
			}
		}

		// The unknown tile is the last cell of the sheet; if the table has it,
		// reuse its UVs so a broken rule still draws something recognisable.
		if (const auto it = g_icons.find("unknown"); it != g_icons.end()) {
			g_unknown = it->second;
		}

		spdlog::info("[BuffHUD] icons: {} entries, atlas {}x{}, cell {}",
			g_icons.size(), g_atlasW, g_atlasH, g_cell);
		return !g_icons.empty();
	}

	const IconDef* Find(std::string_view a_id)
	{
		const auto it = g_icons.find(std::string{ a_id });
		if (it != g_icons.end()) {
			return &it->second;
		}
		return Unknown();
	}

	const IconDef* Unknown()
	{
		return &g_unknown;
	}

	int AtlasWidth()
	{
		return g_atlasW;
	}

	int AtlasHeight()
	{
		return g_atlasH;
	}

	float CellSize()
	{
		return g_cell;
	}

	std::size_t Count()
	{
		return g_icons.size();
	}
}
