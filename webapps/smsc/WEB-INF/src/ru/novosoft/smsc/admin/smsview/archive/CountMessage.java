package ru.novosoft.smsc.admin.smsview.archive;

import ru.novosoft.smsc.admin.smsview.SmsQuery;

import java.io.IOException;
import java.io.OutputStream;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.01.2004
 * Time: 20:17:29
 * To change this template use Options | File Templates.
 */
public class CountMessage extends QueryMessage
{
  public CountMessage(SmsQuery query) {
    super(Message.SMSC_BYTE_COUNT_TYPE, query);
  }

  public void send(OutputStream os) throws IOException {
    super.send(os);
  };

}
