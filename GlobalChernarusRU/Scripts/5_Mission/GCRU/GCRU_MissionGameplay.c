modded class MissionGameplay
{
	protected bool m_GCRU_WelcomeShown;

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (!m_GCRU_WelcomeShown && GetGame().GetPlayer())
		{
			m_GCRU_WelcomeShown = true;
			string welcome = Widget.TranslateString("#STR_GCRU_WELCOME");
			GetGame().Chat(welcome, "colorAction");
			GCRU_Log.Info("Клиент загружен, версия " + GCRU_VERSION);
		}
	}
}
