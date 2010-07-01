package ru.novosoft.smsc.admin.archive_daemon.messages;


/**
 * Служебный класс для работы с ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */
public class EmptyMessage extends Message {
  public EmptyMessage() {
    super(Message.SMSC_BYTE_EMPTY_TYPE);
  }
}
