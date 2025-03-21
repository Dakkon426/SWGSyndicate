/*
 				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/creature/ai/AiAgent.h"
#include "templates/params/creature/CreatureState.h"
#include "templates/params/creature/CreatureFlag.h"

#include "server/zone/managers/object/ObjectManager.h"
#include "server/zone/managers/objectcontroller/ObjectController.h"
#include "server/zone/managers/skill/SkillManager.h"
#include "server/zone/managers/player/PlayerManager.h"
#include "server/zone/managers/combat/CombatManager.h"
#include "server/zone/managers/mission/MissionManager.h"
#include "server/zone/managers/creature/PetManager.h"
#include "server/zone/managers/reaction/ReactionManager.h"
#include "server/zone/ZoneClientSession.h"
#include "server/zone/packets/creature/CreatureObjectMessage1.h"
#include "server/zone/packets/creature/CreatureObjectMessage3.h"
#include "server/zone/packets/creature/CreatureObjectMessage4.h"
#include "server/zone/packets/creature/CreatureObjectMessage6.h"
#include "server/zone/packets/creature/CreatureObjectDeltaMessage1.h"
#include "server/zone/packets/creature/CreatureObjectDeltaMessage3.h"
#include "server/zone/packets/creature/CreatureObjectDeltaMessage4.h"
#include "server/zone/packets/creature/CreatureObjectDeltaMessage6.h"
#include "server/zone/packets/chat/ChatSystemMessage.h"
#include "server/zone/packets/object/CombatSpam.h"
#include "server/zone/packets/object/PostureMessage.h"
#include "server/zone/packets/object/SitOnObject.h"
#include "server/zone/packets/object/CommandQueueRemove.h"
#include "server/zone/packets/object/CommandQueueAdd.h"
#include "server/zone/packets/object/CombatAction.h"
#include "server/zone/packets/object/WeaponRanges.h"
#include "server/zone/packets/player/PlayMusicMessage.h"
#include "server/zone/packets/player/NewbieTutorialRequest.h"
#include "server/zone/packets/ui/NewbieTutorialEnableHudElement.h"
#include "server/zone/packets/ui/OpenHolocronToPageMessage.h"
#include "server/zone/packets/object/Animation.h"
#include "templates/params/creature/CreatureAttribute.h"
#include "templates/params/creature/CreaturePosture.h"
#include "server/zone/objects/creature/commands/effect/CommandEffect.h"
#include "server/zone/objects/creature/events/CommandQueueActionEvent.h"
#include "server/zone/Zone.h"
#include "server/zone/ZoneServer.h"
#include "server/chat/ChatManager.h"
#include "server/chat/StringIdChatParameter.h"
#include "server/zone/objects/creature/variables/CommandQueueAction.h"
#include "server/zone/objects/creature/commands/QueueCommand.h"
#include "server/zone/objects/group/GroupObject.h"
#include "server/zone/objects/player/FactionStatus.h"
#include "server/zone/objects/area/ActiveArea.h"
#include "server/zone/objects/mission/MissionObject.h"
#include "server/zone/objects/area/CampSiteActiveArea.h"
#include "server/zone/objects/tangible/weapon/WeaponObject.h"
#include "server/zone/objects/guild/GuildObject.h"
#include "server/zone/objects/creature/events/DizzyFallDownEvent.h"
#include "server/zone/packets/ui/ExecuteConsoleCommand.h"
#include "server/zone/objects/creature/buffs/StateBuff.h"
#include "server/zone/objects/creature/buffs/PrivateSkillMultiplierBuff.h"
#include "server/zone/objects/creature/buffs/PlayerVehicleBuff.h"
#include "server/zone/objects/intangible/PetControlDevice.h"
#include "server/zone/managers/planet/PlanetManager.h"
#include "terrain/manager/TerrainManager.h"

#include "templates/creature/SharedCreatureObjectTemplate.h"

#include "variables/Skill.h"
#include "server/zone/objects/player/sessions/EntertainingSession.h"

#include "server/zone/packets/ui/ClientMfdStatusUpdateMessage.h"

#include "server/zone/packets/zone/unkByteFlag.h"
#include "server/zone/packets/zone/CmdStartScene.h"
#include "server/zone/packets/zone/ParametersMessage.h"

#include "server/zone/managers/guild/GuildManager.h"
#include "system/lang/ref/Reference.h"
#include "server/zone/objects/player/events/LogoutTask.h"
#include "server/zone/objects/player/events/StoreSpawnedChildrenTask.h"

#include "server/zone/objects/tangible/threat/ThreatMap.h"

#include "engine/core/TaskManager.h"
#include "server/zone/objects/creature/credits/CreditObject.h"

#include "templates/customization/AssetCustomizationManagerTemplate.h"
#include "templates/customization/BasicRangedIntCustomizationVariable.h"
#include "templates/params/PaletteColorCustomizationVariable.h"
#include "templates/appearance/PaletteTemplate.h"
#include "server/zone/managers/auction/AuctionSearchTask.h"

float CreatureObjectImplementation::DEFAULTRUNSPEED = 5.376f;

void CreatureObjectImplementation::initializeTransientMembers() {
	TangibleObjectImplementation::initializeTransientMembers();

	groupInviterID = 0;
	groupInviteCounter = 0;
	currentWeather = 0;
	currentWind = 0;

	if (creditObject == nullptr) {
		creditObject = getCreditObject();
	}

	creditObject->setOwner(asCreatureObject());

	lastActionCounter = 0x40000000;

	setContainerOwnerID(getObjectID());
	setMood(moodID);

	setLoggingName("CreatureObject");
}

void CreatureObjectImplementation::initializeMembers() {
	linkedCreature = nullptr;
	controlDevice = nullptr;

	bankCredits = 0;
	cashCredits = 0;

	pvpStatusBitmask = 0;

	posture = 0;
	factionRank = 0;
	faction = 0;

	hueValue = -1;

	stateBitmask = 0;
	terrainNegotiation = 0.0f;

	listenToID = 0;
	watchToID = 0;

	weapon = nullptr;
	guild = nullptr;
	group = nullptr;
	auctionSearch = nullptr;
	groupInviterID = 0;
	groupInviteCounter = 0;
	targetID = 0;
	moodID = 0;
	performanceCounter = 0;
	instrumentID = 0;

	optionsBitmask = 0x80;

	moodString = "neutral";

	height = 1;

	shockWounds = 0;

	accelerationMultiplierBase = 1.f;
	accelerationMultiplierMod = 1.f;
	speedMultiplierBase = 1.f;
	speedMultiplierMod = 1.f;
	currentSpeed = 0.f;
	walkSpeed = 0.f;
	runSpeed = 0.f;
	turnScale = 1.f;

	cooldownTimerMap = new CooldownTimerMap();
	commandQueue = new CommandQueueActionVector();
	immediateQueue = new CommandQueueActionVector();

	closeobjects = new CloseObjectsVector();
	closeobjects->setNoDuplicateInsertPlan();

	healthWoundHeal = 0;
	actionWoundHeal = 0;
	mindWoundHeal = 0;

	setContainerInheritPermissionsFromParent(false);
	setContainerDefaultDenyPermission(ContainerPermissions::MOVECONTAINER);
	setContainerDenyPermission("owner", ContainerPermissions::MOVECONTAINER);
}

void CreatureObjectImplementation::loadTemplateData(
		SharedObjectTemplate* templateData) {
	TangibleObjectImplementation::loadTemplateData(templateData);

	const SharedCreatureObjectTemplate* creoData =
			dynamic_cast<SharedCreatureObjectTemplate*> (templateData);

	if (creoData == nullptr)
		return;

	slopeModPercent = creoData->getSlopeModPercent();
	slopeModAngle = creoData->getSlopeModAngle();
	swimHeight = creoData->getSwimHeight();

	if (creoData->getMaxScale() == 0)
		height = creoData->getMinScale();
	else
		height = creoData->getMaxScale();

	if (height <= SMALL_NUM)
		height = 1.0;

	debug() << "loaded with height: " << height;

	level = creoData->getLevel();

	encumbrances.removeAll();

	for (int i = 0; i < 3; ++i) {
		encumbrances.add(0);
	}

	const auto& base = creoData->getBaseHAM();

	baseHAM.removeAll();

	for (int i = 0; i < base.size(); ++i)
		baseHAM.add(base.get(i));

	wounds.removeAll();
	for (int i = 0; i < 9; ++i) {
		wounds.add(0);
	}

	hamList.removeAll();
	for (int i = 0; i < 9; ++i) {
		hamList.add(baseHAM.get(i));
	}

	maxHamList.removeAll();
	for (int i = 0; i < 9; ++i) {
		maxHamList.add(baseHAM.get(i));
	}

	frozen = 0;

	const auto& accel = creoData->getAcceleration();

	if (accel.size() > 0) {
		runAcceleration = accel.get(0);
		walkAcceleration = accel.get(1);
	} else {
		runAcceleration = 0;
		walkAcceleration = 0;
	}

	const auto& speedTempl = creoData->getSpeed();

	if (speedTempl.size() > 0) {
		runSpeed = speedTempl.get(0);
		walkSpeed = speedTempl.get(1);
	} else {
		runSpeed = 0;
		walkSpeed = 0;
	}

	auto zoneServer = ServerCore::getZoneServer();

	if (zoneServer && isPlayerCreature()) {
		setLoggerCallback([creo = asCreatureObject(), manager = WeakReference<PlayerManager*>(zoneServer->getPlayerManager())]
				(Logger::LogLevel level, const char* msg) -> int {
			auto playerManager = manager.get();

			if (playerManager != nullptr) {
				playerManager->writePlayerLog(creo, msg, level);
			}

			return Logger::SUCCESS;
		});
	}
}

void CreatureObjectImplementation::finalize() {

}

void CreatureObjectImplementation::sendToOwner(bool doClose) {
	auto owner = this->owner.get();

	if (owner == nullptr)
		return;

	setMovementCounter(0);

	owner->balancePacketCheckupTime();

	BaseMessage* byteFlag = new unkByteFlag();
	owner->sendMessage(byteFlag);

	BaseMessage* startScene = new CmdStartScene(asCreatureObject());
	owner->sendMessage(startScene);

	BaseMessage* parameters = new ParametersMessage();
	owner->sendMessage(parameters);

	ManagedReference<GuildManager*> guildManager =
			server->getZoneServer()->getGuildManager();
	guildManager->sendBaselinesTo(asCreatureObject());

	ManagedReference<SceneObject*> grandParent = getRootParent();

	if (grandParent != nullptr) {
		grandParent->sendTo(asCreatureObject(), true);
	} else
		sendTo(asCreatureObject(), doClose);

	CloseObjectsVector* vec = getCloseObjects();

	fatal(vec != nullptr) << "close objects vector in creo::sendToOwner null";

	SortedVector<QuadTreeEntry*> closeObjects;
	vec->safeCopyTo(closeObjects);

	for (int i = 0; i < closeObjects.size(); ++i) {
		SceneObject* obj = static_cast<SceneObject*> (closeObjects.get(i));

		if (obj != asCreatureObject()) {
			if (obj != grandParent) {
				notifyInsert(obj);
				//obj->sendTo(asCreatureObject(), true);
			}

			if (obj->isPlayerCreature()) { //we need to destroy object to reset movement counter on near clients
				obj->notifyDissapear(asCreatureObject());
			}

			//obj->notifyInsert(asCreatureObject());
			sendTo(obj, true, false);
		}

	}

	if (group != nullptr)
		group->sendTo(asCreatureObject(), true);

	owner->resetPacketCheckupTime();
}

void CreatureObjectImplementation::sendBaselinesTo(SceneObject* player) {
	CreatureObject* thisPointer = asCreatureObject();
	Zone* zone = getZoneUnsafe();

	if (zone == nullptr)
		return;

	if (player == thisPointer) {
		debug() << "sending baselines to myself";

		CreatureObjectMessage1* msg = new CreatureObjectMessage1(thisPointer);
		player->sendMessage(msg);
	}

	CreatureObjectMessage3* msg3 = new CreatureObjectMessage3(thisPointer);
	player->sendMessage(msg3);

	if (player == thisPointer) {
		CreatureObjectMessage4* msg4 = new CreatureObjectMessage4(thisPointer);
		player->sendMessage(msg4);
	}

	CreatureObjectMessage6* msg6 = new CreatureObjectMessage6(thisPointer);
	player->sendMessage(msg6);

	if (!player->isPlayerCreature())
		return;

	CreatureObject* playerCreature = static_cast<CreatureObject*> (player);

	sendPvpStatusTo(playerCreature);
}

void CreatureObjectImplementation::sendSlottedObjectsTo(SceneObject* player) {
	SortedVector<SceneObject*> objects(getSlottedObjectsSize(),
			getSlottedObjectsSize());
	objects.setNoDuplicateInsertPlan();

	try {
		for (int i = 0; i < getSlottedObjectsSize(); ++i) {
			Reference<SceneObject*> object = getSlottedObject(i);

			int arrangementSize = object->getArrangementDescriptorSize();

			bool sendWithoutContents = false;

			if (arrangementSize > 0) {
				const Vector<String>* descriptors = object->getArrangementDescriptor(0);

				if (descriptors->size() > 0) {
					const String& childArrangement = descriptors->get(0);

					if (player != asCreatureObject() && ((childArrangement == "bank")
							|| (childArrangement == "inventory") || (childArrangement
									== "datapad") || (childArrangement == "mission_bag"))) {

						sendWithoutContents = true;
					}
				}
			}

			if (objects.put(object) != -1) {
				if (sendWithoutContents)
					object->sendWithoutContainerObjectsTo(player);
				else
					object->sendTo(player, true);
			}
		}
	} catch (Exception& e) {
		error(e.getMessage());
		e.printStackTrace();
	}
}

void CreatureObjectImplementation::sendSystemMessage(const String& message) {
	if (!isPlayerCreature())
		return;

	UnicodeString msg(message);
	sendSystemMessage(msg);
}

void CreatureObjectImplementation::playMusicMessage(const String& file) {
	PlayMusicMessage* message = new PlayMusicMessage(file);
	sendMessage(message);
}

void CreatureObjectImplementation::sendOpenHolocronToPageMessage() {
	OpenHolocronToPageMessage* message = new OpenHolocronToPageMessage();
	sendMessage(message);
}

void CreatureObjectImplementation::sendNewbieTutorialRequest(
		const String& request) {
	debug() << "sending newbie request " << request;

	NewbieTutorialRequest* message = new NewbieTutorialRequest(request);
	sendMessage(message);
}

void CreatureObjectImplementation::sendNewbieTutorialEnableHudElement(
		const String& ui, bool enable, float blinkCount) {
	NewbieTutorialEnableHudElement* message =
			new NewbieTutorialEnableHudElement(ui, enable, blinkCount);
	sendMessage(message);
}

void CreatureObjectImplementation::sendSystemMessage(
		StringIdChatParameter& message) {
	if (!isPlayerCreature())
		return;

	ChatSystemMessage* msg = new ChatSystemMessage(message);
	sendMessage(msg);
}

void CreatureObjectImplementation::sendSystemMessage(UnicodeString& message) {
	if (!isPlayerCreature())
		return;

	ChatSystemMessage* smsg = new ChatSystemMessage(message);
	sendMessage(smsg);
}

void CreatureObjectImplementation::clearQueueAction(uint32 actioncntr,
		float timer, uint32 tab1, uint32 tab2) {
	if (actioncntr == 0 || !isPlayerCreature())
		return;

	BaseMessage* queuemsg = new CommandQueueRemove(asCreatureObject(), actioncntr, timer,
			tab1, tab2);
	sendMessage(queuemsg);
}

void CreatureObjectImplementation::clearQueueActions(bool combatOnly) {
	for (int i = commandQueue->size() - 1; i >= 0; i--) {
		CommandQueueAction* command = commandQueue->get(i);

		if (command == nullptr)
			continue;

		if (combatOnly) {
			ZoneServer* zoneServer = getZoneServer();
			if (zoneServer == nullptr)
				continue;

			ObjectController* objectController = zoneServer->getObjectController();
			if (objectController == nullptr)
				continue;

			const QueueCommand* qc = objectController->getQueueCommand(command->getCommand());
			if (qc == nullptr)
				continue;

			if (!qc->isCombatCommand())
				continue;
		}

		if (command->getActionCounter() != 0)
			clearQueueAction(command->getActionCounter());

		commandQueue->remove(i);
	}
}

void CreatureObjectImplementation::setWeapon(WeaponObject* weao,
		bool notifyClient) {
	if (weao == nullptr)
		weao = TangibleObjectImplementation::getSlottedObject("default_weapon").castTo<WeaponObject*>();

	weapon = weao;

	if (notifyClient) {
		CreatureObjectDeltaMessage6* msg = new CreatureObjectDeltaMessage6(
				asCreatureObject());
		msg->updateWeapon();
		msg->close();

		broadcastMessage(msg, true);

		WeaponRanges* ranges = new WeaponRanges(asCreatureObject(), getWeapon());
		sendMessage(ranges);
	}
}

void CreatureObjectImplementation::setLevel(int level, bool randomHam) {
	if (this->level == level && level >= 0)
		return;

	TangibleObjectImplementation::setLevel(level);

	CreatureObjectDeltaMessage6* msg = new CreatureObjectDeltaMessage6(asCreatureObject());

	msg->updateLevel(this->level);
	msg->close();

	broadcastMessage(msg, true);

	if (isGrouped()) {
		Locker clocker(group, asCreatureObject());

		group->calcGroupLevel();
	}
}

void CreatureObjectImplementation::setInstrumentID(int instrumentid,
		bool notifyClient) {
	if (instrumentid == instrumentID)
		return;

	instrumentID = instrumentid;

	if (notifyClient) {
		CreatureObjectDeltaMessage6* msg = new CreatureObjectDeltaMessage6(
				asCreatureObject());
		msg->updateInstrumentID(instrumentID);
		msg->close();

		broadcastMessage(msg, true);
	}
}

void CreatureObjectImplementation::setTargetID(uint64 targetID,
		bool notifyClient) {
	CreatureObjectImplementation::targetID = targetID;

	if (notifyClient) {
		CreatureObjectDeltaMessage6* msg = new CreatureObjectDeltaMessage6(
				asCreatureObject());
		msg->updateTarget();
		msg->close();

		broadcastMessage(msg, false);
	}
}

void CreatureObjectImplementation::setHeight(float height, bool notifyClient) {
	if (this->height == height)
		return;

	this->height = height;

	if (!notifyClient)
		return;

	CreatureObjectDeltaMessage3* msg = new CreatureObjectDeltaMessage3(asCreatureObject());
	msg->updateHeight();
	msg->close();

	broadcastMessage(msg, true);
}

void CreatureObjectImplementation::setShockWounds(int newShock,
		bool notifyClient) {
	if (shockWounds == newShock)
		return;

	shockWounds = newShock;

	if (notifyClient) {
		CreatureObjectDeltaMessage3* dcreo3 = new CreatureObjectDeltaMessage3(
				asCreatureObject());
		dcreo3->updateShockWounds();
		dcreo3->close();

		broadcastMessage(dcreo3, true);
	}
}

void CreatureObjectImplementation::addShockWounds(int shockToAdd, bool notifyClient, bool sendSpam) {
	int newShockWounds = shockWounds + shockToAdd;

	if (newShockWounds < 0) {
		newShockWounds = 0;
	} else if (newShockWounds > 1000) {
		newShockWounds = 1000;
	}

	if (sendSpam && shockToAdd > 0 && asCreatureObject()->isPlayerCreature())
		sendStateCombatSpam("cbt_spam", "shock_wound", 1, shockToAdd, false);

	setShockWounds(newShockWounds, notifyClient);
}

void CreatureObjectImplementation::addMountedCombatSlow() {
	if (!isPlayerCreature() || !isRidingMount() || !isInCombat())
		return;

	constexpr uint32 crc = "mounted_combat_slow"_hashCode;

	ManagedReference<CreatureObject*> parent = getParent().get().castTo<CreatureObject*>();

	if (parent == nullptr)
		return;

	if (parent->hasBuff(crc))
		return;

	if (parent->isVehicleObject()) {
		sendSystemMessage("@combat_effects:no_combat_while_driving"); // You cannot attack or react to an attack while driving.
		return;
	}

	if (!parent->isMount())
		return;

	if (parent->hasBuff(STRING_HASHCODE("gallop"))) {
		sendSystemMessage("@combat_effects:no_combat_while_galloping"); // You cannot attack or react to an attack while galloping. Use /gallopStop to stop galloping.
		return;
	}

	ManagedReference<CreatureObject*> creo = asCreatureObject();

	Core::getTaskManager()->executeTask([=] () {
		Locker locker(creo);
		Locker clocker(parent, creo);

		float newSpeed = 1;
		SharedObjectTemplate* templateData = creo->getObjectTemplate();
		SharedCreatureObjectTemplate* playerTemplate = dynamic_cast<SharedCreatureObjectTemplate*> (templateData);

		if (playerTemplate != nullptr) {
			const Vector<FloatParam>& speedTempl = playerTemplate->getSpeed();
			newSpeed = speedTempl.get(0);
		}

		float oldSpeed = 1;
		PetManager* petManager = creo->getZoneServer()->getPetManager();

		if (petManager != nullptr) {
			oldSpeed = petManager->getMountedRunSpeed(parent);
		}

		float magnitude = newSpeed / oldSpeed;

		constexpr auto crc = "mounted_combat_slow"_hashCode;

		StringIdChatParameter startStringId("combat_effects", "mount_slow_for_combat"); // Your mount slows down to prepare for combat.

		ManagedReference<PlayerVehicleBuff*> buff = new PlayerVehicleBuff(parent, crc, 604800, BuffType::OTHER);

		Locker blocker(buff);

		buff->setSpeedMultiplierMod(magnitude);
		buff->setAccelerationMultiplierMod(magnitude);
		buff->setStartMessage(startStringId);

		parent->addBuff(buff);
	}, "AddMountedCombatSlowLambda");
}

void CreatureObjectImplementation::removeMountedCombatSlow(bool showEndMessage) {
	ManagedReference<CreatureObject*> creo = asCreatureObject();
	ManagedReference<CreatureObject*> vehicle = getParent().get().castTo<CreatureObject*>();
	if (vehicle == nullptr)
		return;

	Core::getTaskManager()->executeTask([=] () {
		Locker locker(vehicle);

		constexpr uint32 buffCRC = "mounted_combat_slow"_hashCode;

		bool hasBuff = vehicle->hasBuff(buffCRC);

		if (hasBuff) {
			vehicle->removeBuff(buffCRC);
			if (showEndMessage) {
				//I don't think we want to show this on dismount, or after a gallop
				StringIdChatParameter endStringId("combat_effects", "mount_speed_after_combat"); // Your mount speeds up.
				creo->sendSystemMessage(endStringId);
			}
		}
	}, "RemoveMountedCombatSlowLambda");
}

void CreatureObjectImplementation::setCombatState() {
	//lastCombatAction.update();

	if (!(stateBitmask & CreatureState::COMBAT)) {
		stateBitmask |= CreatureState::COMBAT;

		lastSuccessfulCombatAction.updateToCurrentTime();

		if (stateBitmask & CreatureState::PEACE)
			stateBitmask &= ~CreatureState::PEACE;

		CreatureObjectDeltaMessage3* dcreo3 = new CreatureObjectDeltaMessage3(asCreatureObject());
		dcreo3->updateCreatureBitmask(getOptionsBitmask());
		dcreo3->updateState();
		dcreo3->close();

		broadcastMessage(dcreo3, true);

		if (posture == CreaturePosture::SITTING)
			setPosture(CreaturePosture::UPRIGHT);

		Reference<LogoutTask*> logoutTask = getPendingTask("logout").castTo<LogoutTask*>();

		if (logoutTask != nullptr)
			logoutTask->cancelLogout();

		if (isEntertaining())
			stopEntertaining();

		if (isRidingMount())
			addMountedCombatSlow();

		notifyObservers(ObserverEventType::STARTCOMBAT);
	}
}

void CreatureObjectImplementation::clearCombatState(bool removedefenders) {
	debug("trying to clear CombatState");

	if (stateBitmask & CreatureState::COMBAT) {
		if (stateBitmask & CreatureState::PEACE)
			stateBitmask &= ~CreatureState::PEACE;

		stateBitmask &= ~CreatureState::COMBAT;

		CreatureObjectDeltaMessage3* dcreo3 = new CreatureObjectDeltaMessage3(
				asCreatureObject());
		dcreo3->updateCreatureBitmask(getOptionsBitmask());
		dcreo3->updateState();
		dcreo3->close();

		broadcastMessage(dcreo3, true);
	}

	clearQueueActions(false);

	if (removedefenders)
		removeDefenders();

	if (isRidingMount())
		removeMountedCombatSlow();

	debug("finished clearCombatState");
}

void CreatureObjectImplementation::setAlternateAppearance(const String& appearanceTemplate, bool notifyClient) {
	alternateAppearance = appearanceTemplate;

	if (!notifyClient)
		return;

	CreatureObjectDeltaMessage6* dcreo6 = new CreatureObjectDeltaMessage6(
			asCreatureObject());
	dcreo6->updateAlternateAppearance();
	dcreo6->close();

	broadcastMessage(dcreo6, true);
}

bool CreatureObjectImplementation::setState(uint64 state, bool notifyClient) {
	if (!(stateBitmask & state)) {
		stateBitmask |= state;

		if (notifyClient) {
			if (state == CreatureState::SITTINGONCHAIR) {
				//this is fucking wrong

				Zone* thisZone = getZoneUnsafe();

				setPosture(CreaturePosture::SITTING, false);

				if (thisZone != nullptr) {
					SortedVector<QuadTreeEntry*> closeSceneObjects;
					int maxInRangeObjects = 0;

					if (closeobjects == nullptr) {
#ifdef COV_DEBUG
						info("Null closeobjects vector in CreatureObjectImplementation::setState", true);
#endif
						thisZone->getInRangeObjects(getWorldPositionX(), getWorldPositionY(), ZoneServer::CLOSEOBJECTRANGE, &closeSceneObjects, true);
						maxInRangeObjects = closeSceneObjects.size();
					} else {
						closeobjects->safeCopyReceiversTo(closeSceneObjects, CloseObjectsVector::PLAYERTYPE);
						maxInRangeObjects = closeSceneObjects.size();
					}

					SitOnObject* soo = new SitOnObject(asCreatureObject(), getPositionX(), getPositionZ(), getPositionY());
					CreatureObjectDeltaMessage3* dcreo3 = new CreatureObjectDeltaMessage3(asCreatureObject());
					dcreo3->updatePosture();
					dcreo3->updateState();
					dcreo3->close();

#ifdef LOCKFREE_BCLIENT_BUFFERS
					Reference<BasePacket*> pack1 = soo;
					Reference<BasePacket*> pack2 = dcreo3;
#endif

					for (int i = 0; i < maxInRangeObjects; ++i) {
						SceneObject* object = static_cast<SceneObject*> (closeSceneObjects.get(i));

						if (object->getParent().get() == getParent().get()) {
#ifdef LOCKFREE_BCLIENT_BUFFERS
							object->sendMessage(pack1);
							object->sendMessage(pack2);
#else
							object->sendMessage(soo->clone());
							object->sendMessage(dcreo3->clone());
#endif
						} else {
							sendDestroyTo(object);
							sendTo(object, true);
						}
					}

#ifndef LOCKFREE_BCLIENT_BUFFERS
					delete soo;
					delete dcreo3;
#endif
				}
			} else {
				CreatureObjectDeltaMessage3* dcreo3 = new CreatureObjectDeltaMessage3(asCreatureObject());
				dcreo3->updateState();
				dcreo3->close();

				broadcastMessage(dcreo3, true);
			}

			switch (state) {
			case CreatureState::STUNNED:
				playEffect("clienteffect/combat_special_defender_stun.cef");
				sendSystemMessage("@cbt_spam:go_stunned_single");
				sendStateCombatSpam("cbt_spam", "go_stunned", 0);
				break;
			case CreatureState::BLINDED:
				playEffect("clienteffect/combat_special_defender_blind.cef");
				sendSystemMessage("@cbt_spam:go_blind_single");
				sendStateCombatSpam("cbt_spam", "go_blind", 0);
				break;
			case CreatureState::DIZZY: {
				playEffect("clienteffect/combat_special_defender_dizzy.cef");
				sendSystemMessage("@cbt_spam:go_dizzy_single");
				sendStateCombatSpam("cbt_spam", "go_dizzy", 0);
				break;
			}
			case CreatureState::POISONED:
				break;
			case CreatureState::DISEASED:
				break;
			case CreatureState::ONFIRE:
				break;
			case CreatureState::BLEEDING:
				break;
			case CreatureState::INTIMIDATED:
				playEffect("clienteffect/combat_special_defender_intimidate.cef");
				break;
			case CreatureState::IMMOBILIZED:
				showFlyText("combat_effects", "go_snare", 0, 0xFF, 0);
				break;
			case CreatureState::FROZEN:
				showFlyText("combat_effects", "go_rooted", 0, 0xFF, 0);
				break;
			case CreatureState::RALLIED:
				showFlyText("combat_effects", "go_rally", 0, 0xFF, 0);
				break;
			case CreatureState::BERSERK:
				playEffect("clienteffect/combat_special_attacker_berserk.cef");
				break;
			case CreatureState::AIMING:
				playEffect("clienteffect/combat_special_attacker_aim.cef");
				break;
			case CreatureState::COVER:
				playEffect("clienteffect/combat_special_attacker_cover.cef");
				sendSystemMessage("@cbt_spam:cover_success_single");
				sendStateCombatSpam("cbt_spam", "cover_success", 0);
				break;
			case CreatureState::PEACE:
				sendSystemMessage("@cbt_spam:peace_single");
				sendStateCombatSpam("cbt_spam", "peace", 0);
				break;

			default:
				break;
			}

		}

		return true;
	}

	return false;
}


int CreatureObjectImplementation::getReceiverFlags() const {
	int type = CloseObjectsVector::CREOTYPE;

	if (const_cast<CreatureObjectImplementation*>(this)->isPlayerCreature())
		type = type | CloseObjectsVector::PLAYERTYPE;

	return type | TangibleObjectImplementation::getReceiverFlags();
}

bool CreatureObjectImplementation::clearState(uint64 state, bool notifyClient) {
	if (stateBitmask & state) {
		stateBitmask &= ~state;

		if (notifyClient) {
			CreatureObjectDeltaMessage3* dcreo3 =
					new CreatureObjectDeltaMessage3(asCreatureObject());
			dcreo3->updateState();
			dcreo3->close();

			broadcastMessage(dcreo3, true);
		}

		switch (state) {
		case CreatureState::STUNNED:
			sendSystemMessage("@cbt_spam:no_stunned_single");
			sendStateCombatSpam("cbt_spam", "no_stunned", 0);
			break;
		case CreatureState::BLINDED:
			sendSystemMessage("@cbt_spam:no_blind_single");
			sendStateCombatSpam("cbt_spam", "no_blind", 0);
			break;
		case CreatureState::DIZZY:
			sendSystemMessage("@cbt_spam:no_dizzy_single");
			sendStateCombatSpam("cbt_spam", "no_dizzy", 0);
			break;
		case CreatureState::POISONED:
			sendSystemMessage("@dot_message:stop_poisoned");
			break;
		case CreatureState::DISEASED:
			sendSystemMessage("@dot_message:stop_diseased");
			break;
		case CreatureState::ONFIRE:
			sendSystemMessage("@dot_message:stop_fire");
			break;
		case CreatureState::BLEEDING:
			sendSystemMessage("@dot_message:stop_bleeding");
			break;
		case CreatureState::INTIMIDATED:
			break;
		case CreatureState::IMMOBILIZED:
			showFlyText("combat_effects", "no_snare", 0, 0xFF, 0);
			break;
		case CreatureState::FROZEN:
			showFlyText("combat_effects", "no_rooted", 0, 0xFF, 0);
			break;
		case CreatureState::RALLIED:
			showFlyText("combat_effects", "no_rally", 0xFF, 0, 0);
			break;
		case CreatureState::BERSERK:
			break;
		case CreatureState::AIMING:
			break;
		case CreatureState::COVER: {
			if (hasBuff(CreatureState::COVER)) {
				removeBuff(CreatureState::COVER);
			}
			break;
		}
		default:
			break;
		}

		return true;
	} else {
		return false;
	}
}

void CreatureObjectImplementation::setHAM(int type, int value,
		bool notifyClient) {
	if (hamList.get(type) == value)
		return;

	debug() << "setting ham type " << type << " to " << value;

	if (notifyClient) {
		CreatureObjectDeltaMessage6* msg = new CreatureObjectDeltaMessage6(
				asCreatureObject());
		msg->startUpdate(0x0D);
		hamList.set(type, value, msg);
		msg->close();

		broadcastMessage(msg, true);
	} else {
		hamList.set(type, value, nullptr);
	}
}

int CreatureObjectImplementation::inflictDamage(TangibleObject* attacker, int damageType, float damage, bool destroy, const String& xp, bool notifyClient, bool isCombatAction) {
	if (attacker->isCreatureObject()) {
		CreatureObject* creature = attacker->asCreatureObject();

		if (damage > 0) {
			getThreatMap()->addDamage(creature, damage, xp);
		}
	}

	return inflictDamage(attacker, damageType, damage, destroy, notifyClient, isCombatAction);
}

int CreatureObjectImplementation::inflictDamage(TangibleObject* attacker, int damageType, float damage, bool destroy, bool notifyClient, bool isCombatAction) {
	if (damageType < 0 || damageType >= hamList.size()) {
		error("incorrect damage type in CreatureObjectImplementation::inflictDamage");
		return 0;
	}

	if ((isIncapacitated() && !isFeigningDeath()) || this->isDead() || damage == 0)
		return 0;

	int currentValue = hamList.get(damageType);

	int newValue = currentValue - (int) damage;

	if (!destroy && newValue <= 0)
		newValue = 1;

	if (getSkillMod("avoid_incapacitation") > 0 && newValue <= 0)
		newValue = 1;

	if (damageType % 3 != 0 && newValue < 0) // secondaries never should go negative
		newValue = 0;

	setHAM(damageType, newValue, notifyClient);

	if (attacker == nullptr)
		attacker = asCreatureObject();

	if (newValue <= 0)
		notifyObjectDestructionObservers(attacker, newValue, isCombatAction);

	return 0;
}

int CreatureObjectImplementation::healDamage(TangibleObject* healer,
		int damageType, int damage, bool notifyClient, bool notifyObservers) {
	if (damage == 0)
		return 0;

	if (damageType < 0 || damageType >= hamList.size()) {
		error("incorrect damage type in CreatureObjectImplementation::healDamage");
		return 0;
	}

	int returnValue = damage;

	int currentValue = hamList.get(damageType);

	int newValue = currentValue + damage;

	int maxValue = maxHamList.get(damageType) - wounds.get(damageType);

	if (newValue > maxValue)
		returnValue = maxValue - currentValue;

	newValue = Math::min(newValue, maxValue);

	if (currentValue <= 0 && (isIncapacitated() || isDead())) {
		if (newValue <= 0)
			newValue = 1;

		if (damageType % 3 == 0) {

			setPosture(CreaturePosture::UPRIGHT);

			asCreatureObject()->notifyObservers(ObserverEventType::CREATUREREVIVED, healer, 0);

			if(isPlayerCreature()) {

				PlayerObject* ghost = getPlayerObject();

				if (ghost->getForcePowerMax() > 0 && ghost->getForcePower() < ghost->getForcePowerMax()) {
					ghost->activateForcePowerRegen();
				}
			}

			if (isPet()) {
				AiAgent* pet = asAiAgent();
				ManagedReference<CreatureObject*> player = getLinkedCreature().get();

				if (pet != nullptr && player != nullptr) {
					pet->setFollowObject(player);
					pet->activateMovementEvent();
				}
			}
		}
	}

	if (damageType % 3 != 0 && newValue < 0)
		newValue = 0;

	setHAM(damageType, newValue, notifyClient);

	if(healer != nullptr && notifyObservers) {
		asCreatureObject()->notifyObservers(ObserverEventType::HEALINGRECEIVED, healer, returnValue);
	}

	return returnValue;
}

int CreatureObjectImplementation::healWound(TangibleObject* healer,
		int damageType, int damage, bool notifyClient, bool notifyObservers) {
	if (damage == 0)
		return 0;

	if (damageType < 0 || damageType >= wounds.size()) {
		error("incorrect damage type in CreatureObjectImplementation::healWound");
		return 0;
	}

	int currentValue = wounds.get(damageType);

	int newValue = currentValue - damage;

	newValue = Math::max(newValue, 0);

	int returnValue = currentValue - newValue; //How many wounds we had minus how many we now have = how much was healed.

	setWounds(damageType, newValue, notifyClient);

	if (healer != nullptr && notifyObservers) {
		asCreatureObject()->notifyObservers(ObserverEventType::WOUNDHEALINGRECEIVED, healer, returnValue);
	}

	return returnValue;
}

void CreatureObjectImplementation::setBaseHAM(int type, int value,
		bool notifyClient) {
	if (baseHAM.get(type) == value)
		return;

	debug() << "setting baseham type " << type << " to " << value;

	if (notifyClient) {
		CreatureObjectDeltaMessage1* msg =
				new CreatureObjectDeltaMessage1(this);
		msg->startUpdate(0x02);
		baseHAM.set(type, value, msg);
		msg->close();

		broadcastMessage(msg, true);
	} else {
		baseHAM.set(type, value, nullptr);
	}
}

void CreatureObjectImplementation::setWounds(int type, int value,
		bool notifyClient) {
	if (value < 0)
		value = 0;

	if (value >= baseHAM.get(type))
		value = baseHAM.get(type) - 1;

	if (wounds.get(type) == value)
		return;

	if (notifyClient) {
		CreatureObjectDeltaMessage3* msg = new CreatureObjectDeltaMessage3(
				asCreatureObject());
		msg->startUpdate(0x11);
		wounds.set(type, value, msg);
		msg->close();

		broadcastMessage(msg, true);
	} else {
		wounds.set(type, value, nullptr);
	}

	int maxHamValue = maxHamList.get(type) - wounds.get(type);

	if (getHAM(type) > maxHamValue) {
		setHAM(type, maxHamValue, notifyClient);
	} else if (type % 3 != 0) { //update secondary stats to the max
		setHAM(type, maxHamValue, notifyClient);
	}
}

int CreatureObjectImplementation::addWounds(int type, int value, bool notifyClient, bool doShockWounds) {
	if (type < 0 || type >= wounds.size()) {
		error("unknown wound type in addWounds");
		return 0;
	}

	int returnValue = value;

	int currentValue = wounds.get(type);

	int newValue = currentValue + value;

	if (newValue < 0)
		returnValue = -currentValue;

	if (newValue >= baseHAM.get(type))
		returnValue = baseHAM.get(type) - 1 - currentValue;

	if (value > 0 && asCreatureObject()->isPlayerCreature())
		sendStateCombatSpam("cbt_spam", "wounded", 1, value, false);

	setWounds(type, newValue, notifyClient);

	if (doShockWounds)
		addShockWounds(1, true);

	return returnValue;
}

void CreatureObjectImplementation::setMaxHAM(int type, int value,
		bool notifyClient) {
	if (maxHamList.get(type) == value)
		return;

	debug() << "setting maxham type " << type << " to " << value;

	if (notifyClient) {
		CreatureObjectDeltaMessage6* msg = new CreatureObjectDeltaMessage6(
				asCreatureObject());
		msg->startUpdate(0x0E);
		maxHamList.set(type, value, msg);
		msg->close();

		broadcastMessage(msg, true);
	} else {
		maxHamList.set(type, value, nullptr);
	}

	if (wounds.get(type) >= maxHamList.get(type)) // this will reset our wounds to not overflow max value
		setWounds(type, wounds.get(type), notifyClient);

	if (type % 3 != 0) { //changing secondary stats, updating current value
		healDamage(asCreatureObject(), type, value - hamList.get(type), notifyClient);
	}
}

void CreatureObjectImplementation::setEncumbrance(int type, int value,
		bool notifyClient) {
	if (encumbrances.get(type) == value)
		return;

	if (notifyClient) {
		CreatureObjectDeltaMessage4* msg =
				new CreatureObjectDeltaMessage4(asCreatureObject());
		msg->startUpdate(0x02);
		encumbrances.set(type, value, msg);
		msg->close();

		broadcastMessage(msg, true);
	} else {
		encumbrances.set(type, value, nullptr);
	}
}

void CreatureObjectImplementation::addMaxHAM(int type, int value,
		bool notifyClient) {
	if (type < 0 || type > maxHamList.size()) {
		error("invalid type in CreatureObjectImplementation::addMaxHAM");
		return;
	}

	int currentValue = maxHamList.get(type);
	int newValue = currentValue + value;

	setMaxHAM(type, newValue, notifyClient);
}

void CreatureObjectImplementation::addEncumbrance(int type, int value,
		bool notifyClient) {
	if (type < 0 || type >= encumbrances.size()) {
		error("invalid type in CreatureObjectImplementation::addEncumbrance");
		return;
	}

	int currentValue = encumbrances.get(type);
	int newValue = currentValue + value;

	setEncumbrance(type, newValue, notifyClient);
}

void CreatureObjectImplementation::addSkill(Skill* skill, bool notifyClient) {
	if (skillList.contains(skill))
		return;

	if (notifyClient) {
		CreatureObjectDeltaMessage1* msg =
				new CreatureObjectDeltaMessage1(this);
		msg->startUpdate(0x03);
		skillList.add(skill, msg);
		msg->close();

		sendMessage(msg);
	} else {
		skillList.add(skill, nullptr);
	}

	// Some skills affect player movement, update speed and acceleration.
	updateSpeedAndAccelerationMods();
}

void CreatureObjectImplementation::removeSkill(Skill* skill, bool notifyClient) {
	if (!skillList.contains(skill))
		return;

	if (notifyClient) {
		CreatureObjectDeltaMessage1* msg =
				new CreatureObjectDeltaMessage1(this);
		msg->startUpdate(0x03);
		skillList.remove(skill, msg);
		msg->close();

		sendMessage(msg);
	} else {
		skillList.remove(skill);
	}

	// Some skills affect player movement, update speed and acceleration.
	updateSpeedAndAccelerationMods();
}

void CreatureObjectImplementation::removeSkill(const String& skill,
		bool notifyClient) {
	SkillManager* skillManager = server->getZoneServer()->getSkillManager();

	Reference<Skill*> skillObject = skillManager->getSkill(skill);

	if (skillObject == nullptr) {
		error("trying to remove null skill  " + skill);
		return;
	}

	removeSkill(skillObject, notifyClient);
}

void CreatureObjectImplementation::addSkillMod(const int modType, const String& skillMod, int value, bool notifyClient) {
	Locker locker(&skillModMutex);

	SkillModEntry oldMod;

	if (skillModList.contains(skillMod)) {
		oldMod = skillModList.get(skillMod);
	}

	skillModList.add(modType, skillMod, value);

	SkillModEntry newMod = skillModList.getVisibleSkillMod(skillMod);

	if (newMod == oldMod) {
		return;
	}

	if (notifyClient) {
		CreatureObjectDeltaMessage4* msg = new CreatureObjectDeltaMessage4(asCreatureObject());
		msg->startUpdate(0x03);

		if (newMod.getTotalSkill() != 0)
			skillModList.set(skillMod, newMod, msg, 1);
		else {
			skillModList.drop(skillMod, msg, 1);
		}

		msg->close();

		sendMessage(msg);
	} else {
		if (newMod.getTotalSkill() != 0)
			skillModList.set(skillMod, newMod);
		else
			skillModList.drop(skillMod);
	}
}

void CreatureObjectImplementation::removeSkillMod(const int modType, const String& skillMod, int value, bool notifyClient) {
	addSkillMod(modType, skillMod, -value, notifyClient);
}

void CreatureObjectImplementation::removeAllSkillModsOfType(const int modType, bool notifyClient) {
	Locker locker(&skillModMutex);

	SkillModGroup* modGroup = skillModList.getSkillModGroup(modType);

	if (notifyClient) {
		for (int i = modGroup->size() - 1; i >= 0; --i) {
			VectorMapEntry<String, int>* entry = &modGroup->elementAt(i);
			String key = entry->getKey();
			int val = entry->getValue();
			// use the type instead of hardcoding CITY here
			removeSkillMod(modType, key, val, true);
		}
	} else {
		modGroup->removeAll();
	}
}

int CreatureObjectImplementation::getSkillMod(const String& skillmod) const {
	ReadLocker locker(&skillModMutex);

	return skillModList.getSkillMod(skillmod);
}

int CreatureObjectImplementation::getSkillModOfType(const String& skillmod, const unsigned int modType) {
	Locker locker(&skillModMutex);

	return skillModList.getSkillModOfType(skillmod, modType);
}

void CreatureObjectImplementation::addSkill(const String& skill,
		bool notifyClient) {
	SkillManager* skillManager = server->getZoneServer()->getSkillManager();

	Reference<Skill*> skillObject = skillManager->getSkill(skill);

	if (skillObject == nullptr) {
		error("trying to add null skill box " + skill);
		return;
	}

	addSkill(skillObject, notifyClient);
}

void CreatureObjectImplementation::updatePostures(bool immediate) {

	updateSpeedAndAccelerationMods();

	// TODO: these two seem to be as of yet unused (maybe only necessary in client)
	//CreaturePosture::instance()->getTurnScale((uint8)newPosture);
	//CreaturePosture::instance()->getCanSeeHeightMod((uint8)newPosture);

	if (posture != CreaturePosture::SITTING && hasState(
			CreatureState::SITTINGONCHAIR))
		clearState(CreatureState::SITTINGONCHAIR);


	Vector<BasePacket*> messages;

	if (immediate) {

		// This invokes an immediate posture change animation - Attacks and animations which force a creature to change postures should not send this.
		PostureMessage* octrl = new PostureMessage(asCreatureObject());
		messages.add(octrl);

		if (!isProbotSpecies() && (isPlayerCreature() || isAiAgent())) {
			switch (posture) {
			case CreaturePosture::UPRIGHT:
				sendStateCombatSpam("cbt_spam", "stand", 11);
				break;
			case CreaturePosture::PRONE:
				sendStateCombatSpam("cbt_spam", "prone", 11);
				break;
			case CreaturePosture::CROUCHED:
				sendStateCombatSpam("cbt_spam", "kneel", 11);
				break;
			}
		}
	}

	// This will not instantly force a posture change animation but will update the creatures posture variable in the client.
	// Failing to send this will result in the creature returning to it's previous posture after a CombatAction
	CreatureObjectDeltaMessage3* dcreo3 = new CreatureObjectDeltaMessage3(
			asCreatureObject());
	dcreo3->updatePosture();
	//dcreo3->updateState();
	dcreo3->close();

	messages.add(dcreo3);

	broadcastMessages(&messages, true);

	if(posture != CreaturePosture::UPRIGHT && posture != CreaturePosture::DRIVINGVEHICLE
				&& posture != CreaturePosture::RIDINGCREATURE && posture != CreaturePosture::SKILLANIMATING ) {
		setCurrentSpeed(0);
	}

	updateLocomotion();
	notifyPostureChange(posture);
}

void CreatureObjectImplementation::setPosture(int newPosture, bool immediate, bool notifyClient) {
	if (posture == newPosture)
		return;

	if (posture == CreaturePosture::PRONE && isInCover()) {
		clearState(CreatureState::COVER);
	}

	posture = newPosture;

	if(!notifyClient)
		return;

	updatePostures(immediate);
}

void CreatureObjectImplementation::updateSpeedAndAccelerationMods() {
	float speedboost = 0;

	if(posture == CreaturePosture::PRONE && !hasBuff(CreatureState::COVER)) {
		speedboost = getSkillMod("slope_move") >= 50
				? ((getSkillMod("slope_move") - 50.0f) / 100.0f) / 2 : 0;
	}

	setSpeedMultiplierMod(CreaturePosture::instance()->getMovementScale((uint8) posture) + speedboost, true);

	setAccelerationMultiplierMod(CreaturePosture::instance()->getAccelerationScale((uint8) posture), true);

	setTurnScale(CreaturePosture::instance()->getTurnScale((uint8) posture), true);
}

float CreatureObjectImplementation::calculateSpeed() {
	Time currentTime;
	uint32 deltaTime = currentTime.getMiliTime() - lastCombatActionTime.getMiliTime();
	Vector3 newPosition = getPosition();

	float dist = newPosition.distanceTo(lastCombatPosition);
	float speed = dist / (float) deltaTime * 1000;

	//update position and time
	lastCombatActionTime.updateToCurrentTime();
	lastCombatPosition = newPosition;

	return speed;
}

void CreatureObjectImplementation::updateLocomotion() {
	// 0: stationary, 0-walk: slow, walk-run+; fast
	// the movement table does not seem to want scaling factors...
	if (currentSpeed <= walkSpeed / 10.f)
		locomotion = CreaturePosture::instance()->getLocomotion(posture,
				CreatureLocomotion::STATIONARY);
	else if (currentSpeed <= walkSpeed + walkSpeed / 10.0f)
		locomotion = CreaturePosture::instance()->getLocomotion(posture,
				CreatureLocomotion::SLOW);
	else
		locomotion = CreaturePosture::instance()->getLocomotion(posture,
				CreatureLocomotion::FAST);
}

UnicodeString CreatureObjectImplementation::getCreatureName() const {
	return getCustomObjectName();
}

void CreatureObjectImplementation::updateGroupInviterID(uint64 id,
		bool notifyClient) {
	groupInviterID = id;
	++groupInviteCounter;

	CreatureObjectDeltaMessage6* delta = new CreatureObjectDeltaMessage6(asCreatureObject());
	delta->updateInviterId();
	delta->close();

	broadcastMessage(delta, true);
}

void CreatureObjectImplementation::updateGroup(GroupObject* grp,
		bool notifyClient) {
	group = grp;

	CreatureObjectDeltaMessage6* delta = new CreatureObjectDeltaMessage6(asCreatureObject());
	delta->updateGroupID();
	delta->close();

	broadcastMessage(delta, true);
}

void CreatureObjectImplementation::setMood(byte mood, bool notifyClient) {
	moodID = mood;

	ChatManager* chatManager = server->getChatManager();

	moodString = chatManager->getMoodAnimation(chatManager->getMoodType(moodID));

	if (notifyClient) {
		CreatureObjectDeltaMessage6* dcreo6 = new CreatureObjectDeltaMessage6(
				asCreatureObject());
		dcreo6->updateMoodID();
		dcreo6->updateMoodStr();
		dcreo6->close();

		broadcastMessage(dcreo6, true);
	}
}

void CreatureObjectImplementation::setAccelerationMultiplierBase(
		float newMultiplierBase, bool notifyClient) {
	if (accelerationMultiplierBase == newMultiplierBase)
		return;

	accelerationMultiplierBase = newMultiplierBase;

	if (notifyClient) {
		CreatureObjectDeltaMessage4* dcreo4 = new CreatureObjectDeltaMessage4(
				asCreatureObject());
		dcreo4->updateAccelerationMultiplierBase();
		dcreo4->close();

		sendMessage(dcreo4);
	}
}

void CreatureObjectImplementation::setAccelerationMultiplierMod(float newMultiplierMod, bool notifyClient) {
	float buffMod = getSkillMod("private_acceleration_multiplier") > 0 ? (float)getSkillMod("private_acceleration_multiplier") / 100.f : 1.f;

	if (accelerationMultiplierMod == newMultiplierMod * buffMod)
		return;

	accelerationMultiplierMod = newMultiplierMod * buffMod;

	if (notifyClient) {
		CreatureObjectDeltaMessage4* dcreo4 = new CreatureObjectDeltaMessage4(
				asCreatureObject());
		dcreo4->updateAccelerationMultiplierMod();
		dcreo4->close();

		sendMessage(dcreo4);
	}
}

void CreatureObjectImplementation::setSpeedMultiplierBase(
		float newMultiplierBase, bool notifyClient) {
	if (speedMultiplierBase == newMultiplierBase)
		return;

	speedMultiplierBase = newMultiplierBase;

	if (notifyClient) {
		CreatureObjectDeltaMessage4* dcreo4 = new CreatureObjectDeltaMessage4(
				asCreatureObject());
		dcreo4->updateSpeedMultiplierBase();
		dcreo4->close();

		sendMessage(dcreo4);
	}
}

void CreatureObjectImplementation::setTurnScale(
		float newMultiplierBase, bool notifyClient) {
	if (turnScale == newMultiplierBase)
		return;

	turnScale = newMultiplierBase;

	if (notifyClient) {
		CreatureObjectDeltaMessage4* dcreo4 = new CreatureObjectDeltaMessage4(
				asCreatureObject());
		dcreo4->updateTurnScale();
		dcreo4->close();

		sendMessage(dcreo4);
	}
}

void CreatureObjectImplementation::setFactionRank(int rank, bool notifyClient) {
	if (factionRank == rank)
		return;

	factionRank = rank;

	if (!notifyClient)
		return;

	CreatureObjectDeltaMessage3* msg = new CreatureObjectDeltaMessage3(asCreatureObject());
	msg->updateFactionRank();
	msg->close();
	broadcastMessage(msg, true);
}

void CreatureObjectImplementation::setSpeedMultiplierMod(float newMultiplierMod, bool notifyClient) {
	float buffMod = 1;

	if (posture == CreaturePosture::UPRIGHT) {
		buffMod = getSkillMod("private_speed_multiplier") > 0 ? (float)getSkillMod("private_speed_multiplier") / 100.f : 1.f;
	} else if(posture == CreaturePosture::PRONE && hasState(CreatureState::COVER)) {
		if (hasSkill("combat_rifleman_speed_03")) {
			buffMod = 0.5f;
		} else {
			buffMod = 0.f;
		}
	}

	if (speedMultiplierMod == newMultiplierMod * buffMod)
		return;

	speedMultiplierMod = newMultiplierMod * buffMod;

	int bufferSize = speedMultiplierModChanges.size();

	if (bufferSize > 5) {
		speedMultiplierModChanges.remove(0);
	}

	speedMultiplierModChanges.add(SpeedModChange(speedMultiplierMod));

	if (notifyClient) {
		CreatureObjectDeltaMessage4* dcreo4 = new CreatureObjectDeltaMessage4(
				asCreatureObject());
		dcreo4->updateSpeedMultiplierMod();
		dcreo4->close();

		sendMessage(dcreo4);
	}
}

void CreatureObjectImplementation::setRunSpeed(float newSpeed,
		bool notifyClient) {
	if (runSpeed == newSpeed)
		return;

	runSpeed = newSpeed;

	if (notifyClient) {
		CreatureObjectDeltaMessage4* dcreo4 = new CreatureObjectDeltaMessage4(
				asCreatureObject());
		dcreo4->updateRunSpeed();
		dcreo4->close();

		sendMessage(dcreo4);
	}
}

void CreatureObjectImplementation::setMoodString(
		const String& moodAnimationString, bool notifyClient) {
	moodString = moodAnimationString;

	if (notifyClient) {
		CreatureObjectDeltaMessage6* dcreo6 = new CreatureObjectDeltaMessage6(
				asCreatureObject());
		dcreo6->updateMoodStr();
		dcreo6->close();

		broadcastMessage(dcreo6, true);
	}
}

void CreatureObjectImplementation::setPerformanceCounter(int counter,
		bool notifyClient) {
	if (performanceCounter == counter)
		return;

	performanceCounter = counter;

	if (!notifyClient)
		return;

	CreatureObjectDeltaMessage6* codm4 = new CreatureObjectDeltaMessage6(asCreatureObject());
	codm4->updatePerformanceCounter(counter);
	codm4->close();
	broadcastMessage(codm4, true);
}

void CreatureObjectImplementation::setListenToID(uint64 id, bool notifyClient) {
	if (listenToID == id)
		return;

	listenToID = id;

	if (!notifyClient)
		return;

	CreatureObjectDeltaMessage4* codm4 = new CreatureObjectDeltaMessage4(asCreatureObject());
	codm4->updateListenToID(id);
	codm4->close();
	sendMessage(codm4);
}

void CreatureObjectImplementation::setPerformanceAnimation(
		const String& animation, bool notifyClient) {
	if (performanceAnimation == animation)
		return;

	performanceAnimation = animation;

	if (!notifyClient)
		return;

	CreatureObjectDeltaMessage6* codm4 = new CreatureObjectDeltaMessage6(asCreatureObject());
	codm4->updatePerformanceAnimation(animation);
	codm4->close();
	broadcastMessage(codm4, true);
}

void CreatureObjectImplementation::setTerrainNegotiation(float value,
		bool notifyClient) {
	terrainNegotiation = value;

	if (notifyClient)
		updateTerrainNegotiation();
}

void CreatureObjectImplementation::updateTerrainNegotiation()
{
	CreatureObjectDeltaMessage4* codm4 = new CreatureObjectDeltaMessage4(asCreatureObject());
	codm4->updateTerrainNegotiation();
	codm4->close();
	sendMessage(codm4);
}

float CreatureObjectImplementation::getTerrainNegotiation() const {
	float slopeMod = ((float)getSkillMod("slope_move") / 50.0f) + terrainNegotiation;

	if (slopeMod > 1)
		slopeMod = 1;

	return slopeMod;
}

void CreatureObjectImplementation::enqueueCommand(unsigned int actionCRC, unsigned int actionCount, uint64 targetID, const UnicodeString& arguments, int priority, int compareCounter) {
	ManagedReference<ObjectController*> objectController = getZoneServer()->getObjectController();

	const QueueCommand* queueCommand = objectController->getQueueCommand(actionCRC);

	if (queueCommand == nullptr) {
		return;
	}

	if (queueCommand->addToCombatQueue()) {
		removeBuff(STRING_HASHCODE("private_feign_buff"));
	}

	if (priority < 0)
		priority = queueCommand->getDefaultPriority();

	Reference<CommandQueueAction*> action = nullptr;

	if (priority == QueueCommand::IMMEDIATE) {
#ifndef WITH_STM
		objectController->activateCommand(asCreatureObject(), actionCRC, actionCount, targetID, arguments);
#else
		action = new CommandQueueAction(asCreatureObject(), targetID, actionCRC, actionCount, arguments);

		immediateQueue->put(action.get());

		if (immediateQueue->size() == 1) {
			Reference<CommandQueueActionEvent*> ev = new CommandQueueActionEvent(asCreatureObject(), CommandQueueActionEvent::IMMEDIATE);
			Core::getTaskManager()->executeTask(ev);
		}
#endif

		return;
	}

	if (commandQueue->size() > 15 && priority != QueueCommand::FRONT) {
		clearQueueAction(actionCount);

		return;
	}

	action = new CommandQueueAction(asCreatureObject(), targetID, actionCRC, actionCount, arguments);

	if (compareCounter >= 0)
		action->setCompareToCounter((int)compareCounter);

	if (commandQueue->size() != 0 || !nextAction.isPast()) {
		if (commandQueue->size() == 0) {
			Reference<CommandQueueActionEvent*> e = new CommandQueueActionEvent(asCreatureObject());
			e->schedule(nextAction);
		}

		if (priority == QueueCommand::NORMAL) {
			commandQueue->put(action.get());
		} else if (priority == QueueCommand::FRONT) {
			if (commandQueue->size() > 0) {
				action->setCompareToCounter(commandQueue->get(0)->getCompareToCounter() - 1);
			}

			commandQueue->put(action.get());
		}
	} else {
		nextAction.updateToCurrentTime();

		commandQueue->put(action.get());
		activateQueueAction();
	}
}

void CreatureObjectImplementation::sendCommand(const String& action, const UnicodeString& args, uint64 targetID, int priority) {
	sendCommand(action.hashCode(), args, targetID, priority);
}

void CreatureObjectImplementation::sendCommand(uint32 crc, const UnicodeString& args, uint64 targetID, int priority) {
	uint32 nextCounter = incrementLastActionCounter();
	CommandQueueAdd* msg = new CommandQueueAdd(asCreatureObject(), crc, nextCounter);
	sendMessage(msg);

	int compareCnt = -1;
	if (commandQueue->size() == 0 || priority == QueueCommand::FRONT)
		compareCnt = 0;
	else if (priority == QueueCommand::NORMAL)
		compareCnt = commandQueue->get(commandQueue->size() - 1)->getCompareToCounter() + 1;

	enqueueCommand(crc, nextCounter, targetID, args, priority, compareCnt);
}

void CreatureObjectImplementation::activateImmediateAction() {
	Reference<CommandQueueAction*> action = immediateQueue->get(0);

	ManagedReference<ObjectController*> objectController = getZoneServer()->getObjectController();

	float time = objectController->activateCommand(asCreatureObject(), action->getCommand(), action->getActionCounter(), action->getTarget(), action->getArguments());

	// Remove element from queue after it has been executed in order to ensure that other commands are enqueued and not activated at immediately.
	immediateQueue->remove(0);

	if (immediateQueue->size() > 0) {
		Reference<CommandQueueActionEvent*> ev = new CommandQueueActionEvent(asCreatureObject(), CommandQueueActionEvent::IMMEDIATE);
		Core::getTaskManager()->executeTask(ev);
	}
}

void CreatureObjectImplementation::activateQueueAction() {
	if (nextAction.isFuture()) {
		CommandQueueActionEvent* e = new CommandQueueActionEvent(asCreatureObject());
		e->schedule(nextAction);

		return;
	}

	if (commandQueue->size() == 0) {
		return;
	}

	Reference<CommandQueueAction*> action = commandQueue->get(0);

	ManagedReference<ObjectController*> objectController = getZoneServer()->getObjectController();

	nextAction.updateToCurrentTime();
	nextAction.addMiliTime(1000);

	float time = objectController->activateCommand(asCreatureObject(), action->getCommand(), action->getActionCounter(), action->getTarget(), action->getArguments());

	nextAction.updateToCurrentTime();

	if (time > 0) {
		nextAction.addMiliTime((uint32)(time * 1000));
	}

	// Remove element from queue after it has been executed in order to ensure that other commands are enqueued and not activated at immediately.
	for (int i = 0; i < commandQueue->size(); i++) {
		Reference<CommandQueueAction*> actionToDelete = commandQueue->get(i);
		if (action->getCommand() == actionToDelete->getCommand() && action->getActionCounter() == actionToDelete->getActionCounter() && action->getCompareToCounter() == actionToDelete->getCompareToCounter()) {
			commandQueue->remove(i);
			break;
		}
	}

	if (commandQueue->size() != 0) {
		Reference<CommandQueueActionEvent*> e = new CommandQueueActionEvent(asCreatureObject());

		if (!nextAction.isFuture()) {
			nextAction.updateToCurrentTime();
			nextAction.addMiliTime(100);
		}

		e->schedule(nextAction);
	}
}

void CreatureObjectImplementation::deleteQueueAction(uint32 actionCount) {
	for (int i = 0; i < commandQueue->size(); ++i) {
		CommandQueueAction* action = commandQueue->get(i);

		if (action->getActionCounter() == actionCount) {
			commandQueue->remove(i);
			break;
		}
	}
}

void CreatureObjectImplementation::addBankCredits(int credits, bool notifyClient) {
	Locker locker(creditObject);
	creditObject->addBankCredits(credits, notifyClient);
}

void CreatureObjectImplementation::addCashCredits(int credits, bool notifyClient) {
	Locker locker(creditObject);
	creditObject->addCashCredits(credits, notifyClient);
}

void CreatureObjectImplementation::clearBankCredits(bool notifyClient) {
	Locker locker(creditObject);
	creditObject->clearBankCredits(notifyClient);
}

void CreatureObjectImplementation::clearCashCredits(bool notifyClient) {
	Locker locker(creditObject);
	creditObject->clearCashCredits(notifyClient);
}

void CreatureObjectImplementation::transferCredits(int cash, int bank, bool notifyClient) {
	Locker locker(creditObject);
	creditObject->transferCredits(cash, bank, notifyClient);
}

void CreatureObjectImplementation::subtractBankCredits(int credits) {
	Locker locker(creditObject);
	creditObject->subtractBankCredits(credits, true);
}

void CreatureObjectImplementation::subtractCashCredits(int credits) {
	Locker locker(creditObject);
	creditObject->subtractCashCredits(credits, true);
}

bool CreatureObjectImplementation::subtractCredits(int credits) {
	Locker locker(creditObject);
	return creditObject->subtractCredits(credits, true);
}

bool CreatureObjectImplementation::verifyCashCredits(int credits) {
	Locker locker(creditObject);
	return creditObject->verifyCashCredits(credits);
}

bool CreatureObjectImplementation::verifyBankCredits(int credits) {
	Locker locker(creditObject);
	return creditObject->verifyBankCredits(credits);
}

bool CreatureObjectImplementation::verifyCredits(int credits) {
	Locker locker(creditObject);
	return creditObject->verifyCredits(credits);
}

void CreatureObjectImplementation::notifyLoadFromDatabase() {
	TangibleObjectImplementation::notifyLoadFromDatabase();
	/**
	 * Here we are loading the schematics based on the skills that the
	 * player has, we do this incase we change the items
	 * in the schematic group.
	 */
	PlayerObject* ghost = getPlayerObject();

	if (hasState(CreatureState::ALERT)) {
		clearState(CreatureState::ALERT);
		setMoodString("neutral");
	}

	listenToID = 0;
	watchToID = 0;

	if (isIncapacitated()) {
		int health = getHAM(CreatureAttribute::HEALTH);

		if (health < 0)
			setHAM(CreatureAttribute::HEALTH, 1);

		int action = getHAM(CreatureAttribute::ACTION);

		if (action < 0)
			setHAM(CreatureAttribute::ACTION, 1);

		int mind = getHAM(CreatureAttribute::MIND);

		if (mind < 0)
			setHAM(CreatureAttribute::MIND, 1);


		removeFeignedDeath();

		setPosture(CreaturePosture::UPRIGHT);
	}

	if (ghost == nullptr)
		return;

	getZoneServer()->getPlayerManager()->fixHAM(asCreatureObject());
	getZoneServer()->getPlayerManager()->fixBuffSkillMods(asCreatureObject());

	for (int i = 0; i < creatureBuffs.getBuffListSize(); ++i) {
		ManagedReference<Buff*> buff = creatureBuffs.getBuffByIndex(i);

		buff->loadBuffDurationEvent(asCreatureObject());
	}

	ZoneServer* zoneServer = server->getZoneServer();
	SkillManager* skillManager = SkillManager::instance();

	const SkillList* playerSkillList = getSkillList();

	int totalSkillPointsWasted = 250;

	for (int i = 0; i < playerSkillList->size(); ++i) {
		Skill* skill = playerSkillList->get(i);

		if (skill == nullptr)
			continue;

		skillManager->awardDraftSchematics(skill, ghost, false);

		totalSkillPointsWasted -= skill->getSkillPointsRequired();
	}

	if (ghost->getSkillPoints() != totalSkillPointsWasted) {
		error() << "skill points on load mismatch calculated: " << totalSkillPointsWasted
		       << " found: " << ghost->getSkillPoints();
		ghost->setSkillPoints(totalSkillPointsWasted);
	}

	ghost->getSchematics()->addRewardedSchematics(ghost);

	skillManager->updateXpLimits(ghost);

	if (getZone() != nullptr)
		ghost->setLinkDead();
}

