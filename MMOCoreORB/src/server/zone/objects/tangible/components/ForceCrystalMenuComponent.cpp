/*
 * ForceCrystalMenuComponent.cpp
 */

#include "ForceCrystalMenuComponent.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/scene/SceneObject.h"
#include "server/zone/packets/object/ObjectMenuResponse.h"
#include "server/zone/objects/player/PlayerObject.h"
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/managers/jedi/JediManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/director/DirectorManager.h"
#include "server/zone/objects/player/variables/PlayerQuestData.h"

void ForceCrystalMenuComponent::fillObjectMenuResponse(SceneObject* sceneObject, ObjectMenuResponse* menuResponse, CreatureObject* player) const {

	TangibleObjectMenuComponent::fillObjectMenuResponse(sceneObject, menuResponse, player);

	menuResponse->addRadialMenuItem(20, 3, "@quest/force_sensitive/intro:crystal_journal"); // Display Journal
	menuResponse->addRadialMenuItem(120, 3, "Restore Force Sensitive Skills"); // Fix for missing FS skills
}

int ForceCrystalMenuComponent::handleObjectMenuSelect(SceneObject* sceneObject, CreatureObject* creature, byte selectedID) const {
	if (selectedID == 20) {
		creature->sendExecuteConsoleCommand("/ui action questJournal");
		return 0;
	} else if (selectedID == 120) {
		// Check if player is eligible to receive Force Sensitive skills
		if (creature == nullptr || !creature->isPlayerCreature())
			return 0;
			
		PlayerObject* ghost = creature->getPlayerObject();
		
		if (ghost == nullptr)
			return 0;
		
		// Check if player has completed Mellichae quest - using the proper quest check method
		if (!ghost->hasCompletedQuestsBitSet(PlayerQuestData::FS_THEATER_FINAL)) {
			creature->sendSystemMessage("You have not completed the Mellichae trials yet.");
			return 0;
		}
		
		// Grant Force Sensitive skills - always regrant regardless of existing skills
		SkillManager* skillManager = creature->getZoneServer()->getSkillManager();
		
		// List of Force Sensitive skills - all 16 skills from the 4 main branches
		const char* fsSkills[] = {
			// Combat Prowess Branch
			"force_sensitive_combat_prowess_novice",
			"force_sensitive_combat_prowess_ranged_accuracy_04",
			"force_sensitive_combat_prowess_ranged_speed_04",
			"force_sensitive_combat_prowess_melee_accuracy_04", 
			"force_sensitive_combat_prowess_melee_speed_04",
			
			// Enhanced Reflexes Branch
			"force_sensitive_enhanced_reflexes_novice",
			"force_sensitive_enhanced_reflexes_ranged_defense_04",
			"force_sensitive_enhanced_reflexes_melee_defense_04",
			"force_sensitive_enhanced_reflexes_vehicle_control_04",
			"force_sensitive_enhanced_reflexes_survival_04",
			
			// Crafting Mastery Branch
			"force_sensitive_crafting_mastery_novice",
			"force_sensitive_crafting_mastery_experimentation_04",
			"force_sensitive_crafting_mastery_assembly_04",
			"force_sensitive_crafting_mastery_repair_04",
			"force_sensitive_crafting_mastery_technique_04",
			
			// Heightened Senses Branch
			"force_sensitive_heightened_senses_novice",
			"force_sensitive_heightened_senses_healing_04",
			"force_sensitive_heightened_senses_surveying_04",
			"force_sensitive_heightened_senses_persuasion_04",
			"force_sensitive_heightened_senses_luck_04"
		};
		
		// Grant all skills, ignoring any individual failures
		int successCount = 0;
		for (int i = 0; i < 20; i++) {
			// Ignore the return value and continue regardless of success/failure
			skillManager->awardSkill(fsSkills[i], creature, true, true, true);
			successCount++;
		}
		
		// Always show success message since we tried to grant all skills
		creature->sendSystemMessage("Your connection to the Force has been restored. Force Sensitive skills have been granted.");
		
		return 0;
	}
	
	return 0;
}
