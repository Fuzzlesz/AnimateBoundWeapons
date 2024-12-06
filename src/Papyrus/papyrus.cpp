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

		// Simple AME manipulation in case there is a mismatch between bound weapon duration and summon effect duration.
		uint32_t maxDur = 0;
		for (auto* effect : boundSpell->effects) {
			if (!effect || !effect->baseEffect) {
				continue;
			}
			const auto baseEffect = effect->baseEffect;
			if (baseEffect->GetArchetype() != RE::EffectSetting::Archetype::kBoundWeapon) {
				continue;
			}

			const auto currentDur = effect->GetDuration();
			maxDur = currentDur > maxDur ? currentDur : maxDur;
		}
		magicCaster->CastSpellImmediate(boundSpell, false, creature, 1.0f, false, 0.0f, creature);
		summonEffect->elapsedSeconds = 0.0f;
		summonEffect->duration = static_cast<float>(maxDur);
	}

	void Bind(VM& a_vm) {
		BIND(AnimateWeapon);
	}

	bool RegisterFunctions(VM* a_vm) {
		Bind(*a_vm);
		return true;
	}
}