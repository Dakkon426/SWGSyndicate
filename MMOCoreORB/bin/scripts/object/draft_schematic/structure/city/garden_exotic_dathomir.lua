--Copyright (C) 2010 <SWGEmu>


--This File is part of Core3.

--This program is free software; you can redistribute 
--it and/or modify it under the terms of the GNU Lesser 
--General Public License as published by the Free Software
--Foundation; either version 2 of the License, 
--or (at your option) any later version.

--This program is distributed in the hope that it will be useful, 
--but WITHOUT ANY WARRANTY; without even the implied warranty of 
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
--See the GNU Lesser General Public License for
--more details.

--You should have received a copy of the GNU Lesser General 
--Public License along with this program; if not, write to
--the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

--Linking Engine3 statically or dynamically with other modules 
--is making a combined work based on Engine3. 
--Thus, the terms and conditions of the GNU Lesser General Public License 
--cover the whole combination.

--In addition, as a special exception, the copyright holders of Engine3 
--give you permission to combine Engine3 program with free software 
--programs or libraries that are released under the GNU LGPL and with 
--code included in the standard release of Core3 under the GNU LGPL 
--license (or modified versions of such code, with unchanged license). 
--You may copy and distribute such a system following the terms of the 
--GNU LGPL for Engine3 and the licenses of the other code concerned, 
--provided that you include the source code of that other code when 
--and as the GNU LGPL requires distribution of source code.

--Note that people who make modified versions of Engine3 are not obligated 
--to grant this special exception for their modified versions; 
--it is their choice whether to do so. The GNU Lesser General Public License 
--gives permission to release a modified version without this exception; 
--this exception also makes it possible to release a modified version 


object_draft_schematic_structure_city_garden_exotic_dathomir = object_draft_schematic_structure_city_shared_garden_exotic_dathomir:new {

	templateType = DRAFTSCHEMATIC,

	customObjectName = "Deed for: Dathomir Obelisk (Garden)",

	craftingToolTab = 1024, -- (See DraftSchematicObjectTemplate.h)
	complexity = 27,
	size = 12,
	factoryCrateSize = 1000,
	factoryCrateType = "object/factory/factory_crate_installation.iff",
   
	xpType = "crafting_structure_general",
	xp = 9000,

	assemblySkill = "structure_assembly",
	experimentingSkill = "structure_experimentation",
	customizationSkill = "structure_customization",

	customizationOptions = {},
	customizationStringNames = {},
	customizationDefaults = {},

	ingredientTemplateNames = {"craft_structure_ingredients_n", "craft_structure_ingredients_n", "craft_structure_ingredients_n"},
	ingredientTitleNames = {"decorative_flora", "foundation", "wall_sections"},
	ingredientSlotType = {0, 0, 2},
	resourceTypes = {"vegetable_fungi_dathomir", "ore_siliclastic", "object/tangible/component/structure/shared_wall_module.iff"},
	resourceQuantities = {3000, 1500, 14},
	contribution = {100, 100, 100},

	targetTemplate = "object/tangible/deed/city_deed/garden_dathomir_lrg_01_deed.iff",

	additionalTemplates = {
		"object/tangible/deed/city_deed/shared_garden_dathomir_med_01_deed.iff",
		"object/tangible/deed/city_deed/shared_garden_dathomir_sml_01_deed.iff",
	}
}
ObjectTemplates:addTemplate(object_draft_schematic_structure_city_garden_exotic_dathomir, "object/draft_schematic/structure/city/garden_exotic_dathomir.iff")
