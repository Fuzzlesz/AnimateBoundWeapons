#include "papyrus.h"

#include "RE/misc.h"

namespace Papyrus
{
	void AnimateWeapon(STATIC_ARGS, RE::Actor* a_actor) {
		if (!a_actor || !a_actor->IsCommandedActor()) {
			return;
		}

		const auto commandingActor = a_actor->GetCommandingActor().get();
		const auto middleHigh = commandingActor ? commandingActor->GetMiddleHighProcess() : nullptr;
		if (!middleHigh) {
			return;
		}
		if (commandingActor != RE::PlayerCharacter::GetSingleton()) {
			return;
		}

		const auto& commandedActors = middleHigh->commandedActors;
		if (commandedActors.empty()) {
			return;
		}
		RE::SummonCreatureEffect* summonEffect = nullptr;
		for (const auto commandee : commandedActors) {
			if (commandee.commandedActor.get().get() != a_actor) {
				continue;
			}
			summonEffect = commandee.activeEffect ? skyrim_cast<RE::SummonCreatureEffect*>(commandee.activeEffect) : nullptr;
		}
		if (!summonEffect) {
			return;
		}
		const auto creature = summonEffect->commandedActor.get().get();
		if (!creature) {
			return;
		}
		const auto boundSpell = summonEffect->spell;
		if (!boundSpell) {
			return;
		}
		const auto magicCaster = creature->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand);
		if (!magicCaster) {
			return;
		}

		RE::SetPlayerTeammate(a_actor, true, true);

		//Duration fuckery - The spell was effectively dual cast, hence duration mismatch between bound weapon and summon
		std::vector<uint32_t> oldDurations{};
		float remainder = summonEffect->duration - summonEffect->elapsedSeconds;
		for (auto* effect : boundSpell->effects) {
			oldDurations.push_back(effect->GetDuration());
			effect->effectItem.duration = static_cast<uint32_t>(remainder);
		}
		magicCaster->CastSpellImmediate(boundSpell, false, creature, 1.0f, false, 0.0f, creature);

		auto durIt = oldDurations.begin();
		for (auto effIt = boundSpell->effects.begin(); effIt != boundSpell->effects.end() && durIt != oldDurations.end(); ++effIt, ++durIt) {
			(*effIt)->effectItem.duration = (*durIt);
		}
	}

	void Bind(VM& a_vm) {
		BIND(AnimateWeapon);
	}

	bool RegisterFunctions(VM* a_vm) {
		Bind(*a_vm);
		return true;
	}
}