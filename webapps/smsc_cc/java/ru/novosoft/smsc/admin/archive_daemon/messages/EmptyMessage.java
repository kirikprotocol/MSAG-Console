package ru.novosoft.smsc.admin.archive_daemon.messages;


/**
 * ��������� ����� ��� ������ � ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */
public class EmptyMessage extends Message {
  public EmptyMessage() {
    super(Message.SMSC_BYTE_EMPTY_TYPE);
  }
}