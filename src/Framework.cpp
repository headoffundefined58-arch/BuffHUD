// SMF integration boundary.
//
// This is the ONLY translation unit that includes SKSEMenuFramework.h (which pulls
// in <windows.h>). It deliberately does not use the project precompiled header, so
// CommonLibSSE's REX::W32 types are never dragged in ahead of the Windows SDK and
// the two never clash in one TU. It also avoids including any header that
// transitively pulls CommonLibSSE, so there is nothing to conflict with here.
//
// The render callbacks themselves live in their own translation units (Hud.cpp,
// Menu.cpp — which DO use the PCH and CommonLibSSE); we only need their
// __stdcall signatures to register them with SMF.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Smf.h"

#include <cstdio>
#include <cstring>
#include <string>

#include <spdlog/spdlog.h>

#include "Loc.h"

// Callbacks defined elsewhere; only their __stdcall signatures are needed here.
namespace BuffHUD::Hud
{
	void __stdcall Render();
}
namespace BuffHUD::Menu
{
	void __stdcall RenderSettings();
}
namespace BuffHUD::Input
{
	bool PanelVisible();
}

namespace
{
	void __stdcall HudRender()
	{
		// SMF draws HUD elements every frame. Bail out when a blocking menu
		// (inventory, Mod Control Panel, dialogue, ...) is open so the panel
		// never paints over the game UI.
		if (SKSEMenuFramework::IsAnyBlockingWindowOpened()) {
			return;
		}
		// The HUD only appears while the player holds, or has pinned, the
		// hotkey; Input::PanelVisible() owns that state.
		if (!BuffHUD::Input::PanelVisible()) {
			return;
		}
		BuffHUD::Hud::Render();
	}

	void __stdcall MenuRender()
	{
		BuffHUD::Menu::RenderSettings();
	}
}

namespace BuffHUD::Framework
{
	bool g_ready{ false };

	// AddHudElement() returns a new HudElement whose destructor UNREGISTERS the
	// callback. Keep the pointer for the lifetime of the module - discarding it
	// only "works" by leaking, and a scoped object would kill the HUD at once.
	SKSEMenuFramework::Model::HudElement* g_hud{ nullptr };

	void Register()
	{
		// GetModuleHandleW is authoritative: IsInstalled() probes the relative
		// path "Data/SKSE/Plugins/...", which depends on the process CWD.
		HMODULE smf = ::GetModuleHandleW(L"SKSEMenuFramework");
		if (!smf && !SKSEMenuFramework::IsInstalled()) {
			spdlog::warn("[BuffHUD] SKSE Menu Framework not found; settings + HUD disabled. "
				"Install SKSE Menu Framework 3.x to use BuffHUD.");
			g_ready = false;
			return;
		}
		if (!smf) {
			spdlog::warn("[BuffHUD] SMF dll present on disk but not loaded in this process");
			return;
		}

		using VersionFn = float (*)();
		float version = 0.0f;
		if (auto fn = reinterpret_cast<VersionFn>(::GetProcAddress(smf, "GetMenuFrameworkVersion"))) {
			version = fn();
		}
		spdlog::info("[BuffHUD] SKSE Menu Framework v{:.2f} detected", version);

		// The item name is shown twice: as the child node under "BuffHUD" in
		// the Mod Control Panel tree, and as the page title. Take it from the
		// active language pack so a Chinese install reads 设置. SetSection
		// keeps the ASCII "BuffHUD" path prefix; only the leaf is localised.
		const auto label = BuffHUD::Loc::Get("menu.page");
		const std::string page = label.empty() ? std::string{ "Settings" }
		                                      : std::string{ label };
		SKSEMenuFramework::SetSection("BuffHUD");
		SKSEMenuFramework::AddSectionItem(page, &MenuRender);

		// Always-on foreground HUD element driven by the hotkey.
		g_hud = SKSEMenuFramework::AddHudElement(&HudRender);
		if (!g_hud) {
			spdlog::warn("[BuffHUD] could not allocate SMF HUD element");
		}

		g_ready = true;
		spdlog::info("[BuffHUD] registered with SKSE Menu Framework "
			"(Mod Control Panel -> BuffHUD)");
	}

	bool Ready()
	{
		return g_ready;
	}
}

namespace BuffHUD::Framework
{

bool SystemIsChinese()
{
	const LANGID lang = ::GetUserDefaultUILanguage();
	return PRIMARYLANGID(lang) == LANG_CHINESE;
}

const char* KeyName(int a_scan)
{
	static char buf[64]{};
	if (a_scan <= 0) {
		std::strncpy(buf, "-", sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = '\0';
		return buf;
	}

	const UINT vk = ::MapVirtualKeyW(static_cast<UINT>(a_scan), MAPVK_VSC_TO_VK_EX);
	LONG  lparam = (static_cast<LONG>(a_scan) & 0xFF) << 16;
	if (vk >= VK_PRIOR && vk <= VK_DOWN) {
		lparam |= 1L << 24;
	}
	wchar_t name[64]{};
	if (::GetKeyNameTextW(lparam, name, static_cast<int>(std::size(name))) > 0) {
		const int n = ::WideCharToMultiByte(CP_UTF8, 0, name, -1, buf,
			static_cast<int>(sizeof(buf)) - 1, nullptr, nullptr);
		if (n > 0) {
			buf[n - 1] = '\0';
			return buf;
		}
	}
	std::snprintf(buf, sizeof(buf), "0x%02X", a_scan);
	return buf;
}

}  // namespace BuffHUD::Framework
