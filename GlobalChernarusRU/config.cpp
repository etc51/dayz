class CfgPatches
{
	class GlobalChernarusRU
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = { "DZ_Data", "DZ_Scripts" };
	};
};

class CfgMods
{
	class GlobalChernarusRU
	{
		dir = "GlobalChernarusRU";
		name = "Глобальная Черноруссия (RU)";
		author = "etc51";
		version = "0.1.0";
		type = "mod";
		dependencies[] = { "Game", "World", "Mission" };

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = { "GlobalChernarusRU/Scripts/3_Game" };
			};
			class worldScriptModule
			{
				value = "";
				files[] = { "GlobalChernarusRU/Scripts/4_World" };
			};
			class missionScriptModule
			{
				value = "";
				files[] = { "GlobalChernarusRU/Scripts/5_Mission" };
			};
		};
	};
};
