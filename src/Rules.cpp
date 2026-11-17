#include "PCH.h"

#include "Rules.h"
#include "Paths.h"

#include <fstream>
#include <sstream>

namespace BuffHUD::Rules
{
	namespace
	{
		std::unordered_map<std::string, std::string> g_byKeyword;
		std::unordered_map<std::string, std::string> g_byArchetype;
		std::unordered_map<std::string, std::string> g_byActorValue;

		// Index == EffectArchetypes::ArchetypeID (0..46). Ordered exactly as
		// the enum in RE/E/EffectArchetypes.h so a static_cast can index it.
		constexpr const char* kArchetypeNames[] = {
			"valuemodifier",      // 0
			"script",             // 1
			"dispel",             // 2
			"curedisease",        // 3
			"absorb",             // 4
			"dualvaluemodifier",  // 5
			"calm",               // 6
			"demoralize",         // 7
			"frenzy",             // 8
			"disarm",             // 9
			"commandsummoned",    // 10
			"invisibility",       // 11
			"light",              // 12
			"darkness",           // 13
			"nighteye",           // 14
			"lock",               // 15
			"open",               // 16
			"boundweapon",        // 17
			"summoncreature",     // 18
			"detectlife",         // 19
			"telekinesis",        // 20
			"paralysis",          // 21
			"reanimate",          // 22
			"soultrap",           // 23
			"turnundead",         // 24
			"guide",              // 25
			"werewolffeed",       // 26
			"cureparalysis",      // 27
			"cureaddiction",      // 28
			"curepoison",         // 29
			"concussion",         // 30
			"valueandparts",      // 31
			"accumulatemagnitude",// 32
			"stagger",            // 33
			"peakvaluemodifier",  // 34
			"cloak",              // 35
			"werewolf",           // 36
			"slowtime",           // 37
			"rally",              // 38
			"enhanceweapon",      // 39
			"spawnhazard",        // 40
			"etherealize",        // 41
			"banish",             // 42
			"spawnscriptedref",   // 43
			"disguise",           // 44
			"grabactor",          // 45
			"vampirelord"         // 46
		};

		[[nodiscard]] std::string_view AvToken(RE::ActorValue a_value)
		{
			switch (a_value) {
			case RE::ActorValue::kHealth: return "health";
			case RE::ActorValue::kMagicka: return "magicka";
			case RE::ActorValue::kStamina: return "stamina";
			case RE::ActorValue::kSpeedMult: return "speedmult";
			case RE::ActorValue::kDamageResist: return "damageresist";
			case RE::ActorValue::kResistFire: return "resistfire";
			case RE::ActorValue::kResistFrost: return "resistfrost";
			case RE::ActorValue::kResistShock: return "resistshock";
			case RE::ActorValue::kResistMagic: return "resistmagic";
			case RE::ActorValue::kResistDisease: return "resistdisease";
			case RE::ActorValue::kCarryWeight: return "carryweight";
			case RE::ActorValue::kMeleeDamage: return "meleedamage";
			case RE::ActorValue::kAttackDamageMult: return "attackdamagemult";
			case RE::ActorValue::kDestruction: return "destruction";
			case RE::ActorValue::kRestoration: return "restoration";
			case RE::ActorValue::kConjuration: return "conjuration";
			case RE::ActorValue::kIllusion: return "illusion";
			case RE::ActorValue::kAlteration: return "alteration";
			case RE::ActorValue::kSneak: return "sneak";
			case RE::ActorValue::kWaterBreathing: return "waterbreathing";
			default: return {};
			}
		}

		[[nodiscard]] bool IsValueModifierFamily(int a_archetype)
		{
			// Only these four touch actor values; without the restriction the
			// Fire Damage effect (also a value modifier) would wear the heart.
			return a_archetype == 0 || a_archetype == 5 || a_archetype == 31 || a_archetype == 34;
		}

		[[nodiscard]] const Icons::IconDef* Lookup(const std::unordered_map<std::string, std::string>& a_table,
			std::string_view                                                             a_key)
		{
			if (a_key.empty()) {
				return nullptr;
			}
			std::string lowered{ a_key };
			std::ranges::transform(lowered, lowered.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

			const auto it = a_table.find(lowered);
			if (it == a_table.end()) {
				return nullptr;
			}
			return Icons::Find(it->second);
		}
	}

	bool Load()
	{
		g_byKeyword.clear();
		g_byArchetype.clear();
		g_byActorValue.clear();

		const auto path = Paths::PluginData() / "rules.tsv";
		std::ifstream file(path, std::ios::binary);
		if (!file) {
			spdlog::warn("[BuffHUD] rules.tsv missing at {}; every effect will fall back", path.string());
			return false;
		}

		std::string line;
		while (std::getline(file, line)) {
			if (line.empty() || line.front() == '#') {
				continue;
			}
			std::istringstream stream(line);
			std::string        kind;
			std::string        key;
			std::string        icon;
			if (!(stream >> kind >> key >> icon)) {
				continue;
			}
			std::ranges::transform(key, key.begin(),
				[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

			if (kind == "K") {
				g_byKeyword[key] = icon;
			} else if (kind == "A") {
				g_byArchetype[key] = icon;
			} else if (kind == "V") {
				g_byActorValue[key] = icon;
			}
		}

		spdlog::info("[BuffHUD] rules: {} keyword, {} archetype, {} actor-value",
			g_byKeyword.size(), g_byArchetype.size(), g_byActorValue.size());
		return true;
	}

	const char* ArchetypeName(int a_archetype)
	{
		constexpr int count = static_cast<int>(std::size(kArchetypeNames));
		if (a_archetype >= 0 && a_archetype < count) {
			return kArchetypeNames[a_archetype];
		}
		return "none";
	}

	const Icons::IconDef* Resolve(RE::EffectSetting* a_effect)
	{
		if (!a_effect) {
			return Icons::Unknown();
		}

		// 1. Keywords: the most specific signal a mod can give us.
		for (auto* keyword : a_effect->GetKeywords()) {
			if (!keyword) {
				continue;
			}
			if (const char* editorID = keyword->GetFormEditorID(); editorID && *editorID) {
				if (const auto* def = Lookup(g_byKeyword, editorID)) {
					return def;
				}
			}
		}

		const int archetype = static_cast<int>(a_effect->GetArchetype());

		// 2. Archetype.
		if (const auto* def = Lookup(g_byArchetype, ArchetypeName(archetype))) {
			return def;
		}

		// 3. Actor value, but only where it actually tells us something.
		if (IsValueModifierFamily(archetype)) {
			if (const auto* def = Lookup(g_byActorValue, AvToken(a_effect->data.primaryAV))) {
				return def;
			}
		}

		// 4. Polarity.
		if (a_effect->IsDetrimental() || a_effect->IsHostile()) {
			return Icons::Find("debuff_down");
		}
		return Icons::Find("buff_up");
	}
}
