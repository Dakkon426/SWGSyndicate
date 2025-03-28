/*
 * SlicingSessionImplementation.cpp
 *
 *  Created on: Mar 5, 2011
 *      Author: polonel
 */

#include "server/zone/objects/player/sessions/SlicingSession.h"
#include "server/zone/objects/player/sui/SuiWindowType.h"
#include "server/zone/objects/player/sui/listbox/SuiListBox.h"
#include "server/zone/objects/tangible/tool/smuggler/SlicingTool.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/loot/LootManager.h"
#include "server/zone/managers/gcw/GCWManager.h"
#include "server/zone/managers/gcw/tasks/SecuritySliceTask.h"
#include "server/zone/objects/tangible/Container.h"
#include "server/zone/objects/tangible/RelockLootContainerEvent.h"
#include "server/zone/objects/tangible/weapon/WeaponObject.h"
#include "server/zone/objects/tangible/wearables/ArmorObject.h"
#include "server/zone/objects/tangible/terminal/mission/MissionTerminal.h"
#include "server/zone/objects/tangible/tool/smuggler/PrecisionLaserKnife.h"
#include "server/zone/objects/tangible/powerup/PowerupObject.h"

#include "server/zone/objects/player/sessions/sui/SlicingSessionSuiCallback.h"

#include "server/zone/ZoneServer.h"

#include "server/zone/Zone.h"
#include "server/zone/objects/scene/SceneObjectType.h"

int SlicingSessionImplementation::initializeSession() {
	firstCable = System::random(1);
	nodeCable = 0;

	cableBlue = false;
	cableRed = false;

	usedNode = false;
	usedClamp = false;

	relockEvent = nullptr;

	baseSlice = false;
	keypadSlice = false;

	return 0;
}

void SlicingSessionImplementation::initalizeSlicingMenu(CreatureObject* pl, TangibleObject* obj) {
	player = pl;
	tangibleObject = obj;

	ManagedReference<CreatureObject*> player = pl;
	ManagedReference<TangibleObject*> tangibleObject = obj;

	if (player == nullptr || tangibleObject == nullptr)
		return;

	if (!tangibleObject->isSliceable() && !isBaseSlice() && !isKeypadSlice())
		return;

	if (tangibleObject->containsActiveSession(SessionFacadeType::SLICING)) {
		player->sendSystemMessage("@slicing/slicing:slicing_underway");
		return;
	}

	if (!hasPrecisionLaserKnife(false)) { // do not remove the item on inital window
		player->sendSystemMessage("@slicing/slicing:no_knife");
		return;
	}

	//bugfix 814,819
	ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");
	if (inventory == nullptr)
		return;

	if(!isBaseSlice() && !isKeypadSlice()){
		if (!inventory->hasObjectInContainer(tangibleObject->getObjectID()) && tangibleObject->getGameObjectType() != SceneObjectType::STATICLOOTCONTAINER
				&& tangibleObject->getGameObjectType() != SceneObjectType::MISSIONTERMINAL ) {
			player->sendSystemMessage("The object must be in your inventory in order to perform the slice.");
			return;
		}

		if (tangibleObject->isWeaponObject() && !hasWeaponUpgradeKit()) {
			player->sendSystemMessage("@slicing/slicing:no_weapon_kit");
			return;
		}

		if (tangibleObject->isArmorObject() && !hasArmorUpgradeKit()) {
			player->sendSystemMessage("@slicing/slicing:no_armor_kit");
			return;
		}
	}

	slicingSuiBox = new SuiListBox(player, SuiWindowType::SLICING_MENU, 2);
	slicingSuiBox->setCallback(new SlicingSessionSuiCallback(player->getZoneServer()));

	if (tangibleObject->getGameObjectType() == SceneObjectType::PLAYERLOOTCRATE)
		// Don't close the window when we remove PlayerLootContainer from the player's inventory.
		slicingSuiBox->setForceCloseDisabled();

	slicingSuiBox->setPromptTitle("@slicing/slicing:title");
	slicingSuiBox->setUsingObject(tangibleObject);
	slicingSuiBox->setCancelButton(true, "@cancel");
	generateSliceMenu(slicingSuiBox);

	player->getPlayerObject()->addSuiBox(slicingSuiBox);

	player->addActiveSession(SessionFacadeType::SLICING, _this.getReferenceUnsafeStaticCast());
	tangibleObject->addActiveSession(SessionFacadeType::SLICING, _this.getReferenceUnsafeStaticCast());
}

