// Алгоритм Pure Pursuit — тот же метод расчёта руля, что применяют в реальных
// беспилотных автомобилях и робототехнике. Идея: смотрим не на "ближайшую точку дороги",
// а на точку на маршруте впереди на дистанции look-ahead, и рулим так, чтобы по дуге
// приехать в неё. Даёт плавное вписывание в повороты без резких дёрганий руля.
//
// Этот файл не содержит ничего специфичного для DayZ — только векторную геометрию,
// поэтому его логику можно проверять и отлаживать вне игры (в отдельном скрипте).

class GCVAI_SteerResult
{
	float m_Steer;       // -1 (влево до упора) .. +1 (вправо до упора)
	float m_TargetSpeedKmh;
	int   m_TargetIndex;  // индекс точки маршрута, на которую сейчас целимся
	bool  m_RouteFinished; // true — доехали до конца некольцевого маршрута
}

class GCVAI_PurePursuit
{
	// pos, forward — текущие позиция и направление машины (forward нормализован, Y игнорируем).
	// speedKmh — текущая скорость, нужна, чтобы увеличивать look-ahead на высокой скорости
	// (иначе на скорости целимся слишком близко и получаем пилообразную траекторию).
	static ref GCVAI_SteerResult Compute(GCVAI_Route route, int currentIndex, vector pos, vector forward, float speedKmh, GCVAI_VehicleProfile profile)
	{
		ref GCVAI_SteerResult result = new GCVAI_SteerResult;
		result.m_RouteFinished = false;

		// 1) Долистать currentIndex вперёд, если мы уже проехали ближайшие точки.
		int idx = currentIndex;
		while (true)
		{
			vector wp = route.PointAt(idx);
			float distSq = GCVAI_Flat_DistanceSq(pos, wp);
			if (distSq > GCVAI_WAYPOINT_REACH_RADIUS * GCVAI_WAYPOINT_REACH_RADIUS) break;

			int next = route.NextIndex(idx);
			if (next == -1)
			{
				result.m_RouteFinished = true;
				result.m_TargetIndex = idx;
				result.m_Steer = 0;
				result.m_TargetSpeedKmh = 0;
				return result;
			}
			idx = next;
		}

		// 2) Look-ahead растёт со скоростью: на 50 км/ч смотрим дальше, чем на 10 км/ч.
		float speedFactor = Math.Clamp(speedKmh / profile.m_SpeedStraightKmh, 0, 1);
		float lookahead = Math.Lerp(profile.m_LookaheadMin, profile.m_LookaheadMax, speedFactor);

		// 3) Найти точку на маршруте на расстоянии ~lookahead впереди по ходу (идём по
		// отрезкам маршрута от idx, пока не наберём нужную дистанцию).
		vector targetPoint = route.PointAt(idx);
		float accumulated = 0;
		int scanIdx = idx;
		for (int guard = 0; guard < route.Count() + 1; guard++)
		{
			int nextIdx = route.NextIndex(scanIdx);
			if (nextIdx == -1) break;

			vector a = route.PointAt(scanIdx);
			vector b = route.PointAt(nextIdx);
			float segLen = GCVAI_Flat_Distance(a, b);

			if (accumulated + segLen >= lookahead)
			{
				float t = 0;
				if (segLen > 0.001) t = (lookahead - accumulated) / segLen;
				targetPoint = GCVAI_Flat_Lerp(a, b, t);
				break;
			}

			accumulated += segLen;
			scanIdx = nextIdx;
			targetPoint = b;
		}

		// 4) Классическая формула Pure Pursuit: угол между направлением машины и целью,
		// переведённый в кривизну поворота руля.
		vector toTarget = targetPoint - pos;
		toTarget[1] = 0;
		float targetDist = toTarget.Length();
		if (targetDist < 0.01) targetDist = 0.01;

		vector fwd = forward;
		fwd[1] = 0;
		fwd.Normalize();
		vector toTargetNorm = toTarget;
		toTargetNorm.Normalize();

		// Знак поворота — через "правый" вектор машины (перпендикуляр к forward в плоскости XZ).
		vector right = Vector(fwd[2], 0, -fwd[0]);
		float lateral = vector.Dot(toTargetNorm, right);   // -1..1, куда цель относительно курса
		float forwardDot = vector.Dot(toTargetNorm, fwd);  // -1..1, спереди или сзади цель

		// Кривизна пути по формуле Pure Pursuit: curvature = 2 * lateral_offset / lookahead^2.
		float curvature = 2.0 * lateral * targetDist / (lookahead * lookahead);
		float steer = Math.Clamp(curvature * 3.0, -1, 1); // 3.0 — эмпирический коэффициент чувствительности руля

		// Если цель оказалась почти позади (машину развернуло/снесло) — рулим в её сторону
		// на полную, без плавной формулы, чтобы быстрее вернуться на маршрут.
		if (forwardDot < 0)
		{
			if (lateral >= 0) steer = 1;
			else steer = -1;
		}

		result.m_Steer = steer;
		result.m_TargetIndex = idx;

		// 5) Скорость по кривизне: чем резче поворот, тем медленнее едем. Плюс тормозим
		// заранее (m_BrakeLeadM), если резкий поворот приближается, а не наступил ещё.
		float turnSharpness = Math.AbsFloat(curvature); // 0 = прямая, больше = резче
		float speedT = Math.Clamp(turnSharpness * 8.0, 0, 1);
		result.m_TargetSpeedKmh = Math.Lerp(profile.m_SpeedStraightKmh, profile.m_SpeedTurnKmh, speedT);

		return result;
	}
}

// --- Векторная геометрия в плоскости (игнорируем высоту Y — рельеф считает сама физика) ---

float GCVAI_Flat_DistanceSq(vector a, vector b)
{
	float dx = a[0] - b[0];
	float dz = a[2] - b[2];
	return dx * dx + dz * dz;
}

float GCVAI_Flat_Distance(vector a, vector b)
{
	return Math.Sqrt(GCVAI_Flat_DistanceSq(a, b));
}

vector GCVAI_Flat_Lerp(vector a, vector b, float t)
{
	return Vector(
		Math.Lerp(a[0], b[0], t),
		Math.Lerp(a[1], b[1], t),
		Math.Lerp(a[2], b[2], t)
	);
}
