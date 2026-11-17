#pragma once

// <windows.h> must never appear here: CommonLibSSE ships its own REX::W32
// declarations of the same types and hard-errors if the SDK wins the race.
// Anything that needs the real Windows SDK lives in ImGuiHost.cpp / Loc.cpp,
// which are excluded from the precompiled header.

#include <RE/Skyrim.h>
#include <REL/Relocation.h>
#include <SKSE/SKSE.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