void SlicingSessionImplementation::generateSliceMenu(SuiListBox* suiBox) {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (player == nullptr || tangibleObject == nullptr)
		return;

	uint8 progress = getProgress();
	suiBox->removeAllMenuItems();

	StringBuffer prompt;
	prompt << "@slicing/slicing:";
	prompt << getPrefix(tangibleObject);

	if (progress == 0) {
		if (usedClamp)
			prompt << "clamp_" << firstCable;
		else if (usedNode)
			prompt << "analyze_" << nodeCable;
		else
			prompt << progress;

		suiBox->addMenuItem("@slicing/slicing:blue_cable", 0);
		suiBox->addMenuItem("@slicing/slicing:red_cable", 1);

		if (!usedClamp && !usedNode) {
			suiBox->addMenuItem("@slicing/slicing:use_clamp", 2);
			suiBox->addMenuItem("@slicing/slicing:use_analyzer", 3);
		}

	} else if (progress == 1) {
		prompt << progress;

		suiBox->addMenuItem((cableBlue) ? "@slicing/slicing:blue_cable_cut" : "@slicing/slicing:blue_cable", 0);
		suiBox->addMenuItem((cableRed) ? "@slicing/slicing:red_cable_cut" : "@slicing/slicing:red_cable", 1);
	}

	suiBox->setPromptText(prompt.toString());
	player->getPlayerObject()->addSuiBox(suiBox);
	player->sendMessage(suiBox->generateMessage());

}

void SlicingSessionImplementation::handleMenuSelect(CreatureObject* pl, byte menuID, SuiListBox* suiBox) {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (tangibleObject == nullptr || player == nullptr || player != pl)
		return;

	ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");
	if (inventory == nullptr)
		return;

	if(!isBaseSlice() && !isKeypadSlice() && tangibleObject->getGameObjectType() != SceneObjectType::STATICLOOTCONTAINER && tangibleObject->getGameObjectType() != SceneObjectType::MISSIONTERMINAL){
		if (!inventory->hasObjectInContainer(tangibleObject->getObjectID())) {
			player->sendSystemMessage("The object must be in your inventory in order to perform the slice.");
			return;
		}
	}

	uint8 progress = getProgress();

	// Check if this is the weapon slice choice menu
	if (suiBox->getPromptTitle() == "Weapon Slice Options") {
		if (menuID == 0 || menuID == 1) {
			handleWeaponSlice(menuID);
		} else {
			// If the player cancels, mark the object as sliced but don't apply any mods
			// This ensures they can't exploit the system for XP without actually slicing
			if (tangibleObject->isWeaponObject()) {
				WeaponObject* weap = cast<WeaponObject*>(tangibleObject.get());
				Locker locker(weap);
				
				if (weap->hasPowerup())
					this->detachPowerUp(player, weap);
					
				weap->setSliced(true);
			}
			// End the session on cancel
			tangibleObject->notifyObservers(ObserverEventType::SLICED, player, 0);
			endSlicing();
		}
		return;
	}

	if (progress == 0) {
		switch(menuID) {
		case 0: {
			if (hasPrecisionLaserKnife()) {
				if (firstCable != 0)
					handleSliceFailed(); // Handle failed slice attempt
				else
					cableBlue = true;
			} else
				player->sendSystemMessage("@slicing/slicing:no_knife");
			break;
		}
		case 1: {
			if (hasPrecisionLaserKnife()) {
				if (firstCable != 1)
					handleSliceFailed(); // Handle failed slice attempt
				else
					cableRed = true;
			} else
				player->sendSystemMessage("@slicing/slicing:no_knife");
			break;
		}
		case 2:
			handleUseClamp(); // Handle Use of Molecular Clamp
			break;

		case 3: {
			handleUseFlowAnalyzer(); // Handle Use of Flow Analyzer
			break;
		}
		default:
			cancelSession();
			break;
		}
	} else {
		if (hasPrecisionLaserKnife()) {
			if (firstCable != menuID)
				handleSlice(suiBox); // Handle Successful Slice
			else
				handleSliceFailed(); // Handle failed slice attempt //bugfix 820
			return;
		} else
			player->sendSystemMessage("@slicing/slicing:no_knife");
	}

	generateSliceMenu(suiBox);
}