void CreatureObjectImplementation::notifyInsert(QuadTreeEntry* obj) {
	auto linkedCreature = getLinkedCreature().get();

	if (linkedCreature != nullptr && linkedCreature->getParent() == asCreatureObject()) {
#if DEBUG_COV
		linkedCreature->info("proxy notifyInsert(" + String::valueOf(obj->getObjectID()) + ")");
#endif // DEBUG_COV

		if (linkedCreature->getCloseObjects() != nullptr)
			linkedCreature->addInRangeObject(obj);

		if (obj->getCloseObjects() != nullptr)
			obj->addInRangeObject(linkedCreature);
	}

	TangibleObjectImplementation::notifyInsert(obj);
}

void CreatureObjectImplementation::notifyDissapear(QuadTreeEntry* obj) {
	auto linkedCreature = getLinkedCreature().get();

	if (linkedCreature != nullptr && linkedCreature->getParent() == asCreatureObject()) {
#if DEBUG_COV
		linkedCreature->info("proxy notifyDissapear(" + String::valueOf(obj->getObjectID()) + ")");
#endif // DEBUG_COV
		if (linkedCreature->getCloseObjects() != nullptr)
			linkedCreature->removeInRangeObject(obj);

		if (obj->getCloseObjects() != nullptr)
			obj->removeInRangeObject(linkedCreature);
	}

	TangibleObjectImplementation::notifyDissapear(obj);
}

