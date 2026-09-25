// Запись маршрута прямо в игре: админ садится за руль, включает запись командой в чате,
// едет нужным путём (нормальной скоростью, машина сама потом поедет так же), выключает
// запись — маршрут сохраняется в profiles/GCVAI/routes/<имя>.xml и сразу доступен без
// пересборки PBO (RouteManager подхватывает эту папку тоже).
//
// Команды в общем чате (только админ, см. GCVAI_IsAdmin):
//   !gcvai record start <имя_маршрута>
//   !gcvai record stop
//   !gcvai record loop        — пометить текущую запись как кольцевую (после stop не сработает)
//   !gcvai spawn <имя_маршрута> <ClassName>

class GCVAI_RouteRecorder
{
	protected bool m_Recording;
	protected ref GCVAI_Route m_Current;
	protected PlayerBase m_RecordingPlayer;
	protected vector m_LastRecordedPos;

	// Записываем новую точку, если игрок отъехал от последней записанной дальше этого
	// расстояния — иначе маршрут на прямой дороге получится из тысяч почти одинаковых точек.
	const float GCVAI_RECORD_MIN_STEP = 8;

	void StartRecording(PlayerBase admin, string routeName)
	{
		m_Recording = true;
		m_RecordingPlayer = admin;
		m_Current = new GCVAI_Route;
		m_Current.m_Name = routeName;
		m_Current.m_Loop = false;
		m_LastRecordedPos = "0 0 0";

		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, 500, true);
		GCVAI_Log.Info("Запись маршрута '" + routeName + "' начата игроком " + admin.GetIdentity().GetName());
	}

	void SetLoop(bool loop)
	{
		if (m_Current) m_Current.m_Loop = loop;
	}

	void StopRecording()
	{
		if (!m_Recording) return;
		m_Recording = false;
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Tick);

		if (m_Current.Count() < 2)
		{
			GCVAI_Log.Error("Запись остановлена: меньше 2 точек, файл не сохранён.");
			return;
		}

		string dir = "$profile:GCVAI/routes/";
		MakeDirectory(dir);
		string path = dir + m_Current.m_Name + ".xml";
		if (m_Current.SaveToFile(path))
			GCVAI_Log.Info("Маршрут сохранён: " + path + " (" + m_Current.Count().ToString() + " точек).");
	}

	protected void Tick()
	{
		if (!m_Recording || !m_RecordingPlayer) return;

		vector pos = m_RecordingPlayer.GetPosition();
		if (GCVAI_Flat_Distance(pos, m_LastRecordedPos) < GCVAI_RECORD_MIN_STEP) return;

		m_Current.m_Points.Insert(new GCVAI_RoutePoint(pos, 0));
		m_LastRecordedPos = pos;
	}
}
