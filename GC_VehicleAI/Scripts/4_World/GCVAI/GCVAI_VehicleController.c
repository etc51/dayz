// Контроллер AI-вождения одной машины. Живёт только на сервере (физика транспорта
// в DayZ считается сервером, клиенты получают уже готовый результат) — на клиенте
// этот класс не тикает.
//
// ⚠️ ТРЕБУЕТ ПРОВЕРКИ В WORKBENCH (P:\ или сравнение с исходниками DayZ из DayZ Tools):
// секция "ПРИМЕНИТЬ УПРАВЛЕНИЕ" ниже. Расчёт руля/скорости (GCVAI_PurePursuit) —
// чистая геометрия, она верна независимо от версии игры. А вот КАК передать
// вычисленные steer/throttle/brake в CarScript, если за рулём нет игрока —
// это единственное место, которое надо сверить с реальным API машины перед тестом,
// он менялся между версиями DayZ. Ниже — три способа по возрастанию сложности,
// первый стоит попробовать первым.

class GCVAI_VehicleController
{
	protected CarScript m_Vehicle;
	protected ref GCVAI_Route m_Route;
	protected ref GCVAI_VehicleProfile m_Profile;
	protected int m_RouteIndex;
	protected float m_CurrentSteer;   // сглаженное значение руля (см. m_SteerRateLimit)
	protected vector m_LastPos;
	protected int m_LastProgressTime; // когда последний раз реально продвинулись по маршруту
	protected int m_StuckRetries;
	protected bool m_Reversing;
	protected int m_ReverseUntilTime;
	protected bool m_Active;

	void GCVAI_VehicleController(CarScript vehicle, GCVAI_Route route)
	{
		m_Vehicle = vehicle;
		m_Route = route;
		m_Profile = GCVAI_VehicleProfiles.ForClassName(vehicle.GetType());
		m_RouteIndex = 0;
		m_CurrentSteer = 0;
		m_LastPos = vehicle.GetPosition();
		m_LastProgressTime = GetGame().GetTime();
		m_StuckRetries = 0;
		m_Reversing = false;
		m_Active = true;

		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, GCVAI_TICK_MS, true);
	}

	bool IsActive()
	{
		return m_Active && m_Vehicle && !m_Vehicle.IsDamageDestroyed();
	}

	void Stop()
	{
		m_Active = false;
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Tick);
		GCVAI_ApplyControls(m_Vehicle, 0, 0, 1); // отпустить газ, зажать тормоз перед удалением/передачей игроку
	}

	void Tick()
	{
		if (!IsActive())
		{
			Stop();
			return;
		}

		vector pos = m_Vehicle.GetPosition();
		vector forward = m_Vehicle.GetDirection();
		float speedKmh = m_Vehicle.GetSpeedometer(); // км/ч — см. примечание в конце файла, если такого метода нет в вашей версии

		// --- Проверка "застряли" ---
		float movedSq = GCVAI_Flat_DistanceSq(pos, m_LastPos);
		if (movedSq > 1.0) // проехали больше метра — считаем, что не застряли
		{
			m_LastPos = pos;
			m_LastProgressTime = GetGame().GetTime();
		}

		if (m_Reversing)
		{
			GCVAI_HandleReverse(pos);
			return;
		}

		if (GetGame().GetTime() - m_LastProgressTime > GCVAI_STUCK_TIMEOUT_MS)
		{
			GCVAI_StartReverse();
			return;
		}

		// --- Объезд препятствий: луч вперёд по курсу машины ---
		if (GCVAI_ObstacleAhead(pos, forward))
		{
			GCVAI_ApplyControls(m_Vehicle, 0, 0, 0.6); // притормозить, не дёргать руль, пока едем на препятствие
			return;
		}

		// --- Расчёт руля и целевой скорости ---
		ref GCVAI_SteerResult r = GCVAI_PurePursuit.Compute(m_Route, m_RouteIndex, pos, forward, speedKmh, m_Profile);
		m_RouteIndex = r.m_TargetIndex;

		if (r.m_RouteFinished)
		{
			Stop();
			GCVAI_Log.Info("Машина доехала до конца маршрута '" + m_Route.m_Name + "', AI-вождение отключено.");
			return;
		}

		// Плавно подводим руль к целевому значению, а не дёргаем сразу — иначе автобус
		// на скорости может занести (см. m_SteerRateLimit в профиле).
		float steerDelta = Math.Clamp(r.m_Steer - m_CurrentSteer, -m_Profile.m_SteerRateLimit, m_Profile.m_SteerRateLimit);
		m_CurrentSteer = m_CurrentSteer + steerDelta;

		float throttle = 1.0;
		float brake = 0;
		if (speedKmh > r.m_TargetSpeedKmh)
		{
			throttle = 0;
			brake = Math.Clamp((speedKmh - r.m_TargetSpeedKmh) / 15.0, 0.1, 1.0);
		}

		GCVAI_ApplyControls(m_Vehicle, m_CurrentSteer, throttle, brake);
	}

	protected bool GCVAI_ObstacleAhead(vector pos, vector forward)
	{
		vector from = pos + Vector(0, GCVAI_OBSTACLE_RAY_HEIGHT, 0);
		vector to = from + (forward * GCVAI_OBSTACLE_RAY_LEN);

		autoptr TraceParam trace = new TraceParam();
		trace.Start = from;
		trace.End = to;
		trace.Flags = TraceFlags.WORLD | TraceFlags.OBJECTS;
		trace.LayerMask = ObjIntersectView;
		trace.Exclude = m_Vehicle;

		float hit = GetGame().GetWorld().TraceMove(trace, NULL);
		return hit < 1.0; // луч во что-то упёрся раньше конца
	}

	protected void GCVAI_StartReverse()
	{
		m_Reversing = true;
		m_ReverseUntilTime = GetGame().GetTime() + GCVAI_STUCK_REVERSE_MS;
		m_StuckRetries = m_StuckRetries + 1;
		GCVAI_Log.Warn("Машина застряла, сдаю назад (попытка " + m_StuckRetries.ToString() + "/" + GCVAI_STUCK_MAX_RETRIES.ToString() + ").");
	}

	protected void GCVAI_HandleReverse(vector pos)
	{
		if (GetGame().GetTime() < m_ReverseUntilTime)
		{
			GCVAI_ApplyControls(m_Vehicle, 0, -1, 0); // -1 по газу = задний ход
			return;
		}

		m_Reversing = false;
		m_LastProgressTime = GetGame().GetTime(); // дать шанс снова поехать вперёд, не спамить "застряли"
		m_LastPos = pos;

		if (m_StuckRetries >= GCVAI_STUCK_MAX_RETRIES)
		{
			// Страховка: не даём конвою висеть намертво до рестарта сервера.
			// Телепорт на следующую точку маршрута — заметно игроку рядом, поэтому это
			// крайний случай, а не обычный режим работы.
			GCVAI_Log.Error("Машина не смогла объехать препятствие " + GCVAI_STUCK_MAX_RETRIES.ToString() + " раз подряд — переставляю на следующую точку маршрута.");
			int nextIdx = m_Route.NextIndex(m_RouteIndex);
			if (nextIdx == -1) { Stop(); return; }
			m_RouteIndex = nextIdx;
			m_Vehicle.SetPosition(m_Route.PointAt(nextIdx));
			m_StuckRetries = 0;
		}
	}
}

