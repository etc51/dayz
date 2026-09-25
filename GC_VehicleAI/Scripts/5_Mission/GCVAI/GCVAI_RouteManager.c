// Серверный менеджер: загружает маршруты из GC_VehicleAI/routes/*.xml (или
// profiles/GCVAI/routes/*.xml, если там что-то есть — удобно для правок без пересборки PBO),
// спавнит машины на маршрутах и следит за их контроллерами.

class GCVAI_ActiveConvoy
{
	Object m_Vehicle;
	ref GCVAI_VehicleController m_Controller;
}

class GCVAI_RouteManager
{
	protected ref map<string, ref GCVAI_Route> m_Routes = new map<string, ref GCVAI_Route>;
	protected ref array<ref GCVAI_ActiveConvoy> m_Active = new array<ref GCVAI_ActiveConvoy>;

	void LoadAllRoutes()
	{
		m_Routes.Clear();

		array<string> dirs = new array<string>;
		dirs.Insert("GC_VehicleAI/routes/");        // маршруты, идущие вместе с модом
		dirs.Insert("$profile:GCVAI/routes/");      // маршруты сервера, правятся без пересборки PBO

		foreach (string dir : dirs)
		{
			if (!FileExist(dir)) continue;

			string fileName;
			FindFileHandle fh = FindFile(dir + "*.xml", fileName);
			if (!fh) continue;
			do
			{
				ref GCVAI_Route route = GCVAI_Route.LoadFromFile(dir + fileName);
				if (route) m_Routes.Set(route.m_Name, route);
			}
			while (FindNextFile(fh, fileName));
			CloseFindFile(fh);
		}

		GCVAI_Log.Info("Загружено маршрутов: " + m_Routes.Count().ToString());
	}

	// Заспавнить машину на маршруте. className — например "Bus", "F350_Ambulance",
	// смотрите фактическое имя класса в config.cpp зависимости (или в Черноруссии — types.xml).
	bool SpawnOnRoute(string routeName, string className)
	{
		ref GCVAI_Route route = m_Routes.Get(routeName);
		if (!route)
		{
			GCVAI_Log.Error("Маршрут '" + routeName + "' не найден (загружено: " + m_Routes.Count().ToString() + ").");
			return false;
		}

		vector spawnPos = route.PointAt(0);
		Object obj = GetGame().CreateObjectEx(className, spawnPos, ECE_PLACE_ON_SURFACE | ECE_INITAI);
		CarScript car = CarScript.Cast(obj);
		if (!car)
		{
			GCVAI_Log.Error("Класс '" + className + "' — не машина (CarScript), спавн отменён.");
			if (obj) GetGame().ObjectDelete(obj);
			return false;
		}

		// Смотрим машину сразу по направлению первого отрезка маршрута, а не как повезёт при спавне.
		vector nextPos = route.PointAt(route.NextIndex(0));
		vector dir = nextPos - spawnPos;
		dir[1] = 0;
		if (dir.LengthSq() > 0.01)
		{
			dir.Normalize();
			car.SetOrientation(dir.VectorToAngles());
		}

		ref GCVAI_ActiveConvoy convoy = new GCVAI_ActiveConvoy;
		convoy.m_Vehicle = car;
		convoy.m_Controller = new GCVAI_VehicleController(car, route);
		m_Active.Insert(convoy);

		GCVAI_Log.Info("Заспавнена машина '" + className + "' на маршруте '" + routeName + "'.");
		return true;
	}

	// Периодическая уборка: убрать из списка активных конвои с уничтоженным/отключённым контроллером.
	void Cleanup()
	{
		for (int i = m_Active.Count() - 1; i >= 0; i--)
		{
			if (!m_Active.Get(i).m_Controller.IsActive())
				m_Active.Remove(i);
		}
	}

	array<string> GetRouteNames()
	{
		array<string> names = new array<string>;
		foreach (string k, GCVAI_Route v : m_Routes) names.Insert(k);
		return names;
	}
}
