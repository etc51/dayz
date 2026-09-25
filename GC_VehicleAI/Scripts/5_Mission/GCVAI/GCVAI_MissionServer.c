// Точка входа мода: инициализация менеджера маршрутов и приём команд из чата.
// Чат-команды — самый простой способ управлять модом без отдельного UI; при желании
// потом заменить на админ-меню (например, интеграцию с VPPAdminTools или Community Online Tools).

modded class MissionServer
{
	protected ref GCVAI_RouteManager m_GCVAI_Routes;
	protected ref GCVAI_RouteRecorder m_GCVAI_Recorder;

	override void OnInit()
	{
		super.OnInit();

		m_GCVAI_Routes = new GCVAI_RouteManager;
		m_GCVAI_Routes.LoadAllRoutes();
		m_GCVAI_Recorder = new GCVAI_RouteRecorder;

		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(m_GCVAI_Routes.Cleanup, 30000, true);

		GCVAI_Log.Info("GC Vehicle AI " + GCVAI_VERSION + " инициализирован.");
	}

	// ⚠️ ТРЕБУЕТ ПРОВЕРКИ В WORKBENCH: подписка на реальные сообщения чата.
	// В ванильном DayZ обработка чата обычно идёт через Chat/ChatChannel или через
	// modded PlayerBase.OnChatMessage — в разных версиях/модах называется по-разному
	// (у GCWorldRU может уже быть своя система команд — тогда GCVAI_HandleChatCommand
	// стоит вызывать прямо оттуда, а не заводить второй параллельный обработчик).
	// Пример подключения, если в проекте используется стандартный OnChatMessage:
	//
	//   override bool OnChatMessage(PlayerIdentity sender, PlayerBase player, string message, int channel)
	//   {
	//       if (m_GCVAI_Routes && GCVAI_HandleChatCommand(player, message)) return true; // команда обработана, не показывать в чате
	//       return super.OnChatMessage(sender, player, message, channel);
	//   }
	//
	// Сигнатура OnChatMessage выше ориентировочная — свериться с фактическим методом
	// в MissionServer/MissionBase вашей версии DayZ.

	// Подключение к обработке команд чата. Если в проекте уже есть своя система команд
	// (как в основном моде Черноруссии) — лучше вызывать эти методы оттуда напрямую,
	// а не заводить второй параллельный обработчик чата.
	bool GCVAI_HandleChatCommand(PlayerBase sender, string text)
	{
		if (!GCVAI_IsAdmin(sender)) return false;
		if (text.IndexOf("!gcvai") != 0) return false;

		array<string> parts = new array<string>;
		text.Split(" ", parts);
		// parts[0] = "!gcvai"

		if (parts.Count() >= 3 && parts.Get(1) == "record" && parts.Get(2) == "start" && parts.Count() >= 4)
		{
			m_GCVAI_Recorder.StartRecording(sender, parts.Get(3));
			return true;
		}
		if (parts.Count() >= 3 && parts.Get(1) == "record" && parts.Get(2) == "stop")
		{
			m_GCVAI_Recorder.StopRecording();
			return true;
		}
		if (parts.Count() >= 3 && parts.Get(1) == "record" && parts.Get(2) == "loop")
		{
			m_GCVAI_Recorder.SetLoop(true);
			return true;
		}
		if (parts.Count() >= 4 && parts.Get(1) == "spawn")
		{
			m_GCVAI_Routes.SpawnOnRoute(parts.Get(2), parts.Get(3));
			return true;
		}
		if (parts.Count() >= 2 && parts.Get(1) == "routes")
		{
			array<string> names = m_GCVAI_Routes.GetRouteNames();
			GCVAI_Log.Info("Известные маршруты: " + names.ToStringConcat(", "));
			return true;
		}

		return false;
	}
}

// Проверка прав администратора.
// ⚠️ ЗАГЛУШКА: сейчас всегда возвращает false, то есть команды никому не доступны —
// это осознанная защита по умолчанию, чтобы не выдать AI-команды случайно всем игрокам.
// Перед использованием подключить реальную проверку прав вашего сервера, например:
//   - PermissionsFramework из Community Online Tools (COT) — там есть готовый API проверки
//     группы игрока по UID;
//   - или свой список UID администраторов в конфиге мода.
bool GCVAI_IsAdmin(PlayerBase player)
{
	if (!player || !player.GetIdentity()) return false;
	return false; // TODO: заменить на реальную проверку прав вашего сервера
}
