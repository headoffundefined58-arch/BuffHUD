#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Icons.h"

namespace BuffHUD
{
	struct Entry
	{
		std::string           name;
		std::string           source;
		std::string           desc;
		float                 remain{ 0.0f };   // negative = permanent
		float                 magnitude{ 0.0f };
		bool                  detrimental{ false };
		bool                  inactive{ false };
		const Icons::IconDef* icon{ nullptr };
		int                   count{ 1 };
		std::uint32_t         effectID{ 0 };
		int                   archetype{ -1 };
	};

	struct Snapshot
	{
		std::vector<Entry> items;
		int                total{ 0 };  // before maxRows truncation
	};

	namespace Collector
	{
		// Reads the snapshot the main thread produced. Never touches game
		// memory, so the render thread can call it freely.
		[[nodiscard]] const Snapshot& Get();

		// Queues a rebuild onto the game's main thread. The effect list is
		// mutated there, and walking it from the render thread would be a
		// use-after-free waiting to happen. Cheap to call every frame: it only
		// pushes a task when one is not already in flight.
		void RequestRebuild();

		// Runs immediately; main thread only.
		void RebuildNow();

		void Invalidate();

		// BuildDescription() is intentionally NOT declared here: it takes
		// RE::EffectSetting*, and Hud.cpp includes this header without
		// CommonLibSSE. The helper is private to Collector.cpp.
	}
}
