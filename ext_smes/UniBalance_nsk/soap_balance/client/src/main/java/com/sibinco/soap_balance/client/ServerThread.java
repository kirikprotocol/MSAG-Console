package com.eyelinecom.whoisd.phones.tcp;

import com.eyelinecom.whoisd.sptp.SPTPServerThread;
import com.eyelinecom.whoisd.sptp.PDU;
import com.eyelinecom.whoisd.sptp.ProcessException;
import com.eyelinecom.whoisd.sptp.SPTPServer;
import com.eyelinecom.whoisd.phones.storage.StorageManager;

import java.net.Socket;
import java.io.IOException;

import org.apache.log4j.Category;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class ServerThread extends SPTPServerThread {
  private final static Category logger = Category.getInstance(ServerThread.class);
  private StorageManager storageManager;

  public ServerThread(SPTPServer server, Socket socket, StorageManager storageManager) throws IOException {
    super(server, socket);
    this.storageManager = storageManager;
  }

  protected void processPDU(PDU pdu) throws ProcessException {
    switch (pdu.getCommand()) {
      case Command.SEND_PING:
        returnResponse(Command.defaultResponse());
        break;
      case Command.SEND_ABONENT_DATA:
        SendAbonentData sendAbonentData = new SendAbonentData(pdu);
        storageManager.processRequest(sendAbonentData.getAbonent(), sendAbonentData.getUserAgent(), sendAbonentData.getUrlSpecification(), sendAbonentData.getDiffs());
        returnResponse(new SendAbonentDataResp());
        break;
      case Command.GET_ABONENT_PROFILE_ENTRY:
        GetAbonentProfileEntry getAbonentProfileEntry = new GetAbonentProfileEntry(pdu);
        String value = storageManager.getAbonentProfileEntry(getAbonentProfileEntry.getAbonent(), getAbonentProfileEntry.getEntryKey());
        GetAbonentProfileEntryResp resp = new GetAbonentProfileEntryResp();
        resp.setAbonent(getAbonentProfileEntry.getAbonent());
        resp.setEntryKey(getAbonentProfileEntry.getEntryKey());
        if(value==null){
          value="";
        }
        resp.setEntryValue(value);
        returnResponse(resp);
        break;
      default:
        logger.error("Unsupported command_id: " + pdu.getCommand());
        returnResponse(Command.defaultResponse());
    }
  }

  private void returnResponse(Command resp) {
    returnResponse(resp.getPdu());
  }

  private void returnResponse(PDU resp) {
    try {
      writePDU(resp);
    } catch (IOException e) {
      if (!stop) {
        logger.error("I/O Error: " + e, e);
      }
      stopService();
    }
  }
}
