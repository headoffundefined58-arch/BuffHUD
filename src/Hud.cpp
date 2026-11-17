// BuffHUD HUD layer, rendered through the SkyUI SKSE Menu Framework.
//
// This TU includes Smf.h (which pulls in <windows.h> transitively) and must
// NOT use the project precompiled header: that would drag CommonLibSSE's
// REX::W32 declarations in ahead of the Windows SDK and hard-error. It is
// excluded from the PCH in CMakeLists.txt.
//
// SMF exposes the entire Dear ImGui API under namespace ImGuiMCP; the alias and
// using-directive below keep the existing ImGui::* calls (and the unqualified
// ImVec2 / ImVec4 / ImGuiCol_* names) valid without touching the draw code.

#include "Smf.h"

namespace ImGui = ImGuiMCP;
using namespace ImGuiMCP;

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

#include "Collector.h"
#include "Config.h"
#include "Framework.h"
#include "Icons.h"
#include "Loc.h"
#include "Paths.h"
#include "Rules.h"

namespace BuffHUD::Hud
{
	namespace
	{
		// Cached handle to the icon atlas, loaded once through SMF's texture
		// loader. SMF owns the underlying GPU resource; we only keep the
		// ImTextureID. A zero result (not yet loaded) is retried next frame.
		void* AtlasTexture()
		{
			static void* g_atlas = nullptr;
			if (!g_atlas) {
				const auto path = (BuffHUD::Paths::PluginData() / "icons" / "atlas.png").string();
				g_atlas = SKSEMenuFramework::LoadTexture(path);
			}
			return g_atlas;
		}

		struct Theme
		{
			ImVec4 background;
			ImVec4 text;
			ImVec4 dim;
			ImVec4 rule;
			ImVec4 highlight;
		};

		[[nodiscard]] Theme GetTheme(int a_index)
		{
			switch (a_index) {
			case 1:  // cool grey
				return { { 0.05f, 0.06f, 0.08f, 1.0f }, { 0.87f, 0.89f, 0.92f, 1.0f }, { 0.62f, 0.65f, 0.70f, 1.0f },
					{ 0.30f, 0.32f, 0.36f, 1.0f }, { 1.00f, 0.75f, 0.35f, 1.0f } };
			case 2:  // high contrast
				return { { 0.00f, 0.00f, 0.00f, 1.0f }, { 1.00f, 1.00f, 1.00f, 1.0f }, { 0.78f, 0.78f, 0.78f, 1.0f },
					{ 0.45f, 0.45f, 0.45f, 1.0f }, { 1.00f, 0.85f, 0.30f, 1.0f } };
			default:  // parchment
				return { { 0.075f, 0.070f, 0.060f, 1.0f }, { 0.88f, 0.86f, 0.80f, 1.0f }, { 0.62f, 0.60f, 0.55f, 1.0f },
					{ 0.34f, 0.32f, 0.27f, 1.0f }, { 0.85f, 0.78f, 0.58f, 1.0f } };
			}
		}

		[[nodiscard]] ImVec4 TintColour(Icons::Tint a_tint, bool a_enabled)
		{
			if (!a_enabled) {
				return { 0.90f, 0.89f, 0.85f, 1.0f };
			}
			switch (a_tint) {
			case Icons::Tint::kBuff: return { 0.90f, 0.75f, 0.48f, 1.0f };
			case Icons::Tint::kDebuff: return { 0.88f, 0.42f, 0.46f, 1.0f };
			case Icons::Tint::kVital: return { 0.56f, 0.75f, 0.56f, 1.0f };
			case Icons::Tint::kDamage: return { 0.85f, 0.55f, 0.55f, 1.0f };
			default: return { 0.62f, 0.70f, 0.78f, 1.0f };
			}
		}

		[[nodiscard]] std::string Text(std::string_view a_key)
		{
			return std::string{ Loc::Get(a_key) };
		}

