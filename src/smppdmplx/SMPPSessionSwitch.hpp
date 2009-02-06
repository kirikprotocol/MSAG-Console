#ifndef __SMPPDMPLX_SMPPSESSIONSWITCH_HPP__
# define __SMPPDMPLX_SMPPSESSIONSWITCH_HPP__

/*
** Ответственность класса-
** 1. хранение связки systemId -- сессия_с_SMSC
** 2. возврат сессии_с_SMSC по запросу на основании значения systemId
** 3. хранение связки smpp_сессия_c_SME -- smpp_сессия_с_SMC
** 4. возврат по запросу разделяемой сессии с SMSC. В запросе
**    указывается сессия, обслуживающия запросы от SME.
** 5. удаление связки smpp_сессия_c_SME -- smpp_сессия_с_SMC.
**    При удаленнии последней связки smpp_сессия_c_SME -- smpp_сессия_с_SMC 
**    для данной сессии_с_SMC, транспортное соединение с SMC 
**    должно закрываться.
** 6. возврат сессии для работы с SME (сокет_SME) по данной в запросе
**    сессии с SMSC. Сессии с SME могут возвращаться
**    в соответствии со стратегией round-robin, или как-то еще.
*/

# include <map>
# include <list>
# include <utility>

# include <logger/Logger.h>
# include <util/Singleton.hpp>
# include <smppdmplx/SMPPSession.hpp>
# include <smppdmplx/SMPP_message.hpp>
# include <smppdmplx/core_ax/network/Socket.hpp>

namespace smpp_dmplx {

class SMPPSessionSwitch : public smsc::util::Singleton<SMPPSessionSwitch> {
public:
  SMPPSessionSwitch();

  typedef std::pair<bool,SMPPSession> search_result_t;

  /*
  **  Получить разделяемую сессию с SMSC для для заданного значения 
  **  systemId. Если такая сессия существует, то атрибут first класса
  **  std::pair<bool,SMPPSession> принимает значение true.
  */
  search_result_t getSharedSessionToSMSC(const std::string& systemId);

  /*
  **  Создать разделяемую сессию с SMSC для заданного значения 
  **  systemId.
  */
  void setSharedSessionToSMSC(SMPPSession& sessionToSMSC, const std::string& systemId);

  /*
  **  Удалить разделяемую сессия из пула для заданного значения systemId.
  */
  void dropSharedSessionToSMSC(const std::string& systemId);

  /*
  ** Добавить в пул сессию_с_SME
  */
  void addActiveSmeSession(SMPPSession& sessionFromSme);

  /*
  ** Удалить сессию с SME из пула
  */
  void removeSmeSession(SMPPSession& sessionFromSme);

  /*
  ** Получить разделяемую сессию с SMSC. В запросе передается сессия с SME.
  ** Если в возвращаемой паре first == true, то в second хранится сессия, иначе
  ** связки сессия_с_SME -- сессия_с_SMSC не существует.
  */
  search_result_t
  getCrossedSession(SMPPSession& sessionFromSme);

  /*
  ** По значению systemId получить сессию для работы с SME. Для данного
  ** значение systemId может существовать несколько активных сессий с SME.
  ** Какая конкретно сессия выбирается определается стратегией. На текущий 
  ** момент гвоздями прибита стратегия round-robin.
  */
  search_result_t
  getCrossedSession(const std::string& systemId);

  void broadcastMessageToAllSme(const std::string& systemId, const SMPP_message& messageToSend);
private:
  smsc::logger::Logger* _log;
  /*
  ** Тип для хранения сессий с SME.
  */
  typedef std::list<SMPPSession> smeSessions_t;

  /* 
  ** Тип для получения статистики использования сессий с SME по
  ** значению идентификатора systemId.
  */
  typedef std::map<std::string, smeSessions_t*> sysIdToSmeList_t;
  sysIdToSmeList_t _sysIdToSmeList;

  /*
  ** Тип для хранения связки systemId -> сессия_с_SMSC
  */
  typedef std::map<std::string, SMPPSession> mapRegistredSysIdToSmscSession_t;
  mapRegistredSysIdToSmscSession_t _mapRegistredSysIdToSmscSession;

  //for debug only
  //void dumpRegistredSmeSession(const char* where, smeSessionUsageHistogram_t* sessionUsageHist);
};

}

#endif
