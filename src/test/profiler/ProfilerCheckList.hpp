#ifndef TEST_PROFILER_PROFILER_CHECK_LIST
#define TEST_PROFILER_PROFILER_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace profiler {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
  registerTc(id, desc)

class ProfilerCheckList : public CheckList
{
public:
  ProfilerCheckList()
    : CheckList("Результаты функционального тестирования Profiler", "profiler.chk")
  {
    //createProfileMatch
    __reg_tc__("createProfileMatch",
      "Создание нового профиля для фиксированного адреса");
    __reg_tc__("createProfileMatch.noSubstSymbols",
      "Маска в точности соответствует адресу (без знаков подстановки)");
    /*
    __reg_tc__("createProfileMatch.withQuestionMarks",
      "Маска соответствует адресу с одним или несколькими '?' в конце");
    __reg_tc__("createProfileMatch.entirelyQuestionMarks",
      "Маска целиком из '?', длина соответствует исходному адресу");
    */
    //createProfileNotMatch
    __reg_tc__("createProfileNotMatch",
      "Создание нового профиля с отличиями для фиксированного адреса");
    __reg_tc__("createProfileNotMatch.diffType",
      "В маске отличается typeOfNumber");
    __reg_tc__("createProfileNotMatch.diffPlan",
      "В маске отличается numberingPlan");
    __reg_tc__("createProfileNotMatch.diffValue",
      "В строке адреса маски отличается один символ, но длина строки соответствует");
    /*
    __reg_tc__("createProfileNotMatch.diffValueLength",
      "В строке адреса маски все символы соответствуют исходным, но количество '?' делает строку отличающейся длины");
    */
    //other
    __reg_tc__("updateProfile", "Обновление уже существующего профиля");
    __reg_tc__("lookup", "Поиск профиля для заданного адреса");
    //putCommand
    __reg_tc__("putCommand", "Управление менеджером профилей через deliver_sm pdu");
    __reg_tc__("putCommand.cmdTextDefault",
      "Текст команды в дефолтной кодировке SC");
    __reg_tc__("putCommand.cmdTextUcs2",
      "Текст команды в кодировке UCS2");
    /*
    __reg_tc__("putCommand.cmdTextInvalidDataCoding",
      "Недопустимая кодировка текста команды");
    */
    __reg_tc__("putCommand.reportOptions",
      "Изменение опций уведомления о доставке");
    __reg_tc__("putCommand.reportOptions.reportNoneMixedCase",
      "Отказаться от уведомления о доставке (текст команды в смешанном регистре)");
    __reg_tc__("putCommand.reportOptions.reportNoneSpaces",
      "Отказаться от уведомления о доставке (текст команды с лишними пробелами)");
    __reg_tc__("putCommand.reportOptions.reportFullMixedCase",
      "Установить прием уведомлений о доставке (текст команды в смешанном регистре)");
    __reg_tc__("putCommand.reportOptions.reportFullSpaces",
      "Установить прием уведомлений о доставке (текст команды с лишними пробелами)");
    __reg_tc__("putCommand.dataCoding",
      "Изменение режима приема сообщений");
    __reg_tc__("putCommand.dataCoding.ucs2CodepageMixedCase",
      "Установить режим приема сообщений на русском языке (кодировка ucs2, текст команды в смешанном регистре)");
    __reg_tc__("putCommand.dataCoding.ucs2CodepageSpaces",
      "Установить режим приема сообщений на русском языке (кодировка ucs2, текст команды с лишними пробелами)");
    __reg_tc__("putCommand.dataCoding.defaultCodepageMixedCase",
      "Отказаться от режима приема сообщений на русском языке (текст команды в смешанном регистре)");
    __reg_tc__("putCommand.dataCoding.defaultCodepageSpaces",
      "Отказаться от режима приема сообщений на русском языке (текст команды с лишними пробелами)");
    __reg_tc__("putCommand.incorrectCmdText",
      "Неправильный текст команды");
    //getCommand
    __reg_tc__("getCommand", "Получение ответов от менеджера профилей");
    __reg_tc__("getCommand.submit",
      "Получение submit_sm pdu со статусом выполнения операции обновления профиля");
    __reg_tc__("getCommand.submit.checkDialogId",
      "Правильное значение dialogId");
    __reg_tc__("getCommand.submit.checkFields",
      "Правильные значения полей sms (включая текст сообщения)");
    __reg_tc__("getCommand.deliverResp",
      "Получение deliver_sm_resp pdu в ответ на submit_sm");
  }
};

}
}
}

#endif /* TEST_PROFILER_PROFILER_CHECK_LIST */
