package ru.novosoft.smsc.admin.archive_daemon.messages;

import ru.novosoft.smsc.admin.archive_daemon.SmsQuery;


/**
 * Служебный класс для работы с ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */

public class CountMessage extends QueryMessage {

  public CountMessage(SmsQuery query) {
    super(Message.SMSC_BYTE_COUNT_TYPE, query);
  }

}