		// Replaces the single {0} placeholder the packs use.
		[[nodiscard]] std::string WithArg(std::string_view a_key, std::string_view a_value)
		{
			std::string text{ Loc::Get(a_key) };
			if (const auto pos = text.find("{0}"); pos != std::string::npos) {
				text.replace(pos, 3, a_value);
			}
			return text;
		}

		[[nodiscard]] std::string DurationText(float a_remain)
		{
			if (a_remain < 0.0f) {
				return Text("time.infinite");
			}
			const int total = static_cast<int>(a_remain + 0.5f);
			if (total >= 60) {
				std::string minutes = WithArg("time.minutes", std::to_string(total / 60));
				const auto  pos = minutes.find("{0}");
				if (pos != std::string::npos) {
					minutes.replace(pos, 3, std::to_string(total % 60));
				}
				return minutes;
			}
			return WithArg("time.seconds", std::to_string(total));
		}

		void Row(const Entry& a_entry, float a_scale, const Theme& a_theme, float a_nameWidth, float a_timeWidth,
			float a_sourceWidth, bool a_showTime, bool a_showSource)
		{
			const auto& cfg = Cfg();
			ImGui::PushID(static_cast<int>(a_entry.effectID));

			const float iconSize = 20.0f * a_scale;
			const auto* icon = a_entry.icon ? a_entry.icon : Icons::Unknown();

			// A thin accent bar carries the buff/debuff verdict even when the
			// icon tint is switched off.
			const ImVec4 accent = a_entry.detrimental ? ImVec4{ 0.72f, 0.30f, 0.33f, 0.9f } : ImVec4{ 0.60f, 0.52f, 0.30f, 0.9f };
			const ImVec2 barPos = ImGui::GetCursorScreenPos();
		// Draw-list helpers live in ImGuiMCP::ImDrawListManager, not at the
		// top level of ImGuiMCP (and they are free functions taking the list).
		ImGuiMCP::ImDrawListManager::AddRectFilled(ImGui::GetWindowDrawList(), barPos,
			ImVec2{ barPos.x + 2.0f * a_scale, barPos.y + iconSize }, ImGui::GetColorU32(accent), 0.0f, 0);
			ImGui::Dummy(ImVec2{ 4.0f * a_scale, iconSize });

			ImGui::SameLine();
			if (void* atlas = AtlasTexture()) {
				ImGui::Image(atlas, ImVec2{ iconSize, iconSize },
					ImVec2{ icon->u0, icon->v0 }, ImVec2{ icon->u1, icon->v1 },
					TintColour(icon->tint, cfg.tint));
			} else {
				ImGui::Dummy(ImVec2{ iconSize, iconSize });
			}

			ImGui::SameLine();
			// Absolute column origin for this row: everything to the right is
			// placed from here, otherwise the columns drift one row at a time.
			const float nameX = ImGui::GetCursorPosX();
			ImGui::PushStyleColor(ImGuiCol_Text, a_theme.text);
			if (a_entry.count > 1) {
				ImGui::TextUnformatted((a_entry.name + "  x" + std::to_string(a_entry.count)).c_str());
			} else {
				ImGui::TextUnformatted(a_entry.name.c_str());
			}
			ImGui::PopStyleColor();

			if (a_showTime) {
				const std::string text = DurationText(a_entry.remain);
				ImGui::SameLine();
				ImGui::SetCursorPosX(nameX + a_nameWidth - ImGui::CalcTextSize(text.c_str()).x);
				ImGui::PushStyleColor(ImGuiCol_Text, a_entry.detrimental ? ImVec4{ 0.80f, 0.55f, 0.55f, 1.0f } : a_theme.dim);
				ImGui::TextUnformatted(text.c_str());
				ImGui::PopStyleColor();
			}
			if (a_showSource && !a_entry.source.empty()) {
				ImGui::SameLine();
				ImGui::SetCursorPosX(nameX + a_nameWidth + a_timeWidth + a_sourceWidth - ImGui::CalcTextSize(a_entry.source.c_str()).x);
				ImGui::PushStyleColor(ImGuiCol_Text, a_theme.dim);
				ImGui::TextUnformatted(a_entry.source.c_str());
				ImGui::PopStyleColor();
			}

			ImGui::PopID();
		}