void SlicingSessionImplementation::endSlicing() {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (player == nullptr || tangibleObject == nullptr) {
		cancelSession();
		return;
	}

	if (tangibleObject->isMissionTerminal())
		player->addCooldown("slicing.terminal", (2 * (60 * 1000))); // 2min Cooldown

	cancelSession();

}

int SlicingSessionImplementation::getSlicingSkill(CreatureObject* slicer) {

	String skill0 = "combat_smuggler_novice";
	String skill1 = "combat_smuggler_slicing_01";
	String skill2 = "combat_smuggler_slicing_02";
	String skill3 = "combat_smuggler_slicing_03";
	String skill4 = "combat_smuggler_slicing_04";
	String skill5 = "combat_smuggler_master";

	if (slicer->hasSkill(skill5))
		return 5;
	else if (slicer->hasSkill(skill4))
		return 4;
	else if (slicer->hasSkill(skill3))
		return 3;
	else if (slicer->hasSkill(skill2))
		return 2;
	else if (slicer->hasSkill(skill1))
		return 1;
	else if (slicer->hasSkill(skill0))
		return 0;

	return -1;

}

bool SlicingSessionImplementation::hasPrecisionLaserKnife(bool removeItem) {
	ManagedReference<CreatureObject*> player = this->player.get();

	if (player == nullptr)
		return 0;

	ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

	if (inventory == nullptr)
		return false;

	Locker inventoryLocker(inventory);

	for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
		ManagedReference<SceneObject*> sceno = inventory->getContainerObject(i);

		uint32 objType = sceno->getGameObjectType();

		if (objType == SceneObjectType::LASERKNIFE) {
			PrecisionLaserKnife* knife = sceno.castTo<PrecisionLaserKnife*>();

			if (knife != nullptr) {
				if (removeItem) {
					Locker locker(knife);
					knife->useCharge(player);
				}
				return 1;
			}
		}
	}

	return 0;
}

bool SlicingSessionImplementation::hasWeaponUpgradeKit() {
	ManagedReference<CreatureObject*> player = this->player.get();

	if (player == nullptr)
		return false;

	ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

	if (inventory == nullptr)
		return false;

	for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
		ManagedReference<SceneObject*> sceno = inventory->getContainerObject(i);

		uint32 objType = sceno->getGameObjectType();

		if (objType == SceneObjectType::WEAPONUPGRADEKIT) {
			Locker locker(sceno);
			sceno->destroyObjectFromWorld(true);
			sceno->destroyObjectFromDatabase(true);
			return true;
		}
	}

	return false;
}

bool SlicingSessionImplementation::hasArmorUpgradeKit() {
	ManagedReference<CreatureObject*> player = this->player.get();

	if (player == nullptr)
		return false;

	ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

	if (inventory == nullptr)
		return false;

	for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
		ManagedReference<SceneObject*> sceno = inventory->getContainerObject(i);

		uint32 objType = sceno->getGameObjectType();

		if (objType == SceneObjectType::ARMORUPGRADEKIT) {
			Locker locker(sceno);
			sceno->destroyObjectFromWorld(true);
			sceno->destroyObjectFromDatabase(true);
			return true;
		}
	}

	return false;
}

void SlicingSessionImplementation::useClampFromInventory(SlicingTool* clamp) {
	ManagedReference<CreatureObject*> player = this->player.get();

	if (clamp == nullptr || clamp->getGameObjectType() != SceneObjectType::MOLECULARCLAMP)
		return;

	ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

	Locker locker(clamp);

	//inventory->removeObject(clamp, true);
	clamp->destroyObjectFromWorld(true);
	clamp->destroyObjectFromDatabase(true);
	player->sendSystemMessage("@slicing/slicing:used_clamp");
	usedClamp = true;

	//if (player->hasSuiBox(slicingSuiBox->getBoxID()))
	//	player->closeSuiWindowType(SuiWindowType::SLICING_MENU);
}

