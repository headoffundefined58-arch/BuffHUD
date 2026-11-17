#include "PCH.h"

#include "Collector.h"
#include "Config.h"
#include "Rules.h"

#include <cstdio>

namespace BuffHUD::Collector
{
	// Private helper defined below. Kept out of Collector.h so that Hud.cpp
	// (which cannot see CommonLibSSE types) never needs RE::EffectSetting.
	std::string BuildDescription(RE::EffectSetting* a_effect, float a_magnitude, float a_duration, float a_area);

	namespace
	{
		// Double buffered: the writer fills the back buffer and then flips the
		// front index, so the render thread is never reading the same Snapshot
		// that is being written.
		Snapshot          g_buffers[2]{};
		std::atomic<int>  g_front{ 0 };
		std::atomic<bool> g_taskPending{ false };
		std::atomic<bool> g_dirty{ true };

		void ReplaceAll(std::string& a_text, std::string_view a_needle, std::string_view a_replacement)
		{
			if (a_needle.empty()) {
				return;
			}
			std::size_t pos = 0;
			while ((pos = a_text.find(a_needle, pos)) != std::string::npos) {
				a_text.replace(pos, a_needle.size(), a_replacement);
				pos += a_replacement.size();
			}
		}

		[[nodiscard]] std::string Number(float a_value)
		{
			// Integers print without a decimal tail; everything else gets one
			// decimal, which is what the vanilla menu ends up showing.
			char buffer[32]{};
			if (std::abs(a_value - std::round(a_value)) < 0.05f) {
				std::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(std::round(a_value)));
			} else {
				std::snprintf(buffer, sizeof(buffer), "%.1f", a_value);
			}
			return buffer;
		}

		[[nodiscard]] std::string NameOf(RE::TESForm* a_form)
		{
			if (!a_form) {
				return {};
			}
			if (const char* name = a_form->GetName(); name && *name) {
				return name;
			}
			if (const char* editorID = a_form->GetFormEditorID(); editorID && *editorID) {
				return editorID;
			}
			return {};
		}

