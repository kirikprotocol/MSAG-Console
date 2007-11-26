package com.sibinco.soap_balance.client_loading_test_tool;

import com.eyelinecom.whoisd.sptp.PDU;
import com.eyelinecom.whoisd.sptp.ProcessException;
import com.eyelinecom.whoisd.sptp.SPTPClient;
import com.eyelinecom.whoisd.util.InitializationException;
import com.sibinco.soap_balance.client.BalanceRequest;
import com.sibinco.soap_balance.client.BalanceResponse;
import com.sibinco.soap_balance.client.Command;
import org.apache.log4j.Category;

import java.io.IOException;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class Client {
  private final static Category logger = Category.getInstance(Client.class);
  private SPTPClient client;
  private long lastUsed = System.currentTimeMillis();

  public Client(String serverHost, String serverPort) throws InitializationException {
    client = new SPTPClient(serverHost, serverPort);
    if (!client.connect()) {
      throw new InitializationException("Connect to server " + serverHost + ":" + serverPort + " failed!");
    }
  }

  public double getBalance(String abonent) throws ClientException {
    lastUsed = System.currentTimeMillis();
    BalanceRequest request = new BalanceRequest();
    request.setAbonent(abonent);
    try {
      PDU pduResp=client.sendRequest(request.getPdu());
      if(pduResp.getCommand()!= Command.GET_BALANCE_RESP){
        throw new ClientException(ClientException.SERVER_RESPONSE_ERROR, "Unexpected pdu response command "+pduResp.getCommand()+" for request command "+request.getCommandId());
      }
      BalanceResponse resp;
      try {
        resp = new BalanceResponse(pduResp);
      } catch (ProcessException e) {
        throw new ClientException(ClientException.SERVER_RESPONSE_ERROR, "Error while processing pdu response command "+pduResp.getCommand(),e);
      }
      if(!resp.getAbonent().equals(request.getAbonent())){
        throw new ClientException(ClientException.SERVER_RESPONSE_ERROR, "Unexpected response abonent values ("+resp.getAbonent()+", " + resp.getBalance() + ") for request ("+request.getAbonent() + ")" );
      }
      return resp.getBalance();
    } catch (IOException e) {
      throw new ClientException(ClientException.CONNECT_FAILED, e);
    }
  }

  public boolean checkConnection(){
    PDU request=new PDU(Command.SEND_PING);
    try {
      client.writePDU(request);
      return true;
    } catch (IOException e) {
      logger.error(e.getMessage(), e);
      return false;
    }
  }

  public void close(){
    shutdown();
  }

  public void shutdown(){
    client.close();
  }

  /**
   * Last client usage time
   *
   * @return
   */
  public long getLastUsed() {
    return lastUsed;
  }

}