void SlicingSessionImplementation::handleUseClamp() {
	ManagedReference<CreatureObject*> player = this->player.get();

	if (player == nullptr)
		return;

	ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

	Locker inventoryLocker(inventory);

	for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
		ManagedReference<SceneObject*> sceno = inventory->getContainerObject(i);

		uint32 objType = sceno->getGameObjectType();

		if (objType == SceneObjectType::MOLECULARCLAMP) {
			Locker locker(sceno);
			sceno->destroyObjectFromWorld(true);
			sceno->destroyObjectFromDatabase(true);

			player->sendSystemMessage("@slicing/slicing:used_clamp");
			usedClamp = true;
			return;
		}
	}

	player->sendSystemMessage("@slicing/slicing:no_clamp");
}

void SlicingSessionImplementation::handleUseFlowAnalyzer() {
	ManagedReference<CreatureObject*> player = this->player.get();

	if (player == nullptr)
		return;

	ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

	Locker inventoryLocker(inventory);

	for (int i = 0; i < inventory->getContainerObjectsSize(); ++i) {
		ManagedReference<SceneObject*> sceno = inventory->getContainerObject(i);

		uint32 objType = sceno->getGameObjectType();

		if (objType == SceneObjectType::FLOWANALYZER) {
			SlicingTool* node = cast<SlicingTool*>(sceno.get());

			if (node == nullptr)
				continue;

			nodeCable = node->calculateSuccessRate();

			if (nodeCable) // PASSED
				nodeCable = firstCable;
			else if (nodeCable == firstCable) { // Failed but the cables are Correct
				if (firstCable)
					nodeCable = 0; // Failed - Make the Cable incorrect
			}

			Locker locker(sceno);
			sceno->destroyObjectFromWorld(true);
			sceno->destroyObjectFromDatabase(true);

			player->sendSystemMessage("@slicing/slicing:used_node");
			usedNode = true;
			return;
		}
	}

	player->sendSystemMessage("@slicing/slicing:no_node");
}

void SlicingSessionImplementation::handleSlice(SuiListBox* suiBox) {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (player == nullptr || tangibleObject == nullptr)
		return;

	Locker locker(player);
	Locker clocker(tangibleObject, player);

	PlayerManager* playerManager = player->getZoneServer()->getPlayerManager();

	if (tangibleObject->isContainerObject() || tangibleObject->getGameObjectType() == SceneObjectType::PLAYERLOOTCRATE) {
		// Add the examine message to the success window
		suiBox->removeAllMenuItems();
		suiBox->setCancelButton(false,"@cancel");

		StringBuffer prompt;
		prompt << "@slicing/slicing:";
		prompt << getPrefix(tangibleObject) + "examine";
		suiBox->setPromptText(prompt.toString());

		player->getPlayerObject()->addSuiBox(suiBox);
		player->sendMessage(suiBox->generateMessage());
		
		handleContainerSlice();
		playerManager->awardExperience(player, "slicing", 250, true); // Container Slice XP
	} else if (tangibleObject->isMissionTerminal()) {
		// Add the examine message to the success window
		suiBox->removeAllMenuItems();
		suiBox->setCancelButton(false,"@cancel");

		StringBuffer prompt;
		prompt << "@slicing/slicing:";
		prompt << getPrefix(tangibleObject) + "examine";
		suiBox->setPromptText(prompt.toString());

		player->getPlayerObject()->addSuiBox(suiBox);
		player->sendMessage(suiBox->generateMessage());
		
		MissionTerminal* term = cast<MissionTerminal*>( tangibleObject.get());
		playerManager->awardExperience(player, "slicing", 100, true); // Terminal Slice XP
		term->addSlicer(player);
		player->sendSystemMessage("@slicing/slicing:terminal_success");
	} else if (tangibleObject->isWeaponObject()) {
		// For weapons, DO NOT show the original success window
		// Instead, close the original window by removing it from the player
		player->getPlayerObject()->removeSuiBoxType(SuiWindowType::SLICING_MENU);
		
		// Award XP for the successful slice before showing choice menu
		playerManager->awardExperience(player, "slicing", 250, true); // Weapon Slice XP
		
		// Show choice menu without ending the session
		showWeaponSliceMenu();
		
		// Don't end the session here - let handleWeaponSlice do that after the player selects the slice type
		return;
	} else if (tangibleObject->isArmorObject()) {
		// Add the examine message to the success window
		suiBox->removeAllMenuItems();
		suiBox->setCancelButton(false,"@cancel");

		StringBuffer prompt;
		prompt << "@slicing/slicing:";
		prompt << getPrefix(tangibleObject) + "examine";
		suiBox->setPromptText(prompt.toString());

		player->getPlayerObject()->addSuiBox(suiBox);
		player->sendMessage(suiBox->generateMessage());
		
		handleArmorSlice();
		playerManager->awardExperience(player, "slicing", 250, true); // Armor Slice XP
	} else if (isBaseSlice()){
		// Add the examine message to the success window
		suiBox->removeAllMenuItems();
		suiBox->setCancelButton(false,"@cancel");

		StringBuffer prompt;
		prompt << "@slicing/slicing:";
		prompt << getPrefix(tangibleObject) + "examine";
		suiBox->setPromptText(prompt.toString());

		player->getPlayerObject()->addSuiBox(suiBox);
		player->sendMessage(suiBox->generateMessage());
		
		playerManager->awardExperience(player,"slicing", 1000, true); // Base slicing

		Zone* zone = player->getZone();

		if (zone != nullptr){
			GCWManager* gcwMan = zone->getGCWManager();

			if (gcwMan != nullptr){
				SecuritySliceTask* task = new SecuritySliceTask(gcwMan, tangibleObject.get(), player);
				task->execute();
			}
		}
	}

	tangibleObject->notifyObservers(ObserverEventType::SLICED, player, 1);

	endSlicing();
}

