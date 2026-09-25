class CfgPatches
{
	class GC_VehicleAI
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = { "DZ_Data", "DZ_Scripts" };
	};
};

class CfgMods
{
	class GC_VehicleAI
	{
		dir = "GC_VehicleAI";
		name = "GC Vehicle AI — конвои и патрули на дорогах";
		author = "etc51";
		version = "0.1.0";
		type = "mod";
		dependencies[] = { "Game", "World", "Mission" };

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = { "GC_VehicleAI/Scripts/3_Game" };
			};
			class worldScriptModule
			{
				value = "";
				files[] = { "GC_VehicleAI/Scripts/4_World" };
			};
			class missionScriptModule
			{
				value = "";
				files[] = { "GC_VehicleAI/Scripts/5_Mission" };
			};
		};
	};
};