void CreatureObjectImplementation::notifyPositionUpdate(QuadTreeEntry* entry) {
	auto linkedCreature = getLinkedCreature().get();

	if (linkedCreature != nullptr && linkedCreature->getParent() == asCreatureObject()) {
#if DEBUG_COV
		linkedCreature->info("proxy notifyPositionUpdate(" + String::valueOf(entry->getObjectID()) + ")");
#endif // DEBUG_COV
		if (linkedCreature->getCloseObjects() != nullptr)
			linkedCreature->addInRangeObject(entry);

		if (entry->getCloseObjects() != nullptr)
			entry->addInRangeObject(linkedCreature);
	}

	TangibleObjectImplementation::notifyPositionUpdate(entry);
}

int CreatureObjectImplementation::notifyObjectInserted(SceneObject* object) {
	if (object->isWeaponObject())
		setWeapon( cast<WeaponObject*> (object));

	return TangibleObjectImplementation::notifyObjectInserted(object);
}

int CreatureObjectImplementation::notifyObjectRemoved(SceneObject* object) {
	if (object->isWeaponObject())
		setWeapon( nullptr);

	return TangibleObjectImplementation::notifyObjectInserted(object);
}

void CreatureObjectImplementation::setCreatureLink(CreatureObject* object,
		bool notifyClient) {
	if (linkedCreature == object)
		return;

	linkedCreature = object;

	if (!notifyClient)
		return;

	CreatureObjectDeltaMessage3* delta = new CreatureObjectDeltaMessage3(asCreatureObject());
	delta->updateCreatureLinkID();
	delta->close();

	broadcastMessage(delta, true);
}

