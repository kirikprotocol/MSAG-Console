package com.sibinco.soap_balance.client;

import com.eyelinecom.whoisd.sptp.PDU;
import com.eyelinecom.whoisd.sptp.ProcessException;
import com.eyelinecom.whoisd.sptp.SPTPServer;
import com.eyelinecom.whoisd.sptp.SPTPServerThread;
import com.sibinco.soap_balance.service.BalanceService;
import org.apache.log4j.Category;

import java.io.IOException;
import java.net.Socket;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class ServerThread extends SPTPServerThread {
  private final static Category logger = Category.getInstance(ServerThread.class);
  private BalanceService service;

  public ServerThread(SPTPServer server, Socket socket, BalanceService service) throws IOException {
    super(server, socket);
    this.service = service;
  }

  protected void processPDU(PDU pdu) throws ProcessException {
    switch (pdu.getCommand()) {
      case Command.SEND_PING:
        returnResponse(Command.defaultResponse());
        break;
      case Command.GET_BALANCE:
        BalanceRequest balance_request = new BalanceRequest(pdu);
        String abonent = balance_request.getAbonent();
        //todo retrieve abonent from pdu
        double balance = service.getBalance(abonent);
        //todo send balance in pdu
        BalanceResponse balance_response = new BalanceResponse(pdu);
        balance_response.setAbonent(abonent);
        balance_response.setBalance(balance);
        returnResponse(balance_response);
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
