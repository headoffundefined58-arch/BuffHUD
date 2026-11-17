// BuffHUD settings page, rendered by the SkyUI SKSE Menu Framework.
//
// This TU includes Smf.h (which pulls in <windows.h> transitively) and is kept
// out of the project precompiled header for the same reason as Hud.cpp: it must
// not see CommonLibSSE's REX::W32 declarations. The ImGui API lives in
// namespace ImGuiMCP; the alias + using-directive keep the ImGui::* calls and
// the unqualified ImVec2 / ImGuiCol_* names valid.

#include "Smf.h"

namespace ImGui = ImGuiMCP;
using namespace ImGuiMCP;

#include <string>
#include <string_view>
#include <vector>

#include "Config.h"
#include "Framework.h"
#include "Input.h"
#include "Loc.h"
#include "Menu.h"

namespace BuffHUD::Menu
{
	namespace
	{
		bool g_dirty{ false };
		bool g_langDirty{ false };

		// Localised string for one key.
		[[nodiscard]] std::string T(std::string_view a_key)
		{
			return std::string{ Loc::Get(a_key) };
		}

		// Shows a tooltip from "<key>.desc" when the last item is hovered.
		void Help(std::string_view a_key)
		{
			const std::string tip = T(std::string{ a_key } + ".desc");
			if (!tip.empty() && ImGui::IsItemHovered()) {
				ImGui::SetTooltip("%s", tip.c_str());
			}
		}

		bool CheckL(const char* a_key, bool* a_v)
		{
			const bool r = ImGui::Checkbox(T(a_key).c_str(), a_v);
			Help(a_key);
			return r;
		}

		bool SliderL(const char* a_key, int* a_v, int a_lo, int a_hi, const char* a_fmt = "%d")
		{
			const bool r = ImGui::SliderInt(T(a_key).c_str(), a_v, a_lo, a_hi, a_fmt);
			Help(a_key);
			return r;
		}

		bool ComboL(const char* a_key, int* a_v, std::initializer_list<std::string_view> a_items)
		{
			std::vector<std::string>   owned;
			std::vector<const char*>   ptrs;
			owned.reserve(a_items.size());
			ptrs.reserve(a_items.size());
			for (const auto item : a_items) {
				owned.emplace_back(T(item));
				ptrs.push_back(owned.back().c_str());
			}
			const bool r = ImGui::Combo(T(a_key).c_str(), a_v,
				ptrs.data(), static_cast<int>(ptrs.size()));
			Help(a_key);
			return r;
		}

		void KeyBindL(const char* a_key, int* a_v)
		{
			ImGui::Text("%s", T(a_key).c_str());
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8.0f);

			// Consume a key captured by the SKSE input sink on a previous
			// frame (see Input.cpp).
			const int captured = BuffHUD::Input::ConsumeCapturedKey();
			if (captured >= 0) {
				*a_v = captured;
				g_dirty = true;
			}

			const char* label = BuffHUD::Input::IsCapturing() ? T("menu.binding").c_str()
			                                            : BuffHUD::Framework::KeyName(*a_v);
			if (ImGui::Button(label, ImVec2(120.0f, 0.0f))) {
				BuffHUD::Input::BeginCapture();
			}
			Help(a_key);
		}
	}

	void __stdcall RenderSettings()
	{
		auto& cfg = Edit();

		// ---- Display ----
		ImGui::SeparatorText(T("menu.sec.display").c_str());
		if (CheckL("menu.enabled", &cfg.enabled)) g_dirty = true;
		if (SliderL("menu.maxRows", &cfg.maxRows, 1, 40)) g_dirty = true;
		if (ComboL("menu.anchor", &cfg.anchor,
				{ "menu.anchor.tl", "menu.anchor.tr", "menu.anchor.bl",
				  "menu.anchor.br", "menu.anchor.c" }))
			g_dirty = true;
		if (ComboL("menu.sort", &cfg.sort,
				{ "menu.sort.remaining", "menu.sort.type", "menu.sort.name" }))
			g_dirty = true;
		if (CheckL("menu.merge", &cfg.merge)) g_dirty = true;
		if (CheckL("menu.showTime", &cfg.showTime)) g_dirty = true;
		if (CheckL("menu.showSource", &cfg.showSource)) g_dirty = true;
		if (CheckL("menu.hideInactive", &cfg.hideInactive)) g_dirty = true;

		// ---- Appearance ----
		ImGui::SeparatorText(T("menu.sec.appearance").c_str());
		if (SliderL("menu.scale", &cfg.scale, 70, 200, "%d%%")) g_dirty = true;
		if (SliderL("menu.opacity", &cfg.opacity, 0, 100, "%d%%")) g_dirty = true;
		if (ComboL("menu.theme", &cfg.theme,
				{ "menu.theme.parchment", "menu.theme.cool", "menu.theme.contrast" }))
			g_dirty = true;
		if (CheckL("menu.tint", &cfg.tint)) g_dirty = true;

		// ---- Controls ----
		ImGui::SeparatorText(T("menu.sec.input").c_str());
		KeyBindL("menu.key", &cfg.key);
		if (SliderL("menu.hold", &cfg.holdMs, 100, 2000, "%d ms")) g_dirty = true;
		if (CheckL("menu.combatOnly", &cfg.combatOnly)) g_dirty = true;

		// ---- Compatibility ----
		ImGui::SeparatorText(T("menu.sec.compat").c_str());
		if (CheckL("menu.showHidden", &cfg.showHidden)) g_dirty = true;
		if (CheckL("menu.debug", &cfg.debug)) g_dirty = true;

		// ---- Language ----
		ImGui::SeparatorText(T("menu.sec.language").c_str());
		if (ComboL("menu.language", &cfg.language,
				{ "menu.language.auto", "menu.language.zh", "menu.language.en" })) {
			g_dirty = true;
			g_langDirty = true;
		}

		// ---- About ----
		ImGui::SeparatorText(T("menu.sec.about").c_str());
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle()->Colors[ImGuiCol_TextDisabled]);
		ImGui::TextWrapped("%s", T("menu.about").c_str());
		ImGui::PopStyleColor();

		if (g_dirty) {
			Save();
			if (g_langDirty) {
				Loc::Load(Cfg().language, BuffHUD::Framework::SystemIsChinese());
			}
			g_dirty = false;
			g_langDirty = false;
		}
	}
}