void SlicingSessionImplementation::showWeaponSliceMenu() {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (player == nullptr || tangibleObject == nullptr || !tangibleObject->isWeaponObject())
		return;

	// Create SUI Menu for weapon slice choice
	ManagedReference<SuiListBox*> choiceBox = new SuiListBox(player, SuiWindowType::SLICING_MENU, 2);
	choiceBox->setCallback(new SlicingSessionSuiCallback(player->getZoneServer()));
	choiceBox->setPromptTitle("Weapon Slice Options");
	choiceBox->setPromptText("Select the type of modification you wish to make to this weapon:");
	choiceBox->setUsingObject(tangibleObject);
	choiceBox->setCancelButton(true, "@cancel");
	
	choiceBox->addMenuItem("Damage Modification", 0); // Damage modification
	choiceBox->addMenuItem("Speed Modification", 1);  // Speed modification
	
	player->getPlayerObject()->addSuiBox(choiceBox);
	player->sendMessage(choiceBox->generateMessage());
}

void SlicingSessionImplementation::handleWeaponSlice(int sliceType) {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (player == nullptr || tangibleObject == nullptr || !tangibleObject->isWeaponObject())
		return;

	int sliceSkill = getSlicingSkill(player);
	uint8 min = 0;
	uint8 max = 0;

	switch (sliceSkill) {
	case 5:
		min += 10;
		max += 5;
	case 4:
		min += 5;
		max += 5;
	case 3:
	case 2:
		min += 10;
		max += 25;
		break;
	default:
		return;
	}

	uint8 percentage = System::random(max - min) + min;

	switch(sliceType) {
	case 0:
		handleSliceDamage(percentage);
		break;
	case 1:
		handleSliceSpeed(percentage);
		break;
	default:
		// Default to damage if somehow an invalid option is passed
		handleSliceDamage(percentage);
		break;
	}
	
	// End the slicing session
	tangibleObject->notifyObservers(ObserverEventType::SLICED, player, 1);
	endSlicing();
}

void SlicingSessionImplementation::detachPowerUp(CreatureObject* player, WeaponObject* weap) {
	ManagedReference<PowerupObject*> pup = weap->removePowerup();
	if (pup == nullptr)
		return;

	Locker locker(pup);

	pup->destroyObjectFromWorld(true);
	pup->destroyObjectFromDatabase(true);

	locker.release();

	StringIdChatParameter message("powerup", "prose_remove_powerup"); //You detach your powerup from %TT.
	message.setTT(weap->getDisplayedName());
	player->sendSystemMessage(message);

}

