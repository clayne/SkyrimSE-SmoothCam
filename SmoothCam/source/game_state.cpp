#include "game_state.h"

// Returns the bits for player->actorState->flags04 which appear to convey movement info
const std::bitset<32> GameState::GetPlayerMovementBits(const Actor* player) noexcept {
	const auto bits = std::bitset<32>(player->actorState.flags04);
#ifdef _DEBUG
	// Just to see what actions end up setting these unknown bits
	for (int i = 0; i < 32; i++) {
		if (bits[i]) {
			auto it = knownMovementBits.find(i);
			if (it == knownMovementBits.end()) {
				__debugbreak();
			}
		}
	}
#endif
	return bits;
}

// Returns the bits for player->actorState->flags08 which appear to convey action info
const std::bitset<32> GameState::GetPlayerActionBits(const Actor* player) noexcept {
	const auto bits = std::bitset<32>(player->actorState.flags08);
#ifdef _DEBUG
	// Just to see what actions end up setting these unknown bits
	for (int i = 0; i < 32; i++) {
		if (bits[i]) {
			auto it = knownActionBits.find(i);
			if (it == knownActionBits.end()) {
				__debugbreak();
			}
		}
	}
#endif
	return bits;
}

const bool GameState::IC_InFirstPersonState(const TESObjectREFR* ref, const CorrectedPlayerCamera* camera) noexcept {
	static BSFixedString faceGen = "BSFaceGenNiNodeSkinned";
	if (!ref->loadedState || !ref->loadedState->node || !faceGen.data) return false;

	const auto npc = ref->loadedState->node->GetObjectByName(&faceGen.data);
	if (!npc) return false;

	const auto bits = std::bitset<32>(npc->m_flags);
	return !bits[26];
}

const bool GameState::IFPV_InFirstPersonState(const TESObjectREFR* ref, const CorrectedPlayerCamera* camera) noexcept {
	static BSFixedString faceGen = "BSFaceGenNiNodeSkinned";
	if (!ref->loadedState || !ref->loadedState->node || !faceGen.data) return false;

	const auto npc = ref->loadedState->node->GetObjectByName(&faceGen.data);
	if (!npc) return false;

	const auto bits = std::bitset<32>(npc->m_flags);
	return bits[0];
}

// Returns true if the player is in first person
const bool GameState::IsFirstPerson(const TESObjectREFR* ref, const CorrectedPlayerCamera* camera) noexcept {
	if (Config::GetCurrentConfig()->compatIFPV) {
		const auto fps = camera->cameraState == camera->cameraStates[camera->kCameraState_FirstPerson];
		return fps || IFPV_InFirstPersonState(ref, camera);
	} else {
		return camera->cameraState == camera->cameraStates[camera->kCameraState_FirstPerson];
	}
}

// Returns true if the player is in third person
const bool GameState::IsThirdPerson(const TESObjectREFR* ref, const CorrectedPlayerCamera* camera) noexcept {
	if (Config::GetCurrentConfig()->compatIFPV) {
		const auto tps = camera->cameraState == camera->cameraStates[camera->kCameraState_ThirdPerson2];
		return tps && !IFPV_InFirstPersonState(ref, camera);
	} else {
		return camera->cameraState == camera->cameraStates[camera->kCameraState_ThirdPerson2];
	}
}

// Returns true if the player has a weapon drawn and in third person
const bool GameState::IsThirdPersonCombat(const Actor* player, const CorrectedPlayerCamera* camera) noexcept {
	return GameState::IsThirdPerson(player, camera) && GameState::IsWeaponDrawn(player);
}

// Returns true if a kill move is playing
const bool GameState::IsInKillMove(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_VATS];
}

// Returns true if the camera is tweening
const bool GameState::IsInTweenCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_TweenMenu];
}

// Returns true if the camera is transitioning
const bool GameState::IsInCameraTransition(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Transition];
}

// Returns true if the player is using an object
const bool GameState::IsInUsingObjectCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_ThirdPerson1];
}

// Returns true if the camera is in auto vanity mode
const bool GameState::IsInAutoVanityCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_AutoVanity];
}