void CreatureObjectImplementation::executeObjectControllerAction(
		unsigned int actionCRC) {
	ManagedReference<ObjectController*> objectController =
			getZoneServer()->getObjectController();

	objectController->activateCommand(asCreatureObject(), actionCRC, 0, 0, "");
}

void CreatureObjectImplementation::executeObjectControllerAction(
		unsigned int actionCRC, uint64 targetID, const UnicodeString& args) {
	ManagedReference<ObjectController*> objectController =
			getZoneServer()->getObjectController();

	objectController->activateCommand(asCreatureObject(), actionCRC, 0, targetID, args);
}

void CreatureObjectImplementation::doCombatAnimation(TangibleObject* defender,
		uint32 animcrc, byte hit, byte trails, uint64 weaponID) {

	CombatAction* action = new CombatAction(asCreatureObject(), defender, animcrc, hit, trails, weaponID);
	broadcastMessage(action, true);
}

void CreatureObjectImplementation::doCombatAnimation(unsigned int animationCRC) {
	CombatAction* action = new CombatAction(asCreatureObject(), animationCRC);
	broadcastMessage(action, true);
}

void CreatureObjectImplementation::doAnimation(const String& anim) {
	Animation* msg = new Animation(asCreatureObject(), anim);

	broadcastMessage(msg, true);
}

