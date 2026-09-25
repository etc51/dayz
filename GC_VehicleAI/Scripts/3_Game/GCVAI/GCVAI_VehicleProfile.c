// Профиль вождения под конкретный класс машины. У автобуса и легковушки разная физика
// (база длиннее, центр тяжести выше, руль поворачивается медленнее и на меньший угол,
// тормозной путь длиннее) — одни и те же коэффициенты Pure Pursuit для обоих будут либо
// вилять на легковушке, либо переворачивать автобус на повороте.
//
// Профили подбираются опытным путём в игре (см. docs/VEHICLE_AI_RU.md, раздел "Настройка").
// Список машин без явного профиля получает GCVAI_DefaultProfile().

class GCVAI_VehicleProfile
{
	float m_LookaheadMin;   // метры — минимальная дистанция до целевой точки
	float m_LookaheadMax;   // метры — на скорости смотрим дальше вперёд
	float m_SpeedStraightKmh;
	float m_SpeedTurnKmh;   // скорость в резком повороте (кривизна пути высокая)
	float m_SteerRateLimit; // макс. изменение руля за тик (0..1 за GCVAI_TICK_MS) — не даёт дёргать руль резко
	float m_BrakeLeadM;     // за сколько метров до поворота начинать тормозить

	void GCVAI_VehicleProfile(float lookMin, float lookMax, float speedStraight, float speedTurn, float steerRate, float brakeLead)
	{
		m_LookaheadMin = lookMin;
		m_LookaheadMax = lookMax;
		m_SpeedStraightKmh = speedStraight;
		m_SpeedTurnKmh = speedTurn;
		m_SteerRateLimit = steerRate;
		m_BrakeLeadM = brakeLead;
	}
}

class GCVAI_VehicleProfiles
{
	// Легковая машина: короткая база, разворачивается быстро, руль реагирует резко.
	static ref GCVAI_VehicleProfile Car()
	{
		return new GCVAI_VehicleProfile(6, 16, 50, 18, 0.15, 12);
	}

	// Автобус / грузовик: длинная база, руль ограничен по углу, тормозить нужно заранее.
	// Меньше скорость на прямой, ещё меньше в повороте, дальше смотрим вперёд (иначе
	// "срезает" повороты и цепляет обочину задними колёсами), медленнее крутим руль.
	static ref GCVAI_VehicleProfile Bus()
	{
		return new GCVAI_VehicleProfile(12, 24, 38, 12, 0.08, 22);
	}

	// Профиль по умолчанию — консервативный, подходит как временный вариант для машин,
	// под которые ещё не подобрали точные коэффициенты.
	static ref GCVAI_VehicleProfile Default()
	{
		return new GCVAI_VehicleProfile(8, 20, 40, 15, 0.12, 15);
	}

	// Подобрать профиль по имени класса машины (см. GCVAI_VehicleAIConfig.c — там же
	// список классов, для которых включается AI-вождение).
	static ref GCVAI_VehicleProfile ForClassName(string className)
	{
		string c = className;
		c.ToLower();
		if (c.Contains("bus") || c.Contains("truck") || c.Contains("kamaz") || c.Contains("ural"))
			return Bus();
		return Car();
	}
}
