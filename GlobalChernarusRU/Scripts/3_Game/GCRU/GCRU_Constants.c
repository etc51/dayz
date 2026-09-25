// Общие константы мода «Глобальная Черноруссия (RU)».
const string GCRU_VERSION = "0.1.0";
const string GCRU_TAG = "[GCRU]";

class GCRU_Log
{
	static void Info(string msg)
	{
		Print(GCRU_TAG + " " + msg);
	}
}
