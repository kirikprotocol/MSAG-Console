package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;

import java.sql.CallableStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Random;

import oracle.jdbc.OracleTypes;

public class DummyBalanceProcessor implements Runnable {
  private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(DummyBalanceProcessor.class);

  private SmeEngine smeEngine = null;
  private RequestState state = null;

  public DummyBalanceProcessor(SmeEngine smeEngine, RequestState state) {
    this.smeEngine = smeEngine;
    this.state = state;
  }

  private void process() {
    if (state == null)
      return;

    String abonent = state.getAbonentRequest().getSourceAddress();

    GetBalanceResult balance = null;

    if (logger.isDebugEnabled())
      logger.debug("CBOSS request for abonent " + abonent);
    state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_CBOSS);
    balance = getCbossBalance(state.getAbonentRequest().getSourceAddress());
    state.setBillingSystemResponseTime(SmeEngine.BILLING_SYSTEM_CBOSS, System.currentTimeMillis());
    if (logger.isDebugEnabled()) {
      if (balance != null) {
        logger.debug("This is CBOSS abonent");
      } else {
        logger.debug("This is not CBOSS abonent");
      }
    }

    if (balance != null) {
      Message message = new Message();
      message.setSourceAddress(state.getAbonentRequest().getDestinationAddress());
      message.setDestinationAddress(state.getAbonentRequest().getSourceAddress());
      if (state.getAbonentRequest().hasUssdServiceOp()) {
        message.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
      }
      message.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
      message.setMessageString(smeEngine.getMessageFormat().format(new String[]{balance.getBalance()}));
      message.setType(Message.TYPE_SUBMIT);
      state.setAbonentResponse(message);
      state.setBalanceReady(true);
    } else {
      if (logger.isInfoEnabled())
        logger.info("Can not get balance for " + abonent);
      state.setError(true);
    }
    smeEngine.closeRequestState(state);
  }

  private GetBalanceResult getCbossBalance(String abonent) {
    Random rand=new Random();
    String balance = "Balans="+rand.nextInt(1000) +"r";
    try {
      Thread.sleep(rand.nextInt(10));
    } catch (InterruptedException e) {
      logger.error(e);
    }
    return new GetBalanceResult(balance);
  }

  public void run() {
    try {
      process();
    } catch (Throwable t) {
      state.setError(true);
      if (logger.isInfoEnabled())
        logger.info("Can not get balance for " + state.getAbonentRequest().getSourceAddress());
      logger.error("Unexpected exception occured during processing request.", t);
    }
  }

}
