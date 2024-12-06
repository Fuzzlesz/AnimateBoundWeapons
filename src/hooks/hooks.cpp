#include "hooks.h"

namespace Hooks
{
	void Install()
	{
		DontLowerHands::GetSingleton()->Install();
	}

	void DontLowerHands::Install()
	{
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		assert(dataHandler);
		if (!dataHandler) {
			logger::error("Failed to get data handler (hooks). This will lead to a crash later on, unrelated to this mod.");
			return;
		}

		const auto spectre = dataHandler->LookupForm<RE::TESNPC>(0x800, "AnimatedBoundWeapons.esp");
		if (!spectre) {
			logger::error("Failed to get the proper ghost form. Hooks will not be installed.");
			return;
		}
		unsheathingSpectre = spectre;

		REL::Relocation<std::uintptr_t> vtbl{ RE::Character::VTABLE[0] };
		_drawWeaponMagicHands = vtbl.write_vfunc(0xA6, DrawWeaponMagicHands);
	}

	void DontLowerHands::DrawWeaponMagicHands(RE::Character* a_this, bool a_draw)
	{
		if (a_this && a_this->GetActorBase() == unsheathingSpectre && !a_draw) {
			if (a_this->IsCommandedActor() && a_this->GetCommandingActor().get()) {
				if (const auto weaponState = a_this->GetCommandingActor()->GetWeaponState();
					weaponState == RE::WEAPON_STATE::kDrawing 
					|| weaponState == RE::WEAPON_STATE::kWantToDraw
					|| weaponState == RE::WEAPON_STATE::kDrawn) {
					return;
				}
				else {
					a_this->KillImmediate();
				}
			}
		}
		_drawWeaponMagicHands(a_this, a_draw);
	}
}