// Returns true if the camera is in free mode
const bool GameState::IsInFreeCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Free];
}

// Returns true if the camera is in aiming mode
const bool GameState::IsInAimingCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_IronSights];
}

// Returns true if the camera is in furniture mode
const bool GameState::IsInFurnitureCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Furniture];
}

// Returns true if the player is riding a horse
const bool GameState::IsInHorseCamera(const TESObjectREFR* ref, const CorrectedPlayerCamera* camera) noexcept {
	if (Config::GetCurrentConfig()->compatIFPV) {
		auto horse = camera->cameraStates[PlayerCamera::kCameraState_Horse];
		return camera->cameraState == horse && !IFPV_InFirstPersonState(ref, camera);

	} else {
		return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Horse];
	}
}

// Returns true if the player is bleeding out
const bool GameState::IsInBleedoutCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Bleedout];
}

// Returns true if the player is riding a dragon
const bool GameState::IsInDragonCamera(const CorrectedPlayerCamera* camera) noexcept {
	return camera->cameraState == camera->cameraStates[PlayerCamera::kCameraState_Dragon];
}

const GameState::CameraState GameState::GetCameraState(const Actor* player, const CorrectedPlayerCamera* camera) noexcept {
	GameState::CameraState newState = GameState::CameraState::Unknown;

	if (GameState::IsSleeping(player)) {
		newState = CameraState::FirstPerson;
	} else if (GameState::IsInAutoVanityCamera(camera)) {
		newState = CameraState::Vanity;
	} else if (GameState::IsInTweenCamera(camera)) {
		newState = CameraState::Tweening;
	} else if (GameState::IsInCameraTransition(camera)) {
		newState = CameraState::Transitioning;
	} else if (GameState::IsInUsingObjectCamera(camera)) {
		newState = CameraState::UsingObject;
	} else if (GameState::IsInKillMove(camera)) {
		newState = CameraState::KillMove;
	} else if (GameState::IsInBleedoutCamera(camera)) {
		newState = CameraState::Bleedout;
	} else if (GameState::IsInFreeCamera(camera)) {
		newState = CameraState::Free;
	} else if (GameState::IsInAimingCamera(camera)) {
		newState = CameraState::IronSights;
	} else if (GameState::IsInFurnitureCamera(camera)) {
		newState = CameraState::Furniture;
	} else if (GameState::IsFirstPerson(player, camera)) {
		newState = CameraState::FirstPerson;
	} else if (GameState::IsInHorseCamera(player, camera)) {
		newState = CameraState::Horseback;
	} else if (GameState::IsInDragonCamera(camera)) {
		newState = CameraState::Dragon;
	} else {
		if (GameState::IsThirdPerson(player, camera)) {
			if (GameState::IsThirdPersonCombat(player, camera)) {
				// We have a custom handler for third person with a weapon out
				newState = CameraState::ThirdPersonCombat;
			} else {
				newState = CameraState::ThirdPerson;
			}
		}
	}

#ifdef _DEBUG
	assert(newState != GameState::CameraState::Unknown);
#endif
	return newState;
}

const bool GameState::IsWeaponDrawn(const Actor* player) noexcept {
	const auto bits = GameState::GetPlayerActionBits(player);
	return /*bits[5] &&*/ bits[6]; // Looks like bit 5 flips when switching weapons/magic, which we want to ignore.
}

// Get an equipped weapon
const TESObjectWEAP* GameState::GetEquippedWeapon(const Actor* player, bool leftHand) noexcept {
	if(!player->processManager) return nullptr;

	TESForm* wep = nullptr;
	if(leftHand)
		wep = player->processManager->equippedObject[ActorProcessManager::kEquippedHand_Left];
	else
		wep = player->processManager->equippedObject[ActorProcessManager::kEquippedHand_Right];

	if (!wep || !wep->IsWeapon()) return nullptr;
	return reinterpret_cast<const TESObjectWEAP*>(wep);
}