void CreatureObjectImplementation::dismount() {
	executeObjectControllerAction(STRING_HASHCODE("dismount"));
}

float CreatureObjectImplementation::calculateBFRatio() const {
	if (shockWounds <= 250)
		return 1.f;

	float bfRatio = (1250.f - (float)shockWounds) / 1000.f;

	if (bfRatio > 1.f)
		bfRatio = 1.f;
	else if (bfRatio < 0.25f)
		bfRatio = 0.25f;

	return bfRatio;
}

void CreatureObjectImplementation::removeFeignedDeath() {
	ManagedReference<CreatureObject*> creo = asCreatureObject();

	clearState(CreatureState::FEIGNDEATH); // This must always be done before setting the player upright
	removeBuff(STRING_HASHCODE("private_feign_buff"));
	removeBuff(STRING_HASHCODE("private_feign_damage_buff"));
}

void CreatureObjectImplementation::setFeignedDeathState() {
	setState(CreatureState::FEIGNDEATH);
}

bool CreatureObjectImplementation::canFeignDeath() {
	if (isKneeling())
		return false;

	if (getHAM(CreatureAttribute::HEALTH) <= 100 && getHAM(CreatureAttribute::ACTION) <= 100 && getHAM(CreatureAttribute::MIND) <= 100)
		return false;

	const int maxDefenders = 5;

	int defenderCount = getDefenderList()->size();
	int skillMod = getSkillMod("feign_death");

	if (defenderCount > maxDefenders)
		defenderCount = maxDefenders;

	for (int i = 0; i < defenderCount; i++) {
		if (System::random(100) > skillMod)
			return false;
	}

	return true;
}

void CreatureObjectImplementation::feignDeath() {
	ManagedReference<CreatureObject*> creo = asCreatureObject();

	creo->setCountdownTimer(5);
	creo->updateCooldownTimer("command_message", 5 * 1000);
	creo->setFeignedDeathState();
	creo->setPosture(CreaturePosture::INCAPACITATED, false, false);

	PrivateSkillMultiplierBuff *buff = new PrivateSkillMultiplierBuff(creo, STRING_HASHCODE("private_feign_damage_buff"), std::numeric_limits<float>::max(), BuffType::OTHER);

	Locker blocker(buff, creo);
	buff->setSkillModifier("private_damage_divisor", 4);
	buff->setSkillModifier("private_damage_multiplier", 5);
	creo->addBuff(buff);

	// We need to delay the forcePeace until after the CombatAction is executed
	Core::getTaskManager()->scheduleTask([creo] {
		Locker lock(creo);
		CombatManager::instance()->forcePeace(creo);
	}, "FeignDeathForcePeaceTask", 250);
}

void CreatureObjectImplementation::setDizziedState(int durationSeconds) {
	uint32 buffCRC = Long::hashCode(CreatureState::DIZZY);

	if (hasBuff(buffCRC)) {
		Reference<CreatureObject*> creo = asCreatureObject();
		creo->renewBuff(buffCRC, durationSeconds);
	} else {
		Reference<StateBuff*> state = new StateBuff(asCreatureObject(), CreatureState::DIZZY, durationSeconds);
		Locker locker(state);

		state->setStartFlyText("combat_effects", "go_dizzy", 0, 0xFF, 0);
		state->setEndFlyText("combat_effects", "no_dizzy", 0xFF, 0, 0);

		addBuff(state);
	}
}

void CreatureObjectImplementation::setAimingState(int durationSeconds) {
	if (!hasState(CreatureState::AIMING)) {
		StateBuff* aiming = new StateBuff(asCreatureObject(), CreatureState::AIMING, durationSeconds);

		int aimMods = 0;
		const auto creatureAimMods = weapon->getCreatureAimModifiers();

		for (int i = 0; i < creatureAimMods->size(); ++i) {
			aimMods += getSkillMod(creatureAimMods->get(i));
		}

		Locker locker(aiming);

		aiming->setSkillModifier("private_aim", aimMods);

		addBuff(aiming);
	}
}

void CreatureObjectImplementation::setRalliedState(int durationSeconds) {
	if (!hasState(CreatureState::RALLIED)) {
		Reference<StateBuff*> state = new StateBuff(asCreatureObject(), CreatureState::RALLIED, durationSeconds);

		Locker locker(state);

		state->setStartFlyText("combat_effects", "go_rally", 0, 0xFF, 0);
		state->setEndFlyText("combat_effects", "no_rally", 0xFF, 0, 0);

		addBuff(state);
	}
}

void CreatureObjectImplementation::setCoverState(int durationSeconds) {
	setPosture(CreaturePosture::PRONE);

	if (!hasState(CreatureState::COVER)) {
		Reference<StateBuff*> buff = new StateBuff(asCreatureObject(), CreatureState::COVER, durationSeconds);

		Locker locker(buff);

		buff->setStartFlyText("combat_effects", "go_cover", 0, 0xFF, 0);
		buff->setEndFlyText("combat_effects", "no_cover", 0xFF, 0, 0);

		buff->setSkillModifier("private_defense", 10);
		buff->setSkillModifier("ranged_defense", 25);

		addBuff(buff);

		// Update speed after buff has been applied.
		updateSpeedAndAccelerationMods();
	}
}

void CreatureObjectImplementation::setBerserkedState(uint32 duration) {
	if (!hasState(CreatureState::BERSERK)) {
		Reference<StateBuff*> state = new StateBuff(asCreatureObject(), CreatureState::BERSERK, duration);

		Locker locker(state);

		state->setStartFlyText("combat_effects", "go_berserk", 0, 0xFF, 0);
		state->setEndFlyText("combat_effects", "no_berserk", 0xFF, 0, 0);

		int mod = getSkillMod("berserk");

		state->setSkillModifier("private_melee_accuracy_bonus", 20 + mod);
		state->setSkillModifier("private_melee_damage_bonus", 75 + mod);
		state->setSkillModifier("private_melee_defense", -60);
		state->setSkillModifier("private_ranged_defense", -60);

		addBuff(state);
	}
}

void CreatureObjectImplementation::setStunnedState(int durationSeconds) {
	uint32 buffCRC = Long::hashCode(CreatureState::STUNNED);

	if (hasBuff(buffCRC)) {
		Reference<CreatureObject*> creo = asCreatureObject();
		creo->renewBuff(buffCRC, durationSeconds);
	} else {
		Reference<StateBuff*> state = new StateBuff(asCreatureObject(), CreatureState::STUNNED, durationSeconds, STRING_HASHCODE("private_stun_multiplier"));

		Locker locker(state);

		state->setStartFlyText("combat_effects", "go_stunned", 0, 0xFF, 0);
		state->setEndFlyText("combat_effects", "no_stunned", 0xFF, 0, 0);
		state->setSkillModifier("private_melee_defense", -50);
		state->setSkillModifier("private_ranged_defense", -50);

		addBuff(state);

		Reference<PrivateSkillMultiplierBuff*> multBuff = new PrivateSkillMultiplierBuff(asCreatureObject(), STRING_HASHCODE("private_stun_multiplier"), durationSeconds, BuffType::STATE);

		Locker blocker(multBuff);

		multBuff->setSkillModifier("private_damage_divisor", 5);
		multBuff->setSkillModifier("private_damage_multiplier", 4);

		addBuff(multBuff);
	}
}

