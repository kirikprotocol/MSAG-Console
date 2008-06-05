package ru.sibinco.smpp.ub_sme.inbalance.test;

import ru.sibinco.smpp.ub_sme.inbalance.*;
import ru.sibinco.smpp.ub_sme.InitializationException;
import ru.sibinco.smpp.ub_sme.RequestState;
import ru.sibinco.smpp.ub_sme.BalanceProcessor;
import ru.sibinco.smpp.ub_sme.inman.InManPDUException;
import org.apache.log4j.Category;

import java.util.Properties;
import java.io.IOException;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 08.11.2007
 * Time: 14:47:09
 * To change this template use File | Settings | File Templates.
 */
public class TestApp implements InBalancePDUHandler {
  private final static Category logger = Category.getInstance(TestApp.class);

  private String inBalanceHost = "localhost";
  private String inBalancePort = "9687";
  private String inBalanceUssData = "100";
  private byte inBalanceIN_SSN = (byte) 147;

  private InBalanceClient inBalanceClient = null;

  public void init() throws InitializationException {
    Properties config;
    try {
      config = new Properties();
      config.load(this.getClass().getClassLoader().getResourceAsStream("sme.properties"));
    } catch (IOException e) {
      logger.error("Exception occured during loading global configuration.", e);
      throw new InitializationException("Exception occured during loading global configuration.", e);
    }

    if (logger.isDebugEnabled()) logger.debug("UniBalance SME init started");

    inBalanceHost = config.getProperty("inbalance.host", inBalanceHost);
    if (inBalanceHost.length() == 0) {
      throw new InitializationException("Mandatory config parameter \"inbalance.host\" is missed");
    }
    inBalancePort = config.getProperty("inbalance.port", inBalancePort);
    if (inBalancePort.length() == 0) {
      throw new InitializationException("Mandatory config parameter \"inbalance.port\" is missed");
    }
    inBalanceUssData = config.getProperty("inbalance.uss.data", inBalanceUssData);
    if (inBalanceUssData.length() == 0) {
      throw new InitializationException("Mandatory config parameter \"inbalance.uss.data\" is missed");
    }
    try {
      inBalanceIN_SSN = (byte) Integer.parseInt(config.getProperty("inbalance.in.ssn", Byte.toString(inBalanceIN_SSN)));
    } catch (NumberFormatException e) {
      throw new InitializationException("Mandatory config parameter \"inbalance.in.ssn\" is invalid");
    }

    inBalanceClient = new InBalanceClient(inBalanceHost, inBalancePort, this);
    inBalanceClient.connect();


    if (logger.isDebugEnabled()) logger.debug("UniBalance SME init fineshed");
  }

  public void requestInBalance(String IN_ISDN, String msisdn) throws InBalanceClientException {
    int requestId = inBalanceClient.assignRequestId();
    if (logger.isDebugEnabled())
      logger.debug("Send InBalance request for abonent " + msisdn + ", sn=" + requestId);

    inBalanceClient.sendBalanceRequest(msisdn, inBalanceUssData, inBalanceIN_SSN, IN_ISDN, requestId);
  }

  public static void main(String[] args) {
    TestApp inst = new TestApp();
    inst.init();

    String IN_ISDN;
    if (args.length == 0) {
      System.out.println("Ussage java TestApp [IN_ISDN] [abonent1] ([abonent2] [abonent3]...)");
      return;
    }
    IN_ISDN = args[0];
    for (int i = 1; i < args.length; i++) {
      String arg = args[i];
      try {
        inst.requestInBalance(IN_ISDN, arg);
      } catch (InBalanceClientException e) {
        logger.error("InBalance Client error: " + e, e);
        break;
      }
      inst.sent++;
    }
    while (inst.sent != inst.received) {
      logger.debug("Wait for responses...");
      try {
        Thread.sleep(1000);
      } catch (InterruptedException e) {
        logger.error(e.getMessage(), e);
        break;
      }
    }

    logger.debug("Stop TestApp");
    inst.inBalanceClient.close();
  }

  int sent = 0;
  int received = 0;

  public void handleInBalancePDU(InBalancePDU pdu) {
    if (pdu.getCmdId() != InBalanceResult.CMD_ID) {
      logger.error("Unexpected InBalance PDU: " + pdu);
      return;
    }
    InBalanceResult result = (InBalanceResult) pdu;
    logger.debug("Got InBalance response: " + result);
    received++;
  }

}


