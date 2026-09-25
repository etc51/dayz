modded class PlayerBase
{
	override void OnConnect()
	{
		super.OnConnect();

		PlayerIdentity identity = GetIdentity();
		if (identity)
			GCRU_Log.Info("Игрок подключился: " + identity.GetName());
	}
}
