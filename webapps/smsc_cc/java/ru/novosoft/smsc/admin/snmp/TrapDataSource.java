package ru.novosoft.smsc.admin.snmp;

import ru.novosoft.smsc.admin.AdminException;

import java.util.Date;

/**
 * Хранилище SNMP-записей
 * @author Aleksandr Khalitov
 */

interface TrapDataSource {

  /**
   * Возвращает записи за данный и предыдущий дни
   * @param from начиная с даты
   * @param till по
   * @param visitor "посетитель" записей
   * @throws AdminException ошибка исполнения запроса
   */
  void getTraps(Date from, Date till, SnmpTrapVisitor visitor) throws AdminException;

}