void CreatureObjectImplementation::setBlindedState(int durationSeconds) {
	uint32 buffCRC = Long::hashCode(CreatureState::BLINDED);

	if (hasBuff(buffCRC)) {
		Reference<CreatureObject*> creo = asCreatureObject();
		creo->renewBuff(buffCRC, durationSeconds);
	} else {
		Reference<StateBuff*> state = new StateBuff(asCreatureObject(), CreatureState::BLINDED, durationSeconds);

		Locker locker(state);

		state->setStartFlyText("combat_effects", "go_blind", 0, 0xFF, 0);
		state->setEndFlyText("combat_effects", "no_blind", 0xFF, 0, 0);

		state->setSkillModifier("private_attack_accuracy", -60);
		state->setSkillModifier("private_dodge_attack", -60);

		addBuff(state);
	}
}

void CreatureObjectImplementation::setIntimidatedState(int durationSeconds) {
	uint32 buffCRC = Long::hashCode(CreatureState::INTIMIDATED);

	if(hasBuff(buffCRC)) {
		Reference<CreatureObject*> creo = asCreatureObject();

		showFlyText("combat_effects", "go_intimidated", 0, 0xFF, 0);

		creo->playEffect("clienteffect/combat_special_defender_intimidate.cef");

		creo->renewBuff(buffCRC, durationSeconds);
	} else {
		ManagedReference<StateBuff*> state = new StateBuff(asCreatureObject(), CreatureState::INTIMIDATED, durationSeconds, STRING_HASHCODE("private_intimidate_multiplier"));

		Locker locker(state);

		state->setStartFlyText("combat_effects", "go_intimidated", 0, 0xFF, 0);
		state->setEndFlyText("combat_effects", "no_intimidated", 0xFF, 0, 0);

		state->setSkillModifier("private_melee_defense", -20);
		state->setSkillModifier("private_ranged_defense", -20);

		addBuff(state);

		if (!hasSkill("combat_bountyhunter_master")){
			Reference<PrivateSkillMultiplierBuff*> multBuff = new PrivateSkillMultiplierBuff(asCreatureObject(), STRING_HASHCODE("private_intimidate_multiplier"), durationSeconds, BuffType::STATE);

			Locker blocker(multBuff);

			multBuff->setSkillModifier("private_damage_divisor", 2);
	
			addBuff(multBuff);
		}
	}
}

void CreatureObjectImplementation::setSnaredState(int durationSeconds) {
	if (!hasState(CreatureState::IMMOBILIZED)) {
		Reference<StateBuff*> state = new StateBuff(asCreatureObject(), CreatureState::IMMOBILIZED, durationSeconds);

		Locker locker(state);

		state->setStartFlyText("combat_effects", "go_snare", 0, 0xFF, 0);
		state->setEndFlyText("combat_effects", "no_snare", 0xFF, 0, 0);

		addBuff(state);
	}
}

void CreatureObjectImplementation::setRootedState(int durationSeconds) {
	if (!hasState(CreatureState::FROZEN)) {
		Reference<StateBuff*> state = new StateBuff(asCreatureObject(), CreatureState::FROZEN, durationSeconds);

		Locker locker(state);

		state->setStartFlyText("combat_effects", "go_rooted", 0, 0xFF, 0);
		state->setEndFlyText("combat_effects", "no_rooted", 0xFF, 0, 0);

		addBuff(state);
	}
}

bool CreatureObjectImplementation::setNextAttackDelay(uint32 mod, int del) {
	if (cooldownTimerMap->isPast("nextAttackDelayRecovery")) {
		//del += mod;
		cooldownTimerMap->updateToCurrentAndAddMili("nextAttackDelay", del * 1000);
		cooldownTimerMap->updateToCurrentAndAddMili("nextAttackDelayRecovery", 30000 + (del * 1000));

		showFlyText("combat_effects", "warcry_hit", 0x00, 0xFF, 0x00);

		if (isPlayerCreature()) {
			StringIdChatParameter stringId("combat_effects", "delay_applied_self");
			stringId.setDI(del);
			sendSystemMessage(stringId);
		}

		return true;
	}

	return false;
}

void CreatureObjectImplementation::setMeditateState() {
	if (isInCombat())
		return;

	setMoodString("meditating");
	setPosture(CreaturePosture::SITTING);
	setState(CreatureState::ALERT);
}

void CreatureObjectImplementation::queueDizzyFallEvent() {
	if (hasDizzyEvent())
		return;

	dizzyFallDownEvent = new DizzyFallDownEvent(asCreatureObject());
	dizzyFallDownEvent->schedule(200);
}

void CreatureObjectImplementation::activateStateRecovery() {
	if (damageOverTimeList.hasDot() && damageOverTimeList.isNextTickPast()) {
		damageOverTimeList.activateDots(asCreatureObject());
	}

	// clear any stuck states
	if (isBleeding() && !damageOverTimeList.hasDot(CreatureState::BLEEDING))
		clearState(CreatureState::BLEEDING);
	if (isPoisoned() && !damageOverTimeList.hasDot(CreatureState::POISONED))
		clearState(CreatureState::POISONED);
	if (isDiseased() && !damageOverTimeList.hasDot(CreatureState::DISEASED))
		clearState(CreatureState::DISEASED);
	if (isOnFire() && !damageOverTimeList.hasDot(CreatureState::ONFIRE))
		clearState(CreatureState::ONFIRE);
}

void CreatureObjectImplementation::updateToDatabaseAllObjects(bool startTask) {
	if (!isPersistent())
		return;

	creatureBuffs.updateBuffsToDatabase();

	TangibleObjectImplementation::updateToDatabaseAllObjects(startTask);
}

void CreatureObjectImplementation::addBuff(Buff* buff) {
	if (buff == nullptr)
		return;

	uint32 buffcrc = buff->getBuffCRC();

	creatureBuffs.addBuff(buff);
}

void CreatureObjectImplementation::renewBuff(uint32 buffCRC, int duration, bool sendToClient) {
	Reference<CreatureObject*> creo = asCreatureObject();
	Reference<Buff*> buff = getBuff(buffCRC);

	if (buff != nullptr) {

		Locker blocker(buff, creo);

		if (buff->getTimeLeft() < duration) {
			buff->renew(duration);

			if(sendToClient)
				buff->sendTo(creo);
		}

		const Vector<unsigned long long>* secondaryCRCs = buff->getSecondaryBuffCRCs();
		blocker.release();

		for(int i = 0; i < secondaryCRCs->size(); i++) {
			creo->renewBuff(secondaryCRCs->get(i), duration, sendToClient);
		}
	}
}

bool CreatureObjectImplementation::removeBuff(uint32 buffcrc) {
	Reference<Buff*> buff = getBuff(buffcrc);

	//BuffList::removeBuff checks to see if the buffcrc exists in the map.
	bool ret = creatureBuffs.removeBuff(buffcrc);

	if (buff != nullptr) {
		const Vector<unsigned long long>* secondaryCRCs = buff->getSecondaryBuffCRCs();

		for (int i = 0; i < secondaryCRCs->size(); i++) {
			removeBuff(secondaryCRCs->get(i));
		}
	}

	return ret;
}

bool CreatureObjectImplementation::removeStateBuff(uint64 state) {
	bool ret = removeBuff(Long::hashCode(state));

	fatal(!hasState(state), "state was not removed after removeBuff");

	return ret;
}

void CreatureObjectImplementation::removeBuff(Buff* buff) {
	if (buff == nullptr)
		return;

	uint32 buffcrc = buff->getBuffCRC();

	//BuffList::removeBuff checks to see if the buffcrc exists in the map.
	creatureBuffs.removeBuff(buff);
}

void CreatureObjectImplementation::clearBuffs(bool updateclient, bool removeAll) {
	creatureBuffs.clearBuffs(updateclient, removeAll);
}

void CreatureObjectImplementation::notifyPostureChange(int newPosture) {
	if (hasState(CreatureState::ALERT)) {
		// Following fixes beings 'stuck' in meditate after a server restart, by re-registering the observers if the player has the Alert (meditating) state.
		SortedVector<ManagedReference<Observer* > > observers = getObservers(ObserverEventType::POSITIONCHANGED);

		if (observers.size() == 0) {
			PlayerManager* playermgr = server->getZoneServer()->getPlayerManager();
			registerObserver(ObserverEventType::POSTURECHANGED, playermgr);
		}
	}

	notifyObservers(ObserverEventType::POSTURECHANGED, nullptr, newPosture);
}

void CreatureObjectImplementation::updateGroupMFDPositions() {
	Reference<CreatureObject*> creo = asCreatureObject();
	auto group = this->group;

	if (group != nullptr) {
		GroupList* list = group->getGroupList();
		if (list != nullptr) {
			auto zone = getZone();

			if (zone == nullptr) {
				return;
			}

			ClientMfdStatusUpdateMessage* msg = new ClientMfdStatusUpdateMessage(creo, zone->getZoneName());

#ifdef LOCKFREE_BCLIENT_BUFFERS
			Reference<BasePacket*> pack = msg;
#endif

			for (int i = 0; i < list->size(); i++) {

				Reference<CreatureObject*> member = list->getSafe(i).get();

				if (member == nullptr || creo == member || !member->isPlayerCreature())
					continue;

				CloseObjectsVector* cev = (CloseObjectsVector*)member->getCloseObjects();

				if (cev == nullptr || cev->contains(creo.get()))
					continue;

#ifdef LOCKFREE_BCLIENT_BUFFERS
				member->sendMessage(pack);
#else
				member->sendMessage(msg->clone());
#endif
			}

#ifndef LOCKFREE_BCLIENT_BUFFERS
			delete msg;
#endif
		}
	}
}

void CreatureObjectImplementation::notifySelfPositionUpdate() {
	auto zone = getZoneUnsafe();

	if (zone != nullptr && hasState(CreatureState::ONFIRE)) {
		PlanetManager* planetManager =
				zone->getPlanetManager();

		if (planetManager != nullptr) {
			TerrainManager* terrainManager = planetManager->getTerrainManager();

			if (terrainManager != nullptr) {
				float waterHeight;

				CreatureObject* creature = asCreatureObject();

				if (parent == nullptr && terrainManager->getWaterHeight(getPositionX(), getPositionY(), waterHeight)) {
					if ((getPositionZ() + getSwimHeight() - waterHeight < 0.2)) {
						Reference<CreatureObject*> strongRef = asCreatureObject();

						Core::getTaskManager()->executeTask([strongRef] () {
							Locker locker(strongRef);

							if (strongRef->hasState(CreatureState::ONFIRE))
								strongRef->healDot(CreatureState::ONFIRE, 100);
						}, "CreoPositionUpdateHealFireLambda");
					}
				}
			}
		}
	}

	if (cooldownTimerMap->isPast("groupMFDUpdate")) {
		cooldownTimerMap->updateToCurrentAndAddMili("groupMFDUpdate", 2000);

		updateGroupMFDPositions();
	}

	updateLocomotion();

	TangibleObjectImplementation::notifySelfPositionUpdate();
}

void CreatureObjectImplementation::activateHAMRegeneration(int latency) {
	if (isIncapacitated() || isDead())
		return;

	if (!isPlayerCreature() && isInCombat())
		return;

	float modifier = (float)latency/1000.f;

	if (isKneeling())
		modifier *= 1.25f;
	else if (isSitting())
		modifier *= 1.75f;

	// this formula gives the amount of regen per second
	uint32 healthTick = (uint32) ceil((float) Math::max(0, getHAM(
			CreatureAttribute::CONSTITUTION)) * 13.0f / 2100.0f * modifier);
	uint32 actionTick = (uint32) ceil((float) Math::max(0, getHAM(
			CreatureAttribute::STAMINA)) * 13.0f / 2100.0f * modifier);
	uint32 mindTick = (uint32) ceil((float) Math::max(0, getHAM(
			CreatureAttribute::WILLPOWER)) * 13.0f / 2100.0f * modifier);

	if (healthTick < 1)
		healthTick = 1;

	if (actionTick < 1)
		actionTick = 1;

	if (mindTick < 1)
		mindTick = 1;

	healDamage(asCreatureObject(), CreatureAttribute::HEALTH, healthTick, true, false);
	healDamage(asCreatureObject(), CreatureAttribute::ACTION, actionTick, true, false);
	healDamage(asCreatureObject(), CreatureAttribute::MIND, mindTick, true, false);

	activatePassiveWoundRegeneration();
}

void CreatureObjectImplementation::activatePassiveWoundRegeneration() {
	/// Health wound regen
	int healthRegen = getSkillMod("private_med_wound_health");

	if(healthRegen > 0) {
		healthWoundHeal += (int)(healthRegen * 0.2);
		if(healthWoundHeal >= 100) {
			healWound(asCreatureObject(), CreatureAttribute::HEALTH, 1, true, false);
			healWound(asCreatureObject(), CreatureAttribute::STRENGTH, 1, true, false);
			healWound(asCreatureObject(), CreatureAttribute::CONSTITUTION, 1, true, false);
			healthWoundHeal -= 100;
		}
	}

	/// Action wound regen
	int actionRegen = getSkillMod("private_med_wound_action");

	if(actionRegen > 0) {
		actionWoundHeal += (int)(actionRegen * 0.2);
		if(actionWoundHeal >= 100) {
			healWound(asCreatureObject(), CreatureAttribute::ACTION, 1, true, false);
			healWound(asCreatureObject(), CreatureAttribute::QUICKNESS, 1, true, false);
			healWound(asCreatureObject(), CreatureAttribute::STAMINA, 1, true, false);
			actionWoundHeal -= 100;
		}
	}

	/// Mind wound regen
	int mindRegen = getSkillMod("private_med_wound_mind");

	if(mindRegen > 0) {
		mindWoundHeal += (int)(mindRegen * 0.2);
		if(mindWoundHeal >= 100) {
			healWound(asCreatureObject(), CreatureAttribute::MIND, 1, true, false);
			healWound(asCreatureObject(), CreatureAttribute::FOCUS, 1, true, false);
			healWound(asCreatureObject(), CreatureAttribute::WILLPOWER, 1, true, false);
			mindWoundHeal -= 100;
		}
	}
}

bool CreatureObjectImplementation::isResuscitable() {
	return (isDead() && (timeOfDeath.miliDifference()) < DEAD_TOO_LONG);
}

bool CreatureObjectImplementation::isDancing() {
	ManagedReference<Facade*> facade = this->getActiveSession(
			SessionFacadeType::ENTERTAINING);

	if (facade == nullptr)
		return false;

	EntertainingSession* session =
			dynamic_cast<EntertainingSession*> (facade.get());

	if (session == nullptr)
		return false;

	return session->isDancing();
}

bool CreatureObjectImplementation::isPlayingMusic() {
	ManagedReference<Facade*> facade = this->getActiveSession(
			SessionFacadeType::ENTERTAINING);

	if (facade == nullptr)
		return false;

	EntertainingSession* session =
			dynamic_cast<EntertainingSession*> (facade.get());

	if (session == nullptr)
		return false;

	return session->isPlayingMusic();
}

void CreatureObjectImplementation::stopEntertaining() {
	ManagedReference<Facade*> facade = this->getActiveSession(
			SessionFacadeType::ENTERTAINING);

	if (facade == nullptr)
		return;

	EntertainingSession* session =
			dynamic_cast<EntertainingSession*> (facade.get());

	if (session == nullptr)
		return;

	session->cancelSession();
}

void CreatureObjectImplementation::sendMessage(BasePacket* msg) {
	ManagedReference<ZoneClientSession*> ownerClient = owner.get();

	if (ownerClient == nullptr) {
#ifdef LOCKFREE_BCLIENT_BUFFERS
		if (!msg->getReferenceCount())
#endif
		delete msg;

		return;
	} else {
		debug() << "sending " << *msg;

		ownerClient->sendMessage(msg);
	}
}

Reference<ZoneClientSession*> CreatureObjectImplementation::getClient() {
	return owner.WeakReference::get();
}

