#include "PCH.h"

#include "Config.h"
#include "Collector.h"
#include "Input.h"
#include "Menu.h"

namespace BuffHUD::Input
{
	namespace
	{
		bool                                 g_held{ false };
		bool                                 g_pinned{ false };
		bool                                 g_longFired{ false };
		std::chrono::steady_clock::time_point g_pressedAt{};

		// Key-binding capture, driven by the settings page. g_capturing is set
		// from the render thread (on a button press) and read here on the game's
		// input pump; both are infrequent, so relaxed atomics are enough.
		std::atomic<bool> g_capturing{ false };
		std::atomic<int>  g_capturedKey{ -1 };

		class Sink final : public RE::BSTEventSink<RE::InputEvent*>
		{
		public:
			RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_events,
				RE::BSTEventSource<RE::InputEvent*>*) override
			{
				if (!a_events) {
					return RE::BSEventNotifyControl::kContinue;
				}

				try {
					const auto& cfg = Cfg();
					if (cfg.key <= 0) {
						return RE::BSEventNotifyControl::kContinue;
					}

					// While a menu is up the player is not playing; a hotkey
					// firing behind the inventory would be surprising.
					if (auto* ui = RE::UI::GetSingleton(); ui && ui->GameIsPaused()) {
						return RE::BSEventNotifyControl::kContinue;
					}

					for (auto* event = *a_events; event; event = event->next) {
						auto* button = event->AsButtonEvent();
						if (!button) {
							continue;
						}
						if (button->GetDevice() != RE::INPUT_DEVICE::kKeyboard) {
							continue;
						}

						// While capturing, record the next key press for the
						// settings page and suppress the HUD hotkey logic.
						if (g_capturing.load(std::memory_order_relaxed)) {
							if (button->IsDown()) {
								g_capturedKey.store(static_cast<int>(button->GetIDCode()),
									std::memory_order_relaxed);
								g_capturing.store(false, std::memory_order_relaxed);
							}
							continue;
						}

						if (button->GetIDCode() != static_cast<std::uint32_t>(cfg.key)) {
							continue;
						}

						// IsDown() is only true on the first frame of a press,
						// so auto-repeat cannot retrigger us.
						if (button->IsDown()) {
							g_held = true;
							g_longFired = false;
							g_pressedAt = std::chrono::steady_clock::now();
						} else if (button->IsUp() && g_held) {
							g_held = false;
							if (!g_longFired) {
								// Short tap: latch the panel open so the
								// player can read it hands-free.
								g_pinned = !g_pinned;
								Collector::Invalidate();
							}
						}
					}
				} catch (...) {
					// An exception escaping an SKSE callback kills the game.
				}

				return RE::BSEventNotifyControl::kContinue;
			}
		};

		Sink g_sink{};
	}

	bool Init()
	{
		auto* manager = RE::BSInputDeviceManager::GetSingleton();
		if (!manager) {
			spdlog::error("[BuffHUD] no input device manager");
			return false;
		}
		manager->AddEventSink(&g_sink);
		spdlog::info("[BuffHUD] input sink registered");
		return true;
	}

	void Tick()
	{
		if (!g_held || g_longFired) {
			return;
		}
		const auto held = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - g_pressedAt);
		if (held.count() >= Cfg().holdMs) {
			g_longFired = true;
			Collector::Invalidate();
		}
	}

	bool PanelVisible()
	{
		Tick();
		const bool shown = g_pinned || (g_held && g_longFired);
		if (!shown) {
			return false;
		}

		// "Combat only" is a filter on top of the hotkey, not a second trigger:
		// a pinned panel still disappears the moment the fight ends. Reading
		// IsInCombat() is a cached flag lookup, no combat scan involved.
		if (Cfg().combatOnly) {
			const auto* player = RE::PlayerCharacter::GetSingleton();
			return player && player->IsInCombat();
		}
		return true;
	}

	void BeginCapture()
	{
		g_capturedKey.store(-1, std::memory_order_relaxed);
		g_capturing.store(true, std::memory_order_relaxed);
	}

	bool IsCapturing()
	{
		return g_capturing.load(std::memory_order_relaxed);
	}

	int ConsumeCapturedKey()
	{
		const int v = g_capturedKey.exchange(-1, std::memory_order_relaxed);
		return v >= 0 ? v : -1;
	}
}
