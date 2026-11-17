#pragma once

#include "Icons.h"

namespace RE
{
	class EffectSetting;
}

namespace BuffHUD::Rules
{
	// Resolves which icon an active effect should wear, in this order:
	//   keyword -> archetype -> actor value (value-modifier family only) ->
	//   detrimental flag -> unknown.
	//
	// The tables live in <PluginData>\rules.tsv and are generated from
	// resources/rules/icon_rules.json, so a new mod's effects resolve without
	// touching code: anything unrecognised falls through to the flag test and
	// finally to the 'unknown' tile rather than being dropped.
	bool Load();

	[[nodiscard]] const Icons::IconDef* Resolve(RE::EffectSetting* a_effect);

	// English name of the archetype, for the debug pane and the log.
	[[nodiscard]] const char* ArchetypeName(int a_archetype);
}