		void Collect(RE::Actor* a_actor, Snapshot& a_out)
		{
			auto* magicTarget = a_actor->AsMagicTarget();
			auto* list = magicTarget ? magicTarget->GetActiveEffectList() : nullptr;
			if (!list) {
				return;
			}

			const auto& cfg = Cfg();

			for (auto* active : *list) {
				if (!active) {
					continue;
				}
				auto* effect = active->GetBaseObject();
				if (!effect) {
					continue;
				}

				// Dispel already happened, the engine just has not unlinked it.
				if (active->flags.all(RE::ActiveEffect::Flag::kDispelled)) {
					continue;
				}

				// The game itself hides these; respect that unless asked not to.
				if (effect->data.flags.all(RE::EffectSetting::EffectSettingData::Flag::kHideInUI) && !cfg.showHidden) {
					continue;
				}

				const bool inactive = active->flags.all(RE::ActiveEffect::Flag::kInactive);
				if (inactive && cfg.hideInactive) {
					continue;
				}

				Entry entry{};
				entry.name = NameOf(effect);
				if (entry.name.empty()) {
					entry.name = "?";
				}

				if (active->spell) {
					entry.source = NameOf(active->spell);
				} else if (active->source) {
					entry.source = NameOf(active->source);
				}

				const bool noDuration =
					effect->data.flags.all(RE::EffectSetting::EffectSettingData::Flag::kNoDuration) || active->duration <= 0.0f;

				entry.remain = noDuration ? -1.0f : std::max(0.0f, active->duration - active->elapsedSeconds);
				entry.magnitude = active->magnitude;
				entry.detrimental = effect->IsDetrimental() || effect->IsHostile();
				entry.inactive = inactive;
				entry.icon = Rules::Resolve(effect);
				entry.effectID = effect->GetFormID();
				entry.archetype = static_cast<int>(effect->GetArchetype());
				entry.desc = BuildDescription(effect, active->magnitude, active->duration,
					active->effect ? static_cast<float>(active->effect->GetArea()) : 0.0f);

				a_out.items.push_back(std::move(entry));
			}

			// Merge duplicates: same magic effect, several castings. The timer
			// shown is the longest remaining one, which is what a player cares
			// about, and the count communicates the stack.
			if (cfg.merge) {
				std::vector<Entry> merged;
				merged.reserve(a_out.items.size());
				for (auto& entry : a_out.items) {
					bool folded = false;
					for (auto& kept : merged) {
						if (kept.effectID == entry.effectID) {
							kept.count += entry.count;
							kept.remain = std::max(kept.remain, entry.remain);
							kept.magnitude = std::max(kept.magnitude, entry.magnitude);
							folded = true;
							break;
						}
					}
					if (!folded) {
						merged.push_back(std::move(entry));
					}
				}
				a_out.items = std::move(merged);
			}

			const int sortMode = cfg.sort;
			std::ranges::sort(a_out.items, [sortMode](const Entry& a_lhs, const Entry& a_rhs) {
				if (sortMode == 2) {
					return a_lhs.name < a_rhs.name;
				}
				if (sortMode == 1 && a_lhs.detrimental != a_rhs.detrimental) {
					return a_lhs.detrimental;
				}
				// Permanent effects sink to the bottom; then soonest expiry.
				const bool lhsPerm = a_lhs.remain < 0.0f;
				const bool rhsPerm = a_rhs.remain < 0.0f;
				if (lhsPerm != rhsPerm) {
					return !lhsPerm;
				}
				if (a_lhs.remain != a_rhs.remain) {
					return a_lhs.remain < a_rhs.remain;
				}
				return a_lhs.name < a_rhs.name;
			});
		}
	}

	std::string BuildDescription(RE::EffectSetting* a_effect, float a_magnitude, float a_duration, float a_area)
	{
		if (!a_effect) {
			return {};
		}

		// DNAM. This is literally the string the magic menu's effect column
		// prints, tags and all -- so substituting here reproduces that column
		// word for word instead of inventing a second description.
		std::string text{ a_effect->magicItemDescription.c_str() };
		if (text.empty()) {
			return {};
		}

		const std::string magnitude = Number(a_magnitude);
		const std::string duration = Number(a_duration);
		const std::string area = Number(a_area);

		for (const auto* tag : { "<mag>", "<Mag>", "<MAG>" }) {
			ReplaceAll(text, tag, magnitude);
		}
		for (const auto* tag : { "<dur>", "<Dur>", "<DUR>" }) {
			ReplaceAll(text, tag, duration);
		}
		for (const auto* tag : { "<area>", "<Area>", "<AREA>" }) {
			ReplaceAll(text, tag, area);
		}
		return text;
	}

	const Snapshot& Get()
	{
		return g_buffers[g_front.load(std::memory_order_acquire)];
	}

	void RebuildNow()
	{
		g_dirty.store(false, std::memory_order_relaxed);

		Snapshot built{};
		if (auto* player = RE::PlayerCharacter::GetSingleton()) {
			Collect(player, built);
		}
		built.total = static_cast<int>(built.items.size());

		const auto& cfg = Cfg();
		if (built.total > cfg.maxRows) {
			built.items.resize(static_cast<std::size_t>(cfg.maxRows));
		}

		const int back = 1 - g_front.load(std::memory_order_relaxed);
		g_buffers[back] = std::move(built);
		g_front.store(back, std::memory_order_release);
	}

	void RequestRebuild()
	{
		// Already queued or already clean: nothing to do. This is the whole
		// per-frame cost of the mod while the panel is open.
		if (!g_dirty.load(std::memory_order_relaxed) || g_taskPending.exchange(true)) {
			return;
		}

		auto* tasks = SKSE::GetTaskInterface();
		if (!tasks) {
			g_taskPending.store(false);
			return;
		}

		tasks->AddTask([] {
			try {
				RebuildNow();
			} catch (...) {
				spdlog::error("[BuffHUD] snapshot rebuild threw");
			}
			g_taskPending.store(false);
		});
	}

	void Invalidate()
	{
		g_dirty.store(true, std::memory_order_relaxed);
	}
}
