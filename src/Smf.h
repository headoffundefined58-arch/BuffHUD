#pragma once

// SKSEMenuFramework.h includes <windows.h> unconditionally. Without NOMINMAX
// its min/max macros rewrite std::max(...) in Hud.cpp into a syntax error, and
// WIN32_LEAN_AND_MEAN keeps the SDK out of the CommonLibSSE TU boundary.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Central include for the SkyUI SKSE Menu Framework consumer surface.
//
// SKSEMenuFramework.h is the only translation-unit entry point to SMF's ImGui
// and HUD APIs. It pulls in <windows.h> transitively, so any TU that includes
// this header MUST NOT also include the project precompiled header (PCH.h),
// which drags CommonLibSSE's competing REX::W32 declarations in ahead of the
// Windows SDK. Hud.cpp, Menu.cpp and Framework.cpp are therefore excluded from
// the precompiled header in CMakeLists.txt.
//
// We pre-include the small standard headers the framework's inline functions
// reference (std::filesystem is used by IsInstalled(), etc.) so those TUs do
// not need the PCH to supply them.

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <string>

// The SDK header uses std::atomic in WindowInterface and names RE::InputEvent
// in an input-callback typedef without declaring either. Supply both here:
// including CommonLibSSE instead would drag REX::W32 into the same TU as
// <windows.h> and hard-error. (Same trick CMS uses in fwio.cpp.)
namespace RE
{
	class InputEvent;
}

#include "SKSEMenuFramework.h"
