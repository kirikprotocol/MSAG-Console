var portErrorMsg = "Порт должен быть целым числом в диапазоне от -1 до ";   //portErrorMsg+0xFFFF
var maskErrorMsg = "Неправильная маска";
var idErrorMsg   = "Неправильный идентификатор";
var addressErrorMsg = "Неправильный адресс";
var addrPreErrorMsg = "Неправильный префикс адресса";
var selectErrorMsg = "Должно быть выбрано значение";
var nonEmptyErrorMsg = "Поле не должно быть пустым";
var valueErrorMsg = "Значение должно быть целым положительным числом";
var valueSignErrorMsg = "Значение должно быть десятичным без знака";
var policyErrorMsg = "Неправильная политика перепланировки";
var nonLanguageErrorMsg = "Неправильный язык. Установлен по умолчанию.";

var priorityErrorMsg = "Значение приоритета должно быть целым положительным числом меньше чем "; // priorityErrorMsg + (0x10000/2)
var servIDErrorMsg = "Значение идентификатора сервиса должно быть целым положительным числом";

var rangeValueErrorMsg_pre = "значение должно быть целым числом из диапазона ["; // rangeValueErrorMsg_pre+elem.range_min+", "+elem.range_max+"]"

var causePositiveErrorMsg = "Release cause value must be a positive integer";
var causeNullValueErrorMsg = "Invalid release cause: value is null";
var causeValueErrorMsg_pre = "Invalid release cause: value '"; // causeValueErrorMsg_pre+intValue+causeValueErrorMsg_post
var causeValueErrorMsg_post = "' is undefined";

var unknownValidationTypeErrorMsg = "тип ошибки неизвестен";
var nameAlreadyUsed="Это имя уже используется";
var alphanumerical="Вы можете использовать только символы алфавита, цифры и точки, чтобы разделять их.";
var percentErrorMsg="Значение должно лежать в интервале от 0 до 100.";