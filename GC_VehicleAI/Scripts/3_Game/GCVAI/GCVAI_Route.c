// Один маршрут: упорядоченный список точек с необязательной подсказкой по скорости.
// Хранится в XML в GC_VehicleAI/routes/*.xml (или в profiles/GCVAI/routes/ на сервере —
// см. GCVAI_RouteRecorder). Формат простой и правится руками при необходимости:
//
// <route name="convoy_pavlovo_cherno" loop="1">
//   <point x="1690" y="0" z="3860" speed="45"/>
//   <point x="1720" y="0" z="3910" speed="20"/>
//   ...
// </route>

class GCVAI_RoutePoint
{
	vector m_Pos;
	float  m_SpeedKmh; // 0 = использовать GCVAI_SPEED_STRAIGHT_KMH по умолчанию

	void GCVAI_RoutePoint(vector pos, float speedKmh)
	{
		m_Pos = pos;
		m_SpeedKmh = speedKmh;
	}
}

class GCVAI_Route
{
	string m_Name;
	bool   m_Loop; // true — после последней точки едем на первую (кольцевой маршрут)
	ref array<ref GCVAI_RoutePoint> m_Points = new array<ref GCVAI_RoutePoint>;

	int Count()
	{
		return m_Points.Count();
	}

	vector PointAt(int i)
	{
		return m_Points.Get(i % m_Points.Count()).m_Pos; // % — чтобы кольцевой маршрут не выходил за границы
	}

	int NextIndex(int i)
	{
		if (i + 1 >= m_Points.Count())
		{
			if (m_Loop) return 0;
			return -1; // маршрут закончился
		}
		return i + 1;
	}

	// Загрузка маршрута из XML-файла. Возвращает null, если файл битый или не найден.
	static ref GCVAI_Route LoadFromFile(string path)
	{
		if (!FileExist(path))
		{
			GCVAI_Log.Error("Файл маршрута не найден: " + path);
			return null;
		}

		ref GCVAI_Route route = new GCVAI_Route;
		// Простой ручной парсер атрибутов, без полноценного XML-DOM (в Enforce Script его нет
		// из коробки для произвольных файлов). Формат строго построчный, см. пример выше.
		FileHandle f = OpenFile(path, FileMode.READ);
		if (!f)
		{
			GCVAI_Log.Error("Не удалось открыть файл маршрута: " + path);
			return null;
		}

		string line;
		while (FGets(f, line) >= 0)
		{
			line = GCVAI_TrimString(line);
			if (line.IndexOf("<route") == 0)
			{
				route.m_Name = GCVAI_ExtractAttr(line, "name");
				route.m_Loop = GCVAI_ExtractAttr(line, "loop") == "1";
			}
			else if (line.IndexOf("<point") == 0)
			{
				float x = GCVAI_ExtractAttr(line, "x").ToFloat();
				float y = GCVAI_ExtractAttr(line, "y").ToFloat();
				float z = GCVAI_ExtractAttr(line, "z").ToFloat();
				string speedStr = GCVAI_ExtractAttr(line, "speed");
				float speed = 0;
				if (speedStr != "") speed = speedStr.ToFloat();
				route.m_Points.Insert(new GCVAI_RoutePoint(Vector(x, y, z), speed));
			}
		}
		CloseFile(f);

		if (route.m_Points.Count() < 2)
		{
			GCVAI_Log.Error("Маршрут " + path + " содержит меньше 2 точек — пропущен.");
			return null;
		}

		GCVAI_Log.Info("Загружен маршрут '" + route.m_Name + "': " + route.m_Points.Count().ToString() + " точек, loop=" + route.m_Loop.ToString());
		return route;
	}

	// Сохранение маршрута (используется рекордером). Формат совместим с LoadFromFile.
	bool SaveToFile(string path)
	{
		FileHandle f = OpenFile(path, FileMode.WRITE);
		if (!f)
		{
			GCVAI_Log.Error("Не удалось создать файл маршрута: " + path);
			return false;
		}

		string loopAttr = "0";
		if (m_Loop) loopAttr = "1";
		FPrintln(f, "<route name=\"" + m_Name + "\" loop=\"" + loopAttr + "\">");
		foreach (GCVAI_RoutePoint p : m_Points)
		{
			FPrintln(f, "  <point x=\"" + p.m_Pos[0].ToString() + "\" y=\"" + p.m_Pos[1].ToString()
				+ "\" z=\"" + p.m_Pos[2].ToString() + "\" speed=\"" + p.m_SpeedKmh.ToString() + "\"/>");
		}
		FPrintln(f, "</route>");
		CloseFile(f);
		return true;
	}
}

// --- Вспомогательные функции разбора строк (без внешних зависимостей) ---

string GCVAI_TrimString(string s)
{
	s.TrimInPlace();
	return s;
}

// Достаёт значение атрибута name="значение" из строки XML-подобного тега.
string GCVAI_ExtractAttr(string line, string attr)
{
	string needle = attr + "=\"";
	int start = line.IndexOf(needle);
	if (start == -1) return "";
	start += needle.Length();
	int end = line.IndexOf("\"", start);
	if (end == -1) return "";
	return line.Substring(start, end - start);
}
