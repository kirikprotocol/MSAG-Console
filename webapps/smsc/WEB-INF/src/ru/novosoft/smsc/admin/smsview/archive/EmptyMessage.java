package ru.novosoft.smsc.admin.smsview.archive;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.01.2004
 * Time: 21:13:48
 * To change this template use Options | File Templates.
 */
public class EmptyMessage extends Message
{
  public EmptyMessage() {
    super(Message.SMSC_BYTE_EMPTY_TYPE);
  }
}
