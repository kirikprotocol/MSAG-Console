package ru.novosoft.smsc.admin.smsview.archive;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsview.archive.*;
import ru.novosoft.smsc.admin.smsview.SmsSource;
import ru.novosoft.smsc.admin.smsview.SmsSet;
import ru.novosoft.smsc.admin.smsview.SmsQuery;

import java.net.Socket;
import java.io.*;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 27.01.2004
 * Time: 16:58:00
 * To change this template use Options | File Templates.
 */
public class SmsArchiveSource extends SmsSource
{
  private final static int MAX_SMS_FETCH_SIZE = 200;

  private final static String host = "smsc";
  private final static short  port = 6789;

  public SmsSet getSmsSet(SmsQuery query) throws AdminException
  {
    Socket socket = null;
    InputStream input = null;
    OutputStream output = null;

    SmsSet set = new SmsSet();
    int rowsMaximum = query.getRowsMaximum();
    if (rowsMaximum == 0) return set;

    try {
      QueryMessage request = new QueryMessage(query);

      socket = new Socket(host, port);
      input = socket.getInputStream(); output = socket.getOutputStream();

      DaemonCommunicator communicator = new DaemonCommunicator(input, output);
      communicator.send(request);
      int toReceive = (rowsMaximum < MAX_SMS_FETCH_SIZE) ? rowsMaximum:MAX_SMS_FETCH_SIZE;
      communicator.send(new RsNxtMessage(toReceive));

      Message responce = null;
      boolean allSelected = false;
      while(!allSelected)
      {
        responce = communicator.receive();
        if (responce == null) throw new AdminException("Message from archive daemon is NULL");

        switch(responce.getType())
        {
        case Message.SMSC_BYTE_EMPTY_TYPE:
            allSelected = true;
            break;
        case Message.SMSC_BYTE_RSSMS_TYPE:
            set.addRow(((RsSmsMessage)responce).getSms());
            if (--toReceive <= 0) {
              toReceive = rowsMaximum-set.getRowsCount();
              if (toReceive <= 0) {
                communicator.send(new EmptyMessage());
              } else {
                toReceive = (toReceive < MAX_SMS_FETCH_SIZE) ? toReceive:MAX_SMS_FETCH_SIZE;
                communicator.send(new RsNxtMessage(toReceive));
              }
            }
            break;
        case Message.SMSC_BYTE_ERROR_TYPE:
            throw new AdminException("Archive daemon communication error: "+
                                     ((ErrorMessage)responce).getError());
        default:
            throw new AdminException("Unsupported message received from archive daemon, type: "+
                                     responce.getType());
        }
      }

    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try { if (input  != null) input.close();  input=null;  } catch (Exception exc) {};
      try { if (output != null) output.close(); output=null; } catch (Exception exc) {};
      try { if (socket != null) socket.close(); socket=null; } catch (Exception exc) {};
    }

    return set;
  }

  public int getSmsCount(SmsQuery query) throws AdminException
  {
    Socket socket = null;
    InputStream input = null;
    OutputStream output = null;

    int smsCount = 0;
    try {
      CountMessage request = new CountMessage(query);

      socket = new Socket(host, port);
      input = socket.getInputStream(); output = socket.getOutputStream();

      DaemonCommunicator communicator = new DaemonCommunicator(input, output);
      communicator.send(request);

      Message responce = communicator.receive();
      if (responce == null)
        throw new AdminException("Message from archive daemon is NULL");

      switch(responce.getType())
      {
      case Message.SMSC_BYTE_EMPTY_TYPE:
          break;
      case Message.SMSC_BYTE_TOTAL_TYPE:
          smsCount = (int)((TotalMessage)responce).getCount();
          break;
      case Message.SMSC_BYTE_ERROR_TYPE:
          throw new AdminException("Archive daemon communication error: "+
                                   ((ErrorMessage)responce).getError());
      default:
          throw new AdminException("Unsupported message received from archive daemon, type: "+
                                   responce.getType());
      }
    } catch (Exception exc) {
      exc.printStackTrace();
      throw new AdminException(exc.getMessage());
    } finally {
      try { if (input  != null) input.close();  input=null;  } catch (Exception exc) {};
      try { if (output != null) output.close(); output=null; } catch (Exception exc) {};
      try { if (socket != null) socket.close(); socket=null; } catch (Exception exc) {};
    }
    return smsCount;
  }

}