bool GameState::IsUsingMagicItem(const Actor* player, bool leftHand) noexcept {
	if (leftHand && !player->leftHandSpell) return false;
	if (!leftHand && !player->rightHandSpell) return false;

	EnchantmentItem* ench = leftHand ?
		DYNAMIC_CAST(player->leftHandSpell, TESForm, EnchantmentItem) :
		DYNAMIC_CAST(player->rightHandSpell, TESForm, EnchantmentItem);

	if (!ench) return false;
	const auto wep = GetEquippedWeapon(player, leftHand);
	if (!wep) return false;
	
	// Just look for a staff right now, consider anything else to be non-magic
	return wep->gameData.type == TESObjectWEAP::GameData::kType_Staff ||
		wep->gameData.type == TESObjectWEAP::GameData::kType_Staff2;
}

bool GameState::IsCombatMagic(const SpellItem* spell) noexcept {
	if (!spell) return false;

	const auto spellType = spell->data.type;
	const auto castType = spell->data.castType;
	const auto deliveryType = spell->data.unk14;

	// Spell, leveled spell, scrolls
	if (spellType != 0 && spellType != 9 && spellType != 13)
		return false;

	// On self
	if (deliveryType == 0) return false;

	// Constant effect
	if (castType == 0) return false;

	return true;
}