void SlicingSessionImplementation::handleSliceDamage(uint8 percent) {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (tangibleObject == nullptr || player == nullptr || !tangibleObject->isWeaponObject())
		return;

	WeaponObject* weap = cast<WeaponObject*>(tangibleObject.get());

	Locker locker(weap);

	if (weap->hasPowerup())
		this->detachPowerUp(player, weap);

	weap->setDamageSlice(percent / 100.f);
	weap->setSliced(true);

	StringIdChatParameter params;
	params.setDI(percent);
	params.setStringId("@slicing/slicing:dam_mod");

	player->sendSystemMessage(params);

}

void SlicingSessionImplementation::handleSliceSpeed(uint8 percent) {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (tangibleObject == nullptr || player == nullptr || !tangibleObject->isWeaponObject())
		return;

	WeaponObject* weap = cast<WeaponObject*>(tangibleObject.get());

	Locker locker(weap);

	if (weap->hasPowerup())
		this->detachPowerUp(player, weap);

	weap->setSpeedSlice(percent / 100.f);
	weap->setSliced(true);

	StringIdChatParameter params;
	params.setDI(percent);
	params.setStringId("@slicing/slicing:spd_mod");

	player->sendSystemMessage(params);
}

void SlicingSessionImplementation::handleArmorSlice() {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (tangibleObject == nullptr || player == nullptr)
		return;

	uint8 sliceType = System::random(1);
	int sliceSkill = getSlicingSkill(player);
	uint8 min = 0;
	uint8 max = 0;

	switch (sliceSkill) {
	case 5:
		min += (sliceType == 0) ? 16 : 15;
		max += 5;
	case 4:
		min += (sliceType == 0) ? 0 : 10;
		max += 10;
	case 3:
		min += 5;
		max += (sliceType == 0) ? 20 : 30;
		break;
	default:
		return;
	}

	uint8 percent = System::random(max - min) + min;

	switch (sliceType) {
	case 0:
		handleSliceEffectiveness(percent);
		break;
	case 1:
		handleSliceEncumbrance(percent);
		break;
	}
}

void SlicingSessionImplementation::handleSliceEncumbrance(uint8 percent) {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (tangibleObject == nullptr || player == nullptr || !tangibleObject->isArmorObject())
		return;

	ArmorObject* armor = cast<ArmorObject*>(tangibleObject.get());

	Locker locker(armor);

	if ((armor->getMaxCondition() - armor->getConditionDamage()) <= 0) {
		player->sendSystemMessage("The armor is too badly damaged to slice for condition");
		return;
	}
	int conDmg = armor->getConditionDamage();
	armor->setMaxCondition(armor->getMaxCondition() + (armor->getMaxCondition() - (percent / 100.f)), true);
	armor->setConditionDamage(conDmg, true);

	armor->setSliced(true);

	StringIdChatParameter params;
	params.setDI(percent);
	params.setStringId("@slicing/slicing:enc_mod");

	player->sendSystemMessage(params);
}

void SlicingSessionImplementation::handleSliceEffectiveness(uint8 percent) {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (tangibleObject == nullptr || player == nullptr || !tangibleObject->isArmorObject())
		return;

	ArmorObject* armor = cast<ArmorObject*>(tangibleObject.get());

	Locker locker(armor);

	armor->setEffectivenessSlice(percent / 100.f);
	armor->setSliced(true);

	StringIdChatParameter params;
	params.setDI(percent);
	params.setStringId("@slicing/slicing:eff_mod");

	player->sendSystemMessage(params);
}