// === ПРИМЕНИТЬ УПРАВЛЕНИЕ — сверить с Workbench перед тестом ===
//
// Ниже — три варианта, как передать steer/throttle/brake машине без сидящего за рулём
// игрока. Начать со Способа 1, он самый простой; если в вашей версии DayZ таких методов
// нет — смотреть Способ 2 или 3 в исходниках CarScript (DayZ Tools → P:\scripts или через
// распаковку dta\scripts.pbo).
//
// СПОСОБ 1 (предпочтительный, если доступен): у CarScript есть собственные analog-инпуты
// для AI/скриптового управления, например:
//   m_Vehicle.SetAnalogSteering(steer);
//   m_Vehicle.SetAnalogThrottle(throttle);
//   m_Vehicle.SetAnalogBrake(brake);
// (точные имена методов проверить в CarScript.c/Transport.c из исходников игры)
//
// СПОСОБ 2: через CarControllerBase / физический компонент колёс — обычно принимает
// массив под колесо (для 4 колёс) вместо одного значения на всю машину:
//   PhysicsComponent phys = m_Vehicle.GetCompartment...  // структура зависит от версии
//
// СПОСОБ 3 (запасной, грубый, но надёжный): не трогать физику руля вообще, а напрямую
// двигать машину — читать позицию каждый тик и применять SetPosition/SetOrientation
// небольшими шагами вдоль маршрута (то самое "телепортация по waypoint'ам" из
// обсуждения). Хуже физически (проезжает сквозь мелкие объекты), зато 100% не падает
// из-за смены API между патчами DayZ. Держим как fallback, если Способ 1/2 не заведутся.
void GCVAI_ApplyControls(CarScript vehicle, float steer, float throttle, float brake)
{
	// TODO(Workbench): заменить на реальные вызовы после проверки в P:\ (см. комментарий выше).
	// Пока — заглушка с логом, чтобы контроллер компилировался и его можно было тестировать
	// поэтапно (сначала маршруты/лог, потом реальное управление).
	GCVAI_Log.Info("steer=" + steer.ToString() + " throttle=" + throttle.ToString() + " brake=" + brake.ToString());
}