// Returns true if the player has a melee weapon equiped
const bool GameState::IsMeleeWeaponDrawn(const Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	// Continue to look for a staff here - consider a non-enchanted staff to be melee
	if (right) {
		if (right->gameData.type != TESObjectWEAP::GameData::kType_Bow &&
			/*right->gameData.type != TESObjectWEAP::GameData::kType_Staff &&*/
			right->gameData.type != TESObjectWEAP::GameData::kType_CrossBow &&
			right->gameData.type != TESObjectWEAP::GameData::kType_Bow2 &&
			/*right->gameData.type != TESObjectWEAP::GameData::kType_Staff2 &&*/
			right->gameData.type != TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	if (left) {
		if (left->gameData.type != TESObjectWEAP::GameData::kType_Bow &&
			/*left->gameData.type != TESObjectWEAP::GameData::kType_Staff &&*/
			left->gameData.type != TESObjectWEAP::GameData::kType_CrossBow &&
			left->gameData.type != TESObjectWEAP::GameData::kType_Bow2 &&
			/*left->gameData.type != TESObjectWEAP::GameData::kType_Staff2 &&*/
			left->gameData.type != TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	return false;
}

// Returns true if the player has magic drawn
const bool GameState::IsMagicDrawn(const Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	
	// Using an item that counts as magic
	if (GameState::IsUsingMagicItem(player))
		return true;

	if (GameState::IsUsingMagicItem(player, true))
		return true;

	// Or a spell
	return (GameState::IsCombatMagic(player->leftHandSpell) && !GetEquippedWeapon(player, true)) ||
		(GameState::IsCombatMagic(player->rightHandSpell) && !GetEquippedWeapon(player));
}

// Returns true if the player has a ranged weapon drawn
const bool GameState::IsRangedWeaponDrawn(const Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	if (right) {
		if (right->gameData.type == TESObjectWEAP::GameData::kType_Bow ||
			right->gameData.type == TESObjectWEAP::GameData::kType_CrossBow ||
			right->gameData.type == TESObjectWEAP::GameData::kType_Bow2 ||
			right->gameData.type == TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	if (left) {
		if (left->gameData.type == TESObjectWEAP::GameData::kType_Bow ||
			left->gameData.type == TESObjectWEAP::GameData::kType_CrossBow ||
			left->gameData.type == TESObjectWEAP::GameData::kType_Bow2 ||
			left->gameData.type == TESObjectWEAP::GameData::kType_CBow)
		{
			return true;
		}
	}

	return false;
}

const bool GameState::IsUsingCrossbow(const Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	if (right && (right->gameData.type == TESObjectWEAP::GameData::kType_CrossBow ||
		right->gameData.type == TESObjectWEAP::GameData::kType_CBow))
	{
		return true;
	}

	if (left && (left->gameData.type == TESObjectWEAP::GameData::kType_CrossBow ||
		left->gameData.type == TESObjectWEAP::GameData::kType_CBow))
	{
		return true;
	}

	return false;
}

const bool GameState::IsUsingBow(const Actor* player) noexcept {
	if (!GameState::IsWeaponDrawn(player)) return false;
	const auto right = GameState::GetEquippedWeapon(player);
	const auto left = GameState::GetEquippedWeapon(player, true);

	if (right && (right->gameData.type == TESObjectWEAP::GameData::kType_Bow ||
		right->gameData.type == TESObjectWEAP::GameData::kType_Bow2))
	{
		return true;
	}

	if (left && (left->gameData.type == TESObjectWEAP::GameData::kType_Bow ||
		left->gameData.type == TESObjectWEAP::GameData::kType_Bow2))
	{
		return true;
	}

	return false;
}

// Returns true if the player is sneaking
const bool GameState::IsSneaking(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[9];
}

// Returns true if the player is sprinting
const bool GameState::IsSprinting(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[8];
}

// Returns true if the player is running
const bool GameState::IsRunning(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[7];
}

// Returns true if the player is swimming
const bool GameState::IsSwimming(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[10];
}

// Returns true if the player is walking
const bool GameState::IsWalking(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (movementBits[0] || movementBits[1]) && (movementBits[2] || movementBits[3]) &&
		movementBits[6];
}

// Returns true if the player is holding a bow and an arrow is drawn
const bool GameState::IsBowDrawn(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	const auto actionBits = GameState::GetPlayerActionBits(player);

	// AGO compat
	if (Config::GetCurrentConfig()->compatAGO && !GameState::IsUsingCrossbow(player)) {
		// We get 1 frame here with AGO where we are at our limits just checking the action bits
		// This convoluted bit of a state machine works around that
		static float lastDrawnTimer = 0.0f;
		static bool drawnLastCall = false;
		static bool objective = false;

		if (!movementBits[31] || movementBits[30]/* || actionBits[3]*/) {
			drawnLastCall = objective = false;
			return false;
		}

		// Bits 28 & 29 flip around, randomly, it seems.
		// We want to see bit 29 get set.
		if (movementBits[29]) objective = true;

		if (!drawnLastCall && objective) {
			drawnLastCall = true;
			lastDrawnTimer = GameTime::CurTime() + 0.1f;
		}

		if (objective && GameTime::CurTime() > lastDrawnTimer) {
			return true;
		}

		return false;
	} else {
		if (GameState::IsUsingCrossbow(player)) {
			// Crossbows are a bit different
			return movementBits[31] && movementBits[29];
		} else {
			return movementBits[31];
		}
	}
}

// Returns true if the player is sitting
const bool GameState::IsSitting(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return movementBits[14] && movementBits[15];
}

// Returns true if the player is sleeping
const bool GameState::IsSleeping(const Actor* player) noexcept {
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (GameState::IsSitting(player) && movementBits[16]) || // in bed
		movementBits[15] && movementBits[16] || //getting in bed
		movementBits[17]; // getting out of bed
}

// Returns true if the player is mounting a horse
const bool GameState::IsMountingHorse(const Actor* player) noexcept {
	const auto actionBits = GameState::GetPlayerActionBits(player);
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return (actionBits[3] && actionBits[12] && movementBits[15] || (
		actionBits[12] && movementBits[15]
	)) && !movementBits[14];
}

// Returns true if the player is dismounting a horse
const bool GameState::IsDisMountingHorse(const Actor* player) noexcept {
	const auto actionBits = GameState::GetPlayerActionBits(player);
	const auto movementBits = GameState::GetPlayerMovementBits(player);
	return actionBits[3] && actionBits[12] && (movementBits[16]);
}

const bool GameState::InPOVSlideMode() noexcept {
	return PlayerControls::GetSingleton()->unk04B;
}

// Returns true if the player is a vampire lord
const bool GameState::IsVampireLord(const Actor* player) noexcept {
	if (!player->race) return false;
	return strcmp(player->race->fullName.name.c_str(), "Vampire Lord") == 0;
}

// Returns true if the player is a werewolf
const bool GameState::IsWerewolf(const Actor* player) noexcept {
	if (!player->race) return false;
	return strcmp(player->race->fullName.name.c_str(), "Werewolf") == 0;
}