void SlicingSessionImplementation::handleContainerSlice() {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (tangibleObject == nullptr || player == nullptr)
		return;

	ManagedReference<SceneObject*> inventory = player->getSlottedObject("inventory");

	if (inventory == nullptr)
		return;

	Locker inventoryLocker(inventory);

	LootManager* lootManager = player->getZoneServer()->getLootManager();

	if (tangibleObject->getGameObjectType() == SceneObjectType::PLAYERLOOTCRATE) {
		Reference<SceneObject*> containerSceno = player->getZoneServer()->createObject(STRING_HASHCODE("object/tangible/container/loot/loot_crate.iff"), 1);

		if (containerSceno == nullptr)
			return;

		Locker clocker(containerSceno, player);

		Container* container = dynamic_cast<Container*>(containerSceno.get());

		if (container == nullptr) {
			containerSceno->destroyObjectFromDatabase(true);
			return;
		}

		TransactionLog trx(TrxCode::SLICECONTAINER, player, container);

		if (System::random(10) != 4) {
			lootManager->createLoot(trx, container, "looted_container");
		}

		inventory->transferObject(container, -1);
		container->sendTo(player, true);

		trx.commit();

		if (inventory->hasObjectInContainer(tangibleObject->getObjectID())) {
			//inventory->removeObject(tangibleObject, true);
			tangibleObject->destroyObjectFromWorld(true);
		}

		tangibleObject->destroyObjectFromDatabase(true);

	} else if (tangibleObject->isContainerObject()) {

		Container* container = dynamic_cast<Container*>(tangibleObject.get());
        if (container == nullptr)
			return;

		container->setSliced(true);
		container->setLockedStatus(false);

		if(!container->isRelocking())
		{
			relockEvent = new RelockLootContainerEvent(container);
			relockEvent->schedule(container->getLockTime());
		}
	} else
		return;

	player->sendSystemMessage("@slicing/slicing:container_success");
}

void SlicingSessionImplementation::handleSliceFailed() {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();

	if (tangibleObject == nullptr || player == nullptr)
			return;

	if (tangibleObject->isMissionTerminal())
		player->sendSystemMessage("@slicing/slicing:terminal_fail");
	else if (tangibleObject->isWeaponObject())
		player->sendSystemMessage("@slicing/slicing:fail_weapon");
	else if (tangibleObject->isArmorObject())
		player->sendSystemMessage("@slicing/slicing:fail_armor");
	else if (tangibleObject->isContainerObject() || tangibleObject->getGameObjectType() == SceneObjectType::PLAYERLOOTCRATE)
		player->sendSystemMessage("@slicing/slicing:container_fail");
	else if (isBaseSlice())
		player->sendSystemMessage("@slicing/slicing:hq_security_fail"); // Unable to sucessfully slice the terminal, you realize that the only away
	else if (isKeypadSlice())
		player->sendSystemMessage("@slicing/slicing:keypad_fail"); // Unable to successfully slice the keypad, you realize that the only way to reset it is to carefully repair what damage you have done.
	else
		player->sendSystemMessage("Your attempt to slice the object has failed.");

	if (tangibleObject->isContainerObject()) {

		ManagedReference<Container*> container = tangibleObject.castTo<Container*>();
        Locker clocker(container, player);

		if (!container)
			return;

        container->setSliced(true);
		if (!container->isRelocking())
		{
			relockEvent = new RelockLootContainerEvent(container);
			relockEvent->schedule(container->getLockTime()); // This will reactivate the 'broken' lock. (1 Hour)
		}

	} else if (isBaseSlice()){

		Zone* zone = player->getZone();
		if (zone != nullptr) {
			GCWManager* gcwMan = zone->getGCWManager();
			if(gcwMan != nullptr)
				gcwMan->failSecuritySlice(tangibleObject.get());

		}
	} else if (!tangibleObject->isMissionTerminal() && !isKeypadSlice()) {
		tangibleObject->setSliced(true);
	}

	tangibleObject->notifyObservers(ObserverEventType::SLICED, player, 0);
	endSlicing();

}
int SlicingSessionImplementation::cancelSession() {
	ManagedReference<CreatureObject*> player = this->player.get();
	ManagedReference<TangibleObject*> tangibleObject = this->tangibleObject.get();
	if (player != nullptr) {
		player->dropActiveSession(SessionFacadeType::SLICING);
		player->getPlayerObject()->removeSuiBoxType(SuiWindowType::SLICING_MENU);
	}
	if (tangibleObject != nullptr)
		tangibleObject->dropActiveSession(SessionFacadeType::SLICING);
	clearSession();
	return 0;
}
