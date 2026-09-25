// Общие константы мода GC Vehicle AI.

const string GCVAI_TAG     = "[GCVAI]";
const string GCVAI_VERSION = "0.1.0";

// Как часто пересчитывать управление (мс). 250 мс = 4 раза в секунду — этого достаточно
// для плавного вождения и не нагружает сервер на каждую машину.
const int GCVAI_TICK_MS = 250;

// Радиус поиска целевой точки на маршруте (Pure Pursuit look-ahead), метры.
// Больше — плавнее и "по-взрослому" срезает повороты, меньше — точнее по центру дороги,
// но дёргается на кочках. Подбирается опытным путём под конкретную машину.
const float GCVAI_LOOKAHEAD_MIN = 8;
const float GCVAI_LOOKAHEAD_MAX = 20;

// Порог, после которого точка маршрута считается "пройденной".
const float GCVAI_WAYPOINT_REACH_RADIUS = 5;

// Скорость (км/ч) по умолчанию на прямой и минимальная — в резком повороте.
const float GCVAI_SPEED_STRAIGHT_KMH = 45;
const float GCVAI_SPEED_TURN_KMH     = 15;

// Луч проверки препятствий: длина вперёд по курсу и высота над колёсами.
const float GCVAI_OBSTACLE_RAY_LEN    = 12;
const float GCVAI_OBSTACLE_RAY_HEIGHT = 0.6;

// Если машина не продвинулась по маршруту дольше этого времени (мс) — считаем застрявшей.
const int GCVAI_STUCK_TIMEOUT_MS = 8000;
// Сколько миллисекунд сдавать назад при застревании, прежде чем пробовать снова.
const int GCVAI_STUCK_REVERSE_MS = 1800;
// После скольки подряд неудачных попыток объехать — телепорт на следующую точку маршрута
// (страховка, чтобы конвой не завис намертво до рестарта).
const int GCVAI_STUCK_MAX_RETRIES = 3;

class GCVAI_Log
{
	static void Info(string msg)  { Print(GCVAI_TAG + " " + msg); }
	static void Warn(string msg)  { Print(GCVAI_TAG + " ВНИМАНИЕ: " + msg); }
	static void Error(string msg) { Print(GCVAI_TAG + " ОШИБКА: " + msg); }
}
