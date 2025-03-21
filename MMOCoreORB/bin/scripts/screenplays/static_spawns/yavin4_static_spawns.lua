--This is to be used for static spawns that are NOT part of caves, cities, dungeons, poi's, or other large screenplays.
Yavin4StaticSpawnsScreenPlay = ScreenPlay:new
	{
		screenplayName = "Yavin4StaticSpawnsScreenPlay",
}

registerScreenPlay("Yavin4StaticSpawnsScreenPlay", true)

function Yavin4StaticSpawnsScreenPlay:start()
	if (isZoneEnabled("yavin4")) then
		self:spawnMobiles()
	end
end

function Yavin4StaticSpawnsScreenPlay:spawnMobiles()
	-- Light Jedi Enclave (-5575, 4910)
	local pSentinel = spawnMobile("yavin4", "light_jedi_sentinel", 0, -5579.0, 87.7, 4908.5, -179, 0)

	if (pSentinel ~= nil) then
		writeData(SceneObject(pSentinel):getObjectID() .. ":sentinelID", 1)
		createEvent(10, "Yavin4StaticSpawnsScreenPlay", "setupSentinel", pSentinel, "")
	end

	pSentinel = spawnMobile("yavin4", "light_jedi_sentinel", 0, -5572.6, 87.7, 4908.2, -179, 0)

	if (pSentinel ~= nil) then
		writeData(SceneObject(pSentinel):getObjectID() .. ":sentinelID", 2)
		createEvent(10, "Yavin4StaticSpawnsScreenPlay", "setupSentinel", pSentinel, "")
	end

	-- Dark Jedi Enclave (5068, 310)
	pSentinel = spawnMobile("yavin4", "dark_jedi_sentinel", 0, 5074.2, 78.8, 313.9, 90, 0)

	if (pSentinel ~= nil) then
		writeData(SceneObject(pSentinel):getObjectID() .. ":sentinelID", 1)
		createEvent(10, "Yavin4StaticSpawnsScreenPlay", "setupSentinel", pSentinel, "")
	end

	pSentinel = spawnMobile("yavin4", "dark_jedi_sentinel", 0, 5074.3, 79.2, 306.4, 90, 0)

	if (pSentinel ~= nil) then
		writeData(SceneObject(pSentinel):getObjectID() .. ":sentinelID", 2)
		createEvent(10, "Yavin4StaticSpawnsScreenPlay", "setupSentinel", pSentinel, "")
	end

	-- Yavin4 Ruins (-6299, -1502)
	spawnMobile("yavin4", "crazed_geonosian_guard",300,-6299.1,24.7,-1502,0,0)
	spawnMobile("yavin4", "crazed_geonosian_guard",300,-6299.1,25.6,-1506.4,179,0)
	spawnMobile("yavin4", "crazed_geonosian_guard",300,-6302.1,24.9,-1504.2,-90,0)
	spawnMobile("yavin4", "crazed_geonosian_guard",300,-6296.0,25.3,-1504.2,90,0)

	-- Captain Eso & Yith Seenath's base of operations (1591, 1536)
	spawnMobile("yavin4", "rebel_commando", 360, 1595.0, 102.5, 1531.9, -40, 0)
	spawnMobile("yavin4", "rebel_commando", 360, 1595.0, 102.5, 1530.4, -30, 0)
	spawnMobile("yavin4", "rebel_commando", 360, 1597.8, 102.9, 1530.3, -37, 0)
	spawnMobile("yavin4", "rebel_commando", 360, 1595.3, 102.7, 1527.8, -26, 0)

	-- Imp base camp outside Massassi Temple POI
	spawnMobile("yavin4", "stormtrooper", 360, -3196.7, 69.6, -3139.5, -100, 0)
	spawnMobile("yavin4", "stormtrooper", 360, -3195.8, 69.8, -3130.7, -104, 0)

	--LJ Shuttle
	spawnSceneObject("yavin4", "object/building/player/city/shuttleport_corellia.iff", -5656, 102, 4925, 0, 0)
	local pSentinel = spawnMobile("yavin4", "light_jedi_sentinel", 1, -5646.54, 100.005, 4948.13, 180, 0)
	if (pSentinel ~= nil) then
		writeData(SceneObject(pSentinel):getObjectID() .. ":sentinelID", 3)
		createEvent(10, "Yavin4StaticSpawnsScreenPlay", "setupSentinel", pSentinel, "")
	end

	pSentinel = spawnMobile("yavin4", "light_jedi_sentinel", 1, -5662.64, 98.8252, 4946.8, 167, 0)
	if (pSentinel ~= nil) then
		writeData(SceneObject(pSentinel):getObjectID() .. ":sentinelID", 4)
		createEvent(10, "Yavin4StaticSpawnsScreenPlay", "setupSentinel", pSentinel, "")
	end
	
	--DJ Shuttle
	spawnSceneObject("yavin4", "object/building/player/city/shuttleport_corellia.iff", 5061, 71, 412, 0, 0)
	pSentinel = spawnMobile("yavin4", "dark_jedi_sentinel", 1, 5053.79, 67.9311, 434.323, 177, 0)
	if (pSentinel ~= nil) then
		writeData(SceneObject(pSentinel):getObjectID() .. ":sentinelID", 3)
		createEvent(10, "Yavin4StaticSpawnsScreenPlay", "setupSentinel", pSentinel, "")
	end

	pSentinel = spawnMobile("yavin4", "dark_jedi_sentinel", 1, 5068.99, 67.4718, 434.442, 177, 0)
	if (pSentinel ~= nil) then
		writeData(SceneObject(pSentinel):getObjectID() .. ":sentinelID", 4)
		createEvent(10, "Yavin4StaticSpawnsScreenPlay", "setupSentinel", pSentinel, "")
	end
