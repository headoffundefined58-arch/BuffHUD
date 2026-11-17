#include "PCH.h"

#include "Collector.h"
#include "Config.h"
#include "Framework.h"
#include "Icons.h"
#include "Input.h"
#include "Loc.h"
#include "Paths.h"
#include "Rules.h"

#include <spdlog/sinks/basic_file_sink.h>

// Both runtimes are mandatory for this plugin. See CMakeLists.txt: with AE
// only, REL::Module::GetRuntime() always answers AE, and every REL::VariantID
// resolves through the AE id against an SE address library (i.e. to zero).
#if !defined(ENABLE_SKYRIM_SE) || !defined(ENABLE_SKYRIM_AE)
#	error "BuffHUD must be built with ENABLE_SKYRIM_SE and ENABLE_SKYRIM_AE both ON"
#endif

namespace
{
	// CommonLibSSE-NG installs no file sink, so without this the plugin is
	// completely silent in a game process.
	void SetupLog()
	{
		auto directory = SKSE::log::log_directory();
		if (!directory) {
			return;
		}

		*directory /= "BuffHUD.log";

		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(directory->string(), true);
		sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

		auto logger = std::make_shared<spdlog::logger>("buffhud", std::move(sink));
		logger->set_level(spdlog::level::debug);
		logger->flush_on(spdlog::level::debug);

		spdlog::set_default_logger(std::move(logger));
		spdlog::set_level(spdlog::level::debug);
	}

	void LoadAssets()
	{
		BuffHUD::Icons::Load();
		BuffHUD::Rules::Load();
		BuffHUD::Loc::Load(BuffHUD::Cfg().language, BuffHUD::Framework::SystemIsChinese());
		BuffHUD::Collector::Invalidate();
	}

	void OnSKSEMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		try {
			switch (a_msg->type) {
			case SKSE::MessagingInterface::kDataLoaded:
				spdlog::info("[BuffHUD] data loaded, plugin data at {}", BuffHUD::Paths::PluginData().string());
				LoadAssets();
				if (BuffHUD::Input::Init()) {
					spdlog::info("[BuffHUD] hotkey {} armed (hold {} ms)", BuffHUD::Cfg().key, BuffHUD::Cfg().holdMs);
				}
				BuffHUD::Framework::Register();
				break;

			case SKSE::MessagingInterface::kNewGame:
			case SKSE::MessagingInterface::kPostLoadGame:
				// A load replaces the actor; the old snapshot is meaningless.
				BuffHUD::LoadConfig();
				LoadAssets();
				break;

			default:
				break;
			}
		} catch (const std::exception& e) {
			// Nothing may escape an SKSE callback: it would take the game with it.
			spdlog::error("[BuffHUD] message handler threw: {}", e.what());
		} catch (...) {
			spdlog::error("[BuffHUD] message handler threw an unknown exception");
		}
	}
}

SKSEPluginInfo(
	.Version = REL::Version{ 1, 2, 0, 0 },
	.Name = "BuffHUD",
	.Author = "Icarus",
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary);

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);

	SetupLog();

	spdlog::info("[BuffHUD] loading 1.2.0 (runtime {})", REL::Module::get().version().string());
	spdlog::info("[BuffHUD] built for SE + AE; this process is {}",
		REL::Module::IsSE() ? "SE (1.5.97)" : "AE (1.6.x)");

	BuffHUD::LoadConfig();

	if (const auto* messaging = SKSE::GetMessagingInterface()) {
		messaging->RegisterListener(OnSKSEMessage);
	} else {
		spdlog::error("[BuffHUD] no messaging interface");
		return false;
	}

	return true;
}