void CreatureObjectImplementation::sendStateCombatSpam(const String& fileName, const String& stringName, byte color, int damage, bool broadcast) {
	Zone* zone = getZoneUnsafe();
	if (zone == nullptr)
		return;

	if (isDead()) //We don't need to know when a corpse can see clearly again!
		return;

	auto creature = asCreatureObject();

	if (broadcast) { //Send spam to all nearby players.
		CombatManager::instance()->broadcastCombatSpam(creature, nullptr, nullptr, 0, fileName, stringName, color);

	} else { //Send spam only to originating player.
		if (!creature->isPlayerCreature())
			return;

		CombatSpam* spam = new CombatSpam(creature, nullptr, creature, nullptr, damage, fileName, stringName, color);
		creature->sendMessage(spam);
	}
}

void CreatureObjectImplementation::sendCustomCombatSpam(const UnicodeString& customString, byte color) {
	if (!this->isPlayerCreature())
			return;
	CombatSpam* spam = new CombatSpam(asCreatureObject(), customString, color);
	sendMessage(spam);
}

String CreatureObjectImplementation::getFirstName() const {
	UnicodeString fullName = getCustomObjectName();

	int idx = fullName.indexOf(' ');

	if (idx != -1) {
		return fullName.subString(0, idx).toString();
	} else {
		return fullName.toString();
	}
}

String CreatureObjectImplementation::setFirstName(const String& newFirstName) {
	if (!isPlayerCreature())
		return "Can only set FirstName on players.";

	return getZoneServer()->getPlayerManager()->setFirstName(asCreatureObject(), newFirstName);
}

String CreatureObjectImplementation::getLastName() const {
	UnicodeString lastName;

	UnicodeString fullName = getCustomObjectName();
	UnicodeTokenizer tokenizer(fullName);

	if (tokenizer.hasMoreTokens())
		tokenizer.shiftTokens(1);

	if (tokenizer.hasMoreTokens())
		tokenizer.getUnicodeToken(lastName);

	return lastName.toString();
}

String CreatureObjectImplementation::setLastName(const String& newLastName, bool skipVerify) {
	if (!isPlayerCreature())
		return "Can only set LastName on players.";

	return getZoneServer()->getPlayerManager()->setLastName(asCreatureObject(), newLastName, skipVerify);
}

String CreatureObjectImplementation::setLastName(const String& newLastName) {
	return setLastName(newLastName, false);
}

void CreatureObjectImplementation::sendExecuteConsoleCommand(
		const String& command) {
	BaseMessage* msg = new ExecuteConsoleCommand(command);
	sendMessage(msg);
}

Reference<PlayerObject*> CreatureObjectImplementation::getPlayerObject() {
	return getSlottedObject("ghost").castTo<PlayerObject*> ();
}

bool CreatureObjectImplementation::isAggressiveTo(CreatureObject* object) {
	PlayerObject* ghost = getPlayerObject();
	PlayerObject* targetGhost = object->getPlayerObject();

	if (ghost == nullptr || targetGhost == nullptr)
		return false;

	if (ghost->isOnLoadScreen())
		return false;

	if (hasPersonalEnemyFlag(object) && object->hasPersonalEnemyFlag(asCreatureObject()))
		return true;

	if (ConfigManager::instance()->getPvpMode() && isPlayerCreature())
		return true;

	if (CombatManager::instance()->areInDuel(object, asCreatureObject()))
		return true;

	if ((pvpStatusBitmask & CreatureFlag::OVERT) && (object->getPvpStatusBitmask() & CreatureFlag::OVERT) && object->getFaction() != getFaction())
		return true;

	if (ghost->hasBhTef() && (hasBountyMissionFor(object) || object->hasBountyMissionFor(asCreatureObject()))) {
		return true;
	}

	if (ghost->hasJediTef())
		return true;

	if (object->getPvpStatusBitmask() & CreatureFlag::TEF && getFaction() != object-> getFaction()){
		return true;
	}

	ManagedReference<GuildObject*> guildObject = guild.get();
	if (guildObject != nullptr && guildObject->isInWaringGuild(object))
		return true;

	return false;
}

bool CreatureObjectImplementation::isAttackableBy(TangibleObject* object) {
	auto creo = object->asCreatureObject();

	if (creo != nullptr)
		return isAttackableBy(creo);

	return isAttackableBy(object, false);
}

bool CreatureObjectImplementation::isAttackableBy(TangibleObject* object, bool bypassDeadCheck) {
	PlayerObject* ghost = getPlayerObject();

	if(ghost == nullptr)
		return false;

	if (ghost->isOnLoadScreen())
		return false;

	if ((!bypassDeadCheck && (isDead() || (isIncapacitated() && !isFeigningDeath()))) || isInvisible())
		return false;

	if (getPvpStatusBitmask() == CreatureFlag::NONE)
		return false;

	if(object->getFaction() == 0 )
		return true;

	if(object->getFaction() == getFaction())
		return false;

	// if tano is overt, creature must be overt
	//if((object->getPvpStatusBitmask() & CreatureFlag::OVERT) && !(getPvpStatusBitmask() & CreatureFlag::OVERT))
		//return false;
	if((getFactionStatus() == FactionStatus::COVERT && !(getPvpStatusBitmask() & CreatureFlag::TEF)) && object->getFaction() != 0)
		return false;
	// the other options are overt creature / overt tano  and covert/covert, covert tano, overt creature..  all are attackable
	return true;

}

bool CreatureObjectImplementation::isAttackableBy(CreatureObject* object) {
	return isAttackableBy(object, false);
}

bool CreatureObjectImplementation::isAttackableBy(CreatureObject* object, bool bypassDeadCheck) {
	if (object == asCreatureObject())
		return false;

	if ((!bypassDeadCheck && isDead()) || isInvisible())
		return false;

	if (object->getZoneUnsafe() != getZoneUnsafe())
		return false;

	if (isPlayerCreature()) {
		PlayerObject* ghost = getPlayerObject();
		if (ghost != nullptr) {
			if (ghost->isOnLoadScreen())
				return false;
			if (ConfigManager::instance()->getPvpMode())
				return true;

		}
	}

	if (object->isAiAgent()) {

		if (object->isPet()) {
			ManagedReference<PetControlDevice*> pcd = object->getControlDevice().get().castTo<PetControlDevice*>();
			if (pcd != nullptr && pcd->getPetType() == PetManager::FACTIONPET && isNeutral()) {
				return false;
			}

			ManagedReference<CreatureObject*> owner = object->getLinkedCreature().get();

			if (owner == nullptr)
				return false;

			return isAttackableBy(owner);
		}

		if(!object->isRebel() && !object->isImperial())
			return true;

		if(getFaction() == 0 || getFaction() == object->getFaction())
			return false;
		else if (isPlayerCreature() && getFactionStatus() == FactionStatus::ONLEAVE)
			return false;

		return true;
	}

	PlayerObject* ghost = getPlayerObject();
	PlayerObject* targetGhost = object->getPlayerObject();

	if (ghost == nullptr || targetGhost == nullptr)
		return false;

	if (hasPersonalEnemyFlag(object) && object->hasPersonalEnemyFlag(asCreatureObject()))
		return true;

	bool areInDuel = (ghost->requestedDuelTo(object) && targetGhost->requestedDuelTo(asCreatureObject()));

	if (areInDuel)
		return true;

	if (object->hasBountyMissionFor(asCreatureObject()) || (ghost->hasBhTef() && hasBountyMissionFor(object)))
		return true;

	if (getGroupID() != 0 && getGroupID() == object->getGroupID())
		return false;

	if ((pvpStatusBitmask & CreatureFlag::OVERT) && (object->getPvpStatusBitmask() & CreatureFlag::OVERT) && object->getFaction() != getFaction())
		return true;

	if ((pvpStatusBitmask & CreatureFlag::TEF) && (object->getFaction() != getFaction()) && (object->getFaction() != 0) && object->getPvpStatusBitmask() & CreatureFlag::OVERT) {
		return true;
	}

	if ((pvpStatusBitmask & CreatureFlag::OVERT && ghost->hasPvpTef()) && (object->getFaction() != getFaction()) && (object->getFaction() !=0) && targetGhost->hasPvpTef()){
		return true;
	}

	// if (ghost->hasJediTef() || (ghost->isJedi() && getWeapon()->isJediWeapon()) || (ghost->isJediAttackable())){
	// 	return true;
	// }

	ManagedReference<GuildObject*> guildObject = guild.get();
	if (guildObject != nullptr && guildObject->isInWaringGuild(object))
		return true;

	return false;
}

bool CreatureObjectImplementation::isHealableBy(CreatureObject* object) {
	if (object == asCreatureObject())
		return true;

	if (isInvisible())
		return false;

	if (isAttackableBy(object))
		return false;

	PlayerObject* ghost = object->getPlayerObject(); // ghost is the healer

	if (ghost == nullptr)
		return false;
	//Comment out BH TEF check for healing
	//if (ghost->hasBhTef())
	//	return false;

	//if ((pvpStatusBitmask & CreatureFlag::OVERT) && (object->getPvpStatusBitmask() & CreatureFlag::OVERT) && object->getFaction() != getFaction())

	CreatureObject* targetCreo = asCreatureObject();

	if (isPet()) {
		auto linkedCreature = getLinkedCreature().get();
		if (object == linkedCreature)
			return true;

		if (linkedCreature != nullptr) {
			targetCreo = linkedCreature.get();
			//if (targetCreo->getFaction() == 0 || targetCreo->getFaction() == ghost->getFaction())
		}
	}

	uint32 targetFactionStatus = targetCreo->getFactionStatus();
	uint32 currentFactionStatus = object->getFactionStatus();
	PlayerObject* playerGhost = getPlayerObject();
	if (playerGhost != NULL){
		if (getFaction() != object->getFaction() && ((ghost->hasPvpTef() || playerGhost->hasPvpTef() )))
			return false;

		if (currentFactionStatus == FactionStatus::OVERT && (object->getFaction() == 0 || object->getFaction() != getFaction()))
			return false;

		if (playerGhost->hasPvpTef() && (object->getFaction() == 0 || object->getFaction() != getFaction()))
			return false;
	}

	return true;
}

bool CreatureObjectImplementation::isInvulnerable()  {
	return isPlayerCreature() && (getPvpStatusBitmask() & CreatureFlag::PLAYER) == 0;
}

bool CreatureObjectImplementation::hasBountyMissionFor(CreatureObject* target) {
	if (target == nullptr)
		return false;

	ZoneServer* zoneServer = asCreatureObject()->getZoneServer();

	if (zoneServer == nullptr)
		return false;

	MissionManager* missionManager = zoneServer->getMissionManager();

	if (missionManager == nullptr)
		return false;

	ManagedReference<MissionObject*> mission = missionManager->getBountyHunterMission(asCreatureObject());

	if (mission == nullptr)
		return false;

	return mission->getTargetObjectId() == target->getObjectID();
}

int CreatureObjectImplementation::notifyObjectDestructionObservers(TangibleObject* attacker, int condition, bool isCombatAction) {
	PlayerObject* ghost = getPlayerObject();

	if (ghost != nullptr) {
		PlayerManager* playerManager = getZoneServer()->getPlayerManager();

		playerManager->notifyDestruction(attacker, asCreatureObject(), condition, isCombatAction);
	}

	if (attacker->isAiAgent()) {
		AiAgent* aiAgent = attacker->asAiAgent();
		aiAgent->sendReactionChat(ReactionManager::GLOAT);
	}

	return TangibleObjectImplementation::notifyObjectDestructionObservers(attacker, condition, isCombatAction);
}

int CreatureObjectImplementation::notifyObjectKillObservers(TangibleObject* killer) {
	notifyObservers(ObserverEventType::PLAYERKILLED, killer, 0);

	return 0;
}

void CreatureObjectImplementation::createChildObjects() {
	ZoneServer* zoneServer = server->getZoneServer();

	for (int i = 0; i < templateObject->getChildObjectsSize(); ++i) {
		const ChildObject* child = templateObject->getChildObject(i);

		if (child == nullptr)
			continue;

		ManagedReference<SceneObject*> obj = zoneServer->createObject(
				child->getTemplateFile().hashCode(), getPersistenceLevel());

		if (obj == nullptr)
			continue;

		ContainerPermissions* permissions = obj->getContainerPermissionsForUpdate();
		permissions->setOwner(getObjectID());
		permissions->setInheritPermissionsFromParent(false);
		permissions->setDefaultDenyPermission(ContainerPermissions::MOVECONTAINER);
		permissions->setDenyPermission("owner", ContainerPermissions::MOVECONTAINER);

		if (!transferObject(obj, child->getContainmentType())) {
			obj->destroyObjectFromDatabase(true);
			continue;
		}

		childObjects.put(obj);

		obj->initializeChildObject(asCreatureObject());
	}
}

void CreatureObjectImplementation::addWearableObject(TangibleObject* object, bool notifyClient) {
	if (wearablesVector.contains(object))
		return;

	if (notifyClient) {
		CreatureObjectDeltaMessage6* msg = new CreatureObjectDeltaMessage6(
				asCreatureObject());
		msg->startUpdate(0x0F);
		wearablesVector.add(object, msg);
		msg->close();

		broadcastMessage(msg, true);
	} else {
		wearablesVector.add(object);
	}
}

void CreatureObjectImplementation::removeWearableObject(TangibleObject* object, bool notifyClient) {
	int index = wearablesVector.find(object);

	if (index == -1)
		return;

	if (notifyClient) {
		CreatureObjectDeltaMessage6* msg = new CreatureObjectDeltaMessage6(
				asCreatureObject());
		msg->startUpdate(0x0F);
		wearablesVector.remove(index, msg);
		msg->close();

		broadcastMessage(msg, true);
	} else {
		wearablesVector.remove(index);
	}
}

CampSiteActiveArea* CreatureObjectImplementation::getCurrentCamp() {

	for(int i = 0; i < activeAreas.size(); ++i) {
		if(activeAreas.get(i)->isCampArea())
			return cast<CampSiteActiveArea*>(activeAreas.get(i).get());
	}
	return nullptr;
}

int CreatureObjectImplementation::handleObjectMenuSelect(CreatureObject* player, byte selectedID) {
	if (isDead() && !isPet()) {
		switch (selectedID) {
		case 35:
			player->executeObjectControllerAction(STRING_HASHCODE("loot"), getObjectID(), "");

			return 0;
		case 36:
			player->executeObjectControllerAction(STRING_HASHCODE("loot"), getObjectID(), "all");

			return 0;
		}
	}

	return TangibleObjectImplementation::handleObjectMenuSelect(player, selectedID);
}

float CreatureObjectImplementation::calculateCostAdjustment(uint8 stat, float baseCost) const {
	float cost = baseCost - (((float)(this->getHAM(stat) - 300) / 1200.f) * baseCost);

	if (cost < 0)
		cost = 0;

	return cost;
}

Reference<WeaponObject*> CreatureObjectImplementation::getWeapon() {
	if (weapon == nullptr) {
		return TangibleObjectImplementation::getSlottedObject("default_weapon").castTo<WeaponObject*>();
	} else
		return weapon;
}

void CreatureObjectImplementation::setFaction(unsigned int crc) {
	faction = crc;

	if (isPlayerCreature()) {
		Reference<CreatureObject*> player = asCreatureObject();

		if (player == nullptr)
			return;

		Zone* currentZone = getZone();

		if (currentZone != nullptr) {
			// Notify nearby active areas of faction change
			SortedVector<ManagedReference<ActiveArea* > > activeAreas;
			currentZone->getInRangeActiveAreas(player->getPositionX(), player->getPositionY(), &activeAreas, true);

			for (int i = 0; i < activeAreas.size(); i++) {
				ActiveArea* area = activeAreas.get(i);

				if (area != nullptr)
					area->notifyEnter(asCreatureObject());
			}
		}

		PlayerObject* ghost = player->getPlayerObject();

		if (ghost == nullptr)
			return;

		Vector<ManagedReference<CreatureObject*> > petsToStore;

		for (int i = 0; i < ghost->getActivePetsSize(); i++) {
			ManagedReference<AiAgent*> pet = ghost->getActivePet(i);

			if (pet == nullptr)
				continue;

			const CreatureTemplate* creatureTemplate = pet->getCreatureTemplate();

			if (creatureTemplate != nullptr) {
				String templateFaction = creatureTemplate->getFaction();

				if (!templateFaction.isEmpty() && (templateFaction.hashCode() != crc)) {
					petsToStore.add(pet.castTo<CreatureObject*>());
					player->sendSystemMessage("You're no longer the right faction for one of your pets, storing...");
					continue;
				}
			}

			Locker crossLocker(pet, player);

			pet->setFaction(crc);
		}

		StoreSpawnedChildrenTask* task = new StoreSpawnedChildrenTask(player, std::move(petsToStore));
		task->execute();
	}

	notifyObservers(ObserverEventType::FACTIONCHANGED);
}