end

function Yavin4StaticSpawnsScreenPlay:setupSentinel(pSentinel)
	AiAgent(pSentinel):setAiTemplate("enclavesentinel")
	createObserver(OBJECTDESTRUCTION, "Yavin4StaticSpawnsScreenPlay", "notifySentinelDead", pSentinel)
	createEvent(300 * 1000, "Yavin4StaticSpawnsScreenPlay", "rotateSentinel", pSentinel, "")
end

function Yavin4StaticSpawnsScreenPlay:rotateSentinel(pSentinel)
	if (pSentinel == nil or CreatureObject(pSentinel):isDead()) then
		return
	end

	if (CreatureObject(pSentinel):isInCombat()) then
		createEvent(300 * 1000, "Yavin4StaticSpawnsScreenPlay", "rotateSentinel", pSentinel, "")
		return
	end

	local objName = SceneObject(pSentinel):getObjectName()
	local objID = SceneObject(pSentinel):getObjectID()
	local sentinelID = readData(objID .. ":sentinelID")

	if (objName == "light_jedi_sentinel") then
		SceneObject(pSentinel):updateDirection(-179)
	elseif (objName == "dark_jedi_sentinel") then
		if (sentinelID == "5" or sentinelID == "6") then
			SceneObject(pSentinel):updateDirection(177)
		else
			SceneObject(pSentinel):updateDirection(90)
		end
	end

	createEvent(300 * 1000, "Yavin4StaticSpawnsScreenPlay", "rotateSentinel", pSentinel, "")
end

function Yavin4StaticSpawnsScreenPlay:notifySentinelDead(pSentinel, pKiller)
	if (pSentinel == nil) then
		return 1
	end

	local objName = SceneObject(pSentinel):getObjectName()
	local objID = SceneObject(pSentinel):getObjectID()
	local sentinelID = readData(objID .. ":sentinelID")
	deleteData(objID .. ":sentinelID")

	if (objName == "light_jedi_sentinel") then
		createEvent(300 * 1000, "Yavin4StaticSpawnsScreenPlay", "respawnLightSentinel", pSentinel, sentinelID)
	elseif (objName == "dark_jedi_sentinel") then
		createEvent(300 * 1000, "Yavin4StaticSpawnsScreenPlay", "respawnDarkSentinel", pSentinel, sentinelID)
	end

	return 1
end

function Yavin4StaticSpawnsScreenPlay:sentinelDefenderDropped(pSentinel)
	if (pSentinel == nil or CreatureObject(pSentinel):isDead()) then
		return 1
	end

	createEvent(10 * 1000, "Yavin4StaticSpawnsScreenPlay", "rotateSentinel", pSentinel, "")

	return 0
end

function Yavin4StaticSpawnsScreenPlay:respawnLightSentinel(pOldSentinel, sentinelID)
	local spawnLoc

	if (sentinelID == "1") then
		spawnLoc = { -5579.0, 87.7, 4908.5, -179 }
	elseif (sentinelID == "2") then
		spawnLoc = { -5572.6, 87.7, 4908.2, -179 }
	elseif (sentinelID == "3") then
		spawnLoc = { -5646.54, 100.005, 4948.13, 180 }
	elseif (sentinelID == "4") then
		spawnLoc = { -5662.64, 98.8252, 4946.8, 167 }
	end

	if (spawnLoc == nil) then
		return
	end

	local pSentinel = spawnMobile("yavin4", "light_jedi_sentinel", 0, spawnLoc[1], spawnLoc[2], spawnLoc[3], spawnLoc[4], 0)

	if (pSentinel ~= nil) then
		writeData(SceneObject(pSentinel):getObjectID() .. ":sentinelID", sentinelID)
		createEvent(10, "Yavin4StaticSpawnsScreenPlay", "setupSentinel", pSentinel, "")
	end
end

function Yavin4StaticSpawnsScreenPlay:respawnDarkSentinel(pOldSentinel, sentinelID)
	local spawnLoc

	if (sentinelID == "1") then
		spawnLoc = { 5074.2, 78.8, 313.9, 90 }
	elseif (sentinelID == "2") then
		spawnLoc = { 5074.3, 79.2, 306.4, 90 }
	elseif (sentinelID == "3") then
		spawnLoc = { 5053.79, 67.9311, 434.323, 177 }
	elseif (sentinelID == "4") then
		spawnLoc = { 5068.99, 67.4718, 434.442, 177 }
	end

	if (spawnLoc == nil) then
		return
	end

	local pSentinel = spawnMobile("yavin4", "dark_jedi_sentinel", 0, spawnLoc[1], spawnLoc[2], spawnLoc[3], spawnLoc[4], 0)

	if (pSentinel ~= nil) then
		writeData(SceneObject(pSentinel):getObjectID() .. ":sentinelID", sentinelID)
		createEvent(10, "Yavin4StaticSpawnsScreenPlay", "setupSentinel", pSentinel, "")
	end
end