		void Detail(const Entry& a_entry, float a_scale, const Theme& a_theme, float a_width)
		{
			const auto& cfg = Cfg();

			ImGui::PushStyleColor(ImGuiCol_Separator, a_theme.rule);
			ImGui::Separator();
			ImGui::PopStyleColor();

			const float iconSize = 30.0f * a_scale;
			if (void* atlas = AtlasTexture()) {
				const auto* icon = a_entry.icon ? a_entry.icon : Icons::Unknown();
				ImGui::Image(atlas, ImVec2{ iconSize, iconSize },
					ImVec2{ icon->u0, icon->v0 }, ImVec2{ icon->u1, icon->v1 },
					TintColour(icon->tint, cfg.tint));
				ImGui::SameLine();
			}

			ImGui::BeginGroup();
			ImGui::PushStyleColor(ImGuiCol_Text, a_theme.text);
			ImGui::TextUnformatted(a_entry.name.c_str());
			ImGui::PopStyleColor();
			if (!a_entry.source.empty()) {
				ImGui::PushStyleColor(ImGuiCol_Text, a_theme.dim);
				ImGui::TextUnformatted(a_entry.source.c_str());
				ImGui::PopStyleColor();
			}
			ImGui::EndGroup();

			ImGui::PushStyleColor(ImGuiCol_Text, a_theme.text);
			const std::string body = a_entry.desc.empty() ? Text("detail.noDescription") : a_entry.desc;
			ImGui::PushTextWrapPos(a_width);
			ImGui::TextUnformatted(body.c_str());
			ImGui::PopTextWrapPos();

			std::string meta;
			meta += Text("detail.magnitude") + " " + std::to_string(static_cast<int>(a_entry.magnitude)) + "    ";
			meta += Text("detail.remaining") + " " + DurationText(a_entry.remain);
			if (a_entry.inactive) {
				meta += "    " + Text("detail.inactive");
			}
			ImGui::PushStyleColor(ImGuiCol_Text, a_theme.dim);
			ImGui::TextUnformatted(meta.c_str());

			if (cfg.debug) {
				char buffer[160]{};
				std::snprintf(buffer, sizeof(buffer), "%s %08X    %s %s    id %u",
					Text("debug.mgef").c_str(), a_entry.effectID,
					Text("debug.archetype").c_str(), Rules::ArchetypeName(a_entry.archetype),
					static_cast<unsigned>(a_entry.effectID & 0xFFFFu));
				ImGui::TextUnformatted(buffer);
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
	}

	void __stdcall Render()
	{
		const auto& cfg = Cfg();
		if (!cfg.enabled) {
			return;
		}

		// Maintenance that used to live in the D3D11 Present hook: re-read the
		// ini at most once a second, and refresh the effect snapshot four times a
		// second. Both are cheap and only run while the panel is visible, which
		// is exactly when they matter.
		{
			static auto lastPoll = std::chrono::steady_clock::now();
			static auto lastRefresh = std::chrono::steady_clock::now();
			const auto now = std::chrono::steady_clock::now();
			if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPoll).count() >= 1000) {
				lastPoll = now;
				if (BuffHUD::PollConfig()) {
					BuffHUD::Loc::Load(BuffHUD::Cfg().language, BuffHUD::Framework::SystemIsChinese());
					BuffHUD::Collector::Invalidate();
				}
			}
			if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRefresh).count() >= 250) {
				lastRefresh = now;
				BuffHUD::Collector::RequestRebuild();
			}
		}

		const auto& snapshot = Collector::Get();
		const float scale = static_cast<float>(cfg.scale) / 100.0f;
		const Theme theme = GetTheme(cfg.theme);

		// Fixed column geometry: icons + name | duration | source.
		const float pad = 14.0f * scale;
		const float nameWidth = 190.0f * scale;
		const float timeWidth = cfg.showTime ? 76.0f * scale : 0.0f;
		const float sourceWidth = cfg.showSource ? 140.0f * scale : 0.0f;
		const float iconColumn = 26.0f * scale;
		const float panelWidth = pad * 2.0f + iconColumn + nameWidth + timeWidth + sourceWidth;

		const float rowHeight = 26.0f * scale;
		const float headerHeight = 34.0f * scale;
		const int   rows = static_cast<int>(snapshot.items.size());
		const float detailHeight = rows > 0 ? 118.0f * scale : 0.0f;
		const float panelHeight = pad * 2.0f + headerHeight + rowHeight * static_cast<float>(std::max(rows, 1)) +
			detailHeight + (snapshot.total > rows ? rowHeight : 0.0f);

		ImGuiIO* io = ImGui::GetIO();
		const float margin = 40.0f * scale;
		ImVec2      pos{ margin, margin };
		switch (cfg.anchor) {
		case 1: pos = { io->DisplaySize.x - panelWidth - margin, margin }; break;
		case 2: pos = { margin, io->DisplaySize.y - panelHeight - margin }; break;
		case 3: pos = { io->DisplaySize.x - panelWidth - margin, io->DisplaySize.y - panelHeight - margin }; break;
		case 4: pos = { (io->DisplaySize.x - panelWidth) * 0.5f, (io->DisplaySize.y - panelHeight) * 0.5f }; break;
		default: break;
		}

		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(ImVec2{ panelWidth, panelHeight });
		ImGui::SetNextWindowBgAlpha(static_cast<float>(cfg.opacity) / 100.0f);

		constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav |
		                                   ImGuiWindowFlags_NoBringToFrontOnFocus;

		ImGui::PushStyleColor(ImGuiCol_WindowBg, theme.background);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ pad, pad });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 4.0f * scale, 4.0f * scale });

		if (ImGui::Begin("##BuffHUD", nullptr, flags)) {
			ImGui::PushStyleColor(ImGuiCol_Text, theme.text);
			ImGui::TextUnformatted(Text("panel.title").c_str());
			ImGui::PopStyleColor();

			ImGui::SameLine();
			const std::string count = WithArg("panel.count", std::to_string(snapshot.total));
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - pad - ImGui::CalcTextSize(count.c_str()).x);
			ImGui::PushStyleColor(ImGuiCol_Text, theme.dim);
			ImGui::TextUnformatted(count.c_str());
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Separator, theme.rule);
			ImGui::Separator();
			ImGui::PopStyleColor();

			if (snapshot.items.empty()) {
				ImGui::PushStyleColor(ImGuiCol_Text, theme.dim);
				ImGui::TextUnformatted(Text("panel.empty").c_str());
				ImGui::PopStyleColor();
			}

			for (const auto& entry : snapshot.items) {
				Row(entry, scale, theme, nameWidth, timeWidth, sourceWidth, cfg.showTime, cfg.showSource);
			}

			if (snapshot.total > rows) {
				ImGui::PushStyleColor(ImGuiCol_Text, theme.dim);
				ImGui::TextUnformatted(WithArg("panel.more", std::to_string(snapshot.total - rows)).c_str());
				ImGui::PopStyleColor();
			}

			if (!snapshot.items.empty()) {
				// The detail pane always describes the top row: with a hold
				// hotkey there is no pointer to select with, and the top row
				// is the one sorted to matter most.
				Detail(snapshot.items.front(), scale, theme, panelWidth - pad * 2.0f);
			}
		}
		ImGui::End();

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}
}