void CreatureObjectImplementation::destroyPlayerCreatureFromDatabase(bool destroyContainedObjects) {
	if (!isPlayerCreature())
		return;

	clearBuffs(false, true);

	if(dataObjectComponent != nullptr) {
		dataObjectComponent->notifyObjectDestroyingFromDatabase();
	}

	ZoneServer* server = getZoneServer();

	server->destroyObjectFromDatabase(getObjectID());

	asCreatureObject()->setPersistent(0);

	if (!destroyContainedObjects)
		return;

	MissionManager* missionManager = server->getMissionManager();
	if (missionManager != nullptr)
		missionManager->removePlayerFromBountyList(getObjectID());

	SortedVector<ManagedReference<SceneObject*> > destroyedObjects;
	destroyedObjects.setNoDuplicateInsertPlan();

	for (int i = 0; i < getSlottedObjectsSize(); ++i) {
		ManagedReference<SceneObject*> object = getSlottedObject(i);

		if (destroyedObjects.put(object) != -1) {
			Locker locker(object);
			object->destroyObjectFromDatabase(true);
		}
	}

	for (int j = 0; j < getContainerObjectsSize(); ++j) {
		ManagedReference<SceneObject*> object = getContainerObject(j);

		if (destroyedObjects.put(object) != -1) {
			Locker locker(object);
			object->destroyObjectFromDatabase(true);
		}
	}

	//Remove all child objects from database
	for (int i = 0; i < childObjects.size(); ++i) {
		ManagedReference<SceneObject*> child = childObjects.get(i);

		if (child == nullptr)
			continue;

		Locker locker(child);

		child->destroyObjectFromDatabase(true);
	}

	GuildManager* guildManager = getZoneServer()->getGuildManager();

	uint64 oid = getObjectID();
	if (guildManager->isSponsoredPlayer(oid)) {
		guildManager->removeSponsoredPlayer(oid);
	}

	if (guildManager->isCreatingGuild(oid)) {
		guildManager->removePendingGuild(oid);
	}

	if (isInGuild()) {
		ManagedReference<GuildObject*> guild = getGuildObject().get();

		Locker clocker(guild, asCreatureObject());

		guild->removeMember(oid);
	}
}

float CreatureObjectImplementation::getTemplateRadius() {
	const SharedCreatureObjectTemplate* creoTempl = templateObject.castToPointer<const SharedCreatureObjectTemplate*>();

	if (creoTempl == nullptr)
		return 0;

	return creoTempl->getCollisionRadius() * getHeight();
}

bool CreatureObjectImplementation::hasEffectImmunity(uint8 effectType) const {
	switch (effectType) {
	case CommandEffect::BLIND:
	case CommandEffect::DIZZY:
	case CommandEffect::INTIMIDATE:
	case CommandEffect::STUN:
	case CommandEffect::NEXTATTACKDELAY:
		if (isDroidSpecies() || const_cast<CreatureObjectImplementation*>(this)->isVehicleObject() || isWalkerSpecies())
			return true;
		break;
	case CommandEffect::KNOCKDOWN:
	case CommandEffect::POSTUREUP:
	case CommandEffect::POSTUREDOWN:
		if (const_cast<CreatureObjectImplementation*>(this)->isVehicleObject() || isWalkerSpecies())
			return true;
		break;
	default:
		return false;
	}

	return false;
}

bool CreatureObjectImplementation::hasDotImmunity(uint32 dotType) const {
	switch (dotType) {
	case CreatureState::POISONED:
	case CreatureState::BLEEDING:
	case CreatureState::DISEASED:
		if (isDroidSpecies() || const_cast<CreatureObjectImplementation*>(this)->isVehicleObject())
			return true;
		break;
	case CreatureState::ONFIRE:
		return false;
	default:
		return false;
	}

	return false;
}

int CreatureObjectImplementation::getSpecies() const {
	const SharedCreatureObjectTemplate* creoData = templateObject.castToPointer<const SharedCreatureObjectTemplate*>();

	if (creoData == nullptr)
		return -1;

	return creoData->getSpecies();
}

int CreatureObjectImplementation::getGender() const {
	const SharedCreatureObjectTemplate* creoData = templateObject.castToPointer<const SharedCreatureObjectTemplate*>();

	if (creoData == nullptr)
		return -1;

	return creoData->getGender();
}

void CreatureObjectImplementation::updateVehiclePosition(bool sendPackets) {
	ManagedReference<SceneObject*> parent = getParent().get();

	if (parent == nullptr || (!parent->isVehicleObject() && !parent->isMount()))
		return;

	CreatureObject* creo = cast<CreatureObject*>(parent.get());

	if (creo != nullptr) {
		//Locker clocker(creo, asCreatureObject());
		creo->setCurrentSpeed(getCurrentSpeed());
	}

	TangibleObjectImplementation::updateVehiclePosition(sendPackets);
}

CreatureObject* CreatureObjectImplementation::asCreatureObject() {
	return _this.getReferenceUnsafeStaticCast();
}

CreatureObject* CreatureObject::asCreatureObject() {
	return this;
}

bool CreatureObjectImplementation::isPlayerCreature() {
	if (templateObject == nullptr)
		return false;

	return templateObject->isPlayerCreatureTemplate();
}

CreditObject* CreatureObjectImplementation::getCreditObject() {
	if (creditObject == nullptr) {
		static const uint64 databaseID = ObjectDatabaseManager::instance()->getDatabaseID("credits");

		uint64 oid = ((getObjectID() & 0x0000FFFFFFFFFFFFull) | (databaseID << 48));

		ManagedReference<ManagedObject*> obj = Core::getObjectBroker()->lookUp(oid).castTo<ManagedObject*>();

		if (obj == nullptr) {
			obj = ObjectManager::instance()->createObject("CreditObject", isPersistent() ? 3 : 0, "credits", oid);

			if (obj == nullptr) {
				return nullptr;
			}

			creditObject = obj.castTo<CreditObject*>();

			if (creditObject == nullptr) {
				return nullptr;
			}

			Locker locker(creditObject);
			creditObject->setBankCredits(bankCredits, false);
			creditObject->setCashCredits(cashCredits, false);
			creditObject->setOwner(asCreatureObject());
			cashCredits = 0;
			bankCredits = 0;
		} else {
			creditObject = obj.castTo<CreditObject*>();
		}

		if (creditObject != nullptr && creditObject->getOwnerObjectID() != getObjectID()) {
			Locker locker(creditObject);
			creditObject->setOwner(asCreatureObject());
		}
	}

	return creditObject;
}

void CreatureObjectImplementation::removeOutOfRangeObjects() {
	CreatureObject* creature = asCreatureObject();

	if (creature == nullptr)
		return;

	CreatureObject* parent = getParent().get().castTo<CreatureObject*>();

	if (parent != nullptr && (parent->isVehicleObject() || parent->isMount()))
		creature = parent;

	SortedVector<QuadTreeEntry*> closeObjects;
	auto closeObjectsVector = creature->getCloseObjects();

	if (closeObjectsVector == nullptr)
		return;

	closeObjectsVector->safeCopyTo(closeObjects);

	auto worldPos = getWorldPosition();
	float ourX = worldPos.getX();
	float ourY = worldPos.getY();

	float ourRange = creature->getOutOfRangeDistance();

	auto creatureRootObject = creature->getRootParent();

	int countChecked = 0;
	int countCov = closeObjects.size();

	for (int i = 0; i < closeObjects.size(); ++i) {
		SceneObject* o = static_cast<SceneObject*>(closeObjects.getUnsafe(i));

		// Don't remove ourselves
		if (o == nullptr || o == creature)
			continue;

		// Don't remove things in the same parent as us (e.g. Geo Caves are massive)
		if (creatureRootObject != nullptr && o == creatureRootObject)
			continue;

		// Check for objects inside another object
		auto rootParent = o->getRootParent();

		// They should be managed by the parent
		if (rootParent != nullptr)
			continue;

		countChecked++;

		auto objectWorldPos = o->getWorldPosition();
		float deltaX = ourX - objectWorldPos.getX();
		float deltaY = ourY - objectWorldPos.getY();

		float outOfRangeSqr = Math::sqr(Math::max(ourRange, o->getOutOfRangeDistance()));

		// Check for out of range
		if (deltaX * deltaX + deltaY * deltaY > outOfRangeSqr) {
			countCov--;

			if (getCloseObjects() != nullptr)
				creature->removeInRangeObject(o);

			if (o->getCloseObjects() != nullptr)
				o->removeInRangeObject(creature);
		}
	}

	if (creature->isPlayerCreature()) {
		auto ghost = creature->getPlayerObject();

		// Cov count reporting
		if (ghost != nullptr && countCov > ghost->getCountMaxCov()) {
			creature->error("MaxCountCov = " + String::valueOf(countCov) + " checked = " + String::valueOf(countChecked));
			ghost->setCountMaxCov(countCov);
		}
	}
}

// The player may still have buildings and other far away objects in COV
// this function finds differences and sync's them between child/parent
void CreatureObjectImplementation::synchronizeCloseObjects() {
	auto creature = asCreatureObject();

	if (creature == nullptr)
		return;

	auto parent = getParent().get().castTo<CreatureObject*>();

	if (parent == nullptr)
		return;

	auto closeObjectsVector = getCloseObjects();

	if (closeObjectsVector == nullptr)
		return;

	auto parentCloseObjectsVector = parent->getCloseObjects();

	if (parentCloseObjectsVector == nullptr)
		return;

	SortedVector<QuadTreeEntry*> closeObjects;
	closeObjectsVector->safeCopyTo(closeObjects);

	SortedVector<QuadTreeEntry*> parentCloseObjects;
	parentCloseObjectsVector->safeCopyTo(parentCloseObjects);

	VectorMap<ManagedReference<SceneObject*>, uint8> diff;

	// Diff child/parent
	for (int i = 0; i < closeObjects.size(); ++i) {
		auto obj = static_cast<SceneObject*>(closeObjects.getUnsafe(i));

		if (obj != nullptr)
			diff.put(obj, 1);
	}

	// Diff parent/child
	for (int i = 0; i < parentCloseObjects.size(); ++i) {
		auto obj = static_cast<SceneObject*>(parentCloseObjects.getUnsafe(i));

		uint8 flags = 0;
		int found = diff.find(obj);

		if (found != -1)
			flags = diff.elementAt(found).getValue();

		diff.put(obj, flags | 2);
	}

	// Add anything missing on either side
	int count_c2p = 0;
	int count_p2c = 0;

	for (int i = 0; i < diff.size(); ++i) {
		auto element = diff.elementAt(i);
		auto obj = element.getKey();
		auto flags = element.getValue();

		if (flags == 3)
			continue;

		if (flags == 1) {
			count_c2p++;
			parent->addInRangeObject(obj);
		} else if(flags == 2) {
			count_p2c++;
			addInRangeObject(obj);
		}
	}

#if DEBUG_COV
	if (count_c2p != 0 || count_p2c != 0)
		info("synchronizeCloseObjects - Child => Parent: " + String::valueOf(count_c2p) + " Parent => Child: " + String::valueOf(count_p2c), true);
#endif // DEBUG_COV
}

void CreatureObjectImplementation::addPersonalEnemyFlag(CreatureObject* enemy, uint64 duration) {
	uint64 expireTime = duration;

	if (duration > 0) {
		Time currentTime;
		expireTime += currentTime.getMiliTime();

		ManagedReference<CreatureObject*> creo = asCreatureObject();
		ManagedReference<CreatureObject*> strongEnemy = enemy->asCreatureObject();

		Core::getTaskManager()->scheduleTask([creo, strongEnemy] {
			Locker locker(creo);
			creo->removePersonalEnemyFlag(strongEnemy);
		}, "PersonalEnemyFlagExpiration", duration);
	}

	personalEnemyFlags.put(enemy->getObjectID(), expireTime);
	sendPvpStatusTo(enemy);
}

void CreatureObjectImplementation::removePersonalEnemyFlag(CreatureObject* enemy) {
	uint64 enemyID = enemy->getObjectID();

	if (!personalEnemyFlags.drop(enemyID))
		return;

	sendPvpStatusTo(enemy);
}

void CreatureObjectImplementation::removePersonalEnemyFlag(uint64 enemyID) {
	if (!personalEnemyFlags.drop(enemyID))
		return;

	ZoneServer* zoneServer = server->getZoneServer();
	ManagedReference<CreatureObject*> enemy = zoneServer->getObject(enemyID).castTo<CreatureObject*>();

	if (enemy != nullptr)
		sendPvpStatusTo(enemy);
}

bool CreatureObjectImplementation::hasPersonalEnemyFlag(CreatureObject* enemy) const {
	uint64 enemyOID = enemy->getObjectID();

	if (!personalEnemyFlags.contains(enemyOID))
		return false;

	uint64 expireTime = personalEnemyFlags.get(enemyOID);

	if (expireTime == 0)
		return true;

	Time currentTime;

	return currentTime.getMiliTime() < expireTime;
}

void CreatureObjectImplementation::schedulePersonalEnemyFlagTasks() {
	if (personalEnemyFlags.size() <= 0)
		return;

	Time currentTime;
	uint64 curTime = currentTime.getMiliTime();

	for (int i = personalEnemyFlags.size() - 1; i >= 0; i--) {
		uint64 expireTime = personalEnemyFlags.get(i);
		uint64 enemyID = personalEnemyFlags.getKey(i);

		if (expireTime == 0)
			continue;

		if (expireTime <= curTime) {
			personalEnemyFlags.drop(enemyID);
		} else {
			uint64 timeDiff = expireTime - curTime;

			ManagedReference<CreatureObject*> creo = asCreatureObject();

			Core::getTaskManager()->scheduleTask([creo, enemyID, expireTime] {
				Locker locker(creo);

				if (creo->getPersonalEnemyFlagTime(enemyID) != expireTime)
					return;

				creo->removePersonalEnemyFlag(enemyID);
			}, "PersonalEnemyFlagExpiration", timeDiff);
		}
	}
}

void CreatureObjectImplementation::setHue(int hueIndex) {
	String appearanceFilename = getObjectTemplate()->getAppearanceFilename();
	VectorMap<String, Reference<CustomizationVariable*> > variables;
	AssetCustomizationManagerTemplate::instance()->getCustomizationVariables(appearanceFilename.hashCode(), variables, false);

	for (int i = 0; i < variables.size(); ++i) {
		const auto& varName = variables.elementAt(i).getKey();
		CustomizationVariable* customizationVariable = variables.elementAt(i).getValue().get();

		if (customizationVariable == nullptr)
			continue;

		PaletteColorCustomizationVariable* palette = dynamic_cast<PaletteColorCustomizationVariable*>(customizationVariable);

		if (palette == nullptr)
			continue;

		const auto& paletteFileName = palette->getPaletteFileName();
		UniqueReference<PaletteTemplate*> paletteTemplate(TemplateManager::instance()->getPaletteTemplate(paletteFileName));

		if (paletteTemplate == nullptr)
			continue;

		int maxIndex = paletteTemplate->getColorCount();

		int tempHue = hueIndex;

		if (tempHue < 0)
			tempHue = 0;
		else if (tempHue >= maxIndex)
			tempHue = maxIndex - 1;

		setCustomizationVariable(varName, tempHue, true);
	}

	hueValue = hueIndex;
}

void CreatureObjectImplementation::setClient(ZoneClientSession* cli) {
	owner = cli;
}
