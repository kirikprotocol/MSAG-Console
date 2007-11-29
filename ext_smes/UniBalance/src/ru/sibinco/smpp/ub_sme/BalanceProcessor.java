package ru.sibinco.smpp.ub_sme;

public class BalanceProcessor implements Runnable {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(BalanceProcessor.class);

    private SmeEngine smeEngine = null;
    private RequestState state = null;

    public BalanceProcessor(SmeEngine smeEngine, RequestState state) {
        this.smeEngine = smeEngine;
        this.state = state;
    }
/*
  private void process() {
    if (state == null)
      return;

    String balance = null;
    String abonent = state.getAbonentRequest().getSourceAddress();

    int currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());

    int loopDetector = 0;
    while (state.getCurrentBillingSystemIndex() < smeEngine.getBillingSystemCount(state.getAbonentContractType())) {
      if (loopDetector == 10) {
        logger.error("Breaking on loop_detector=" + loopDetector);
        break;
      }
      loopDetector++;

          if (state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_FORIS_MG)) {
            state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
            currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
          } else {
            if (logger.isDebugEnabled())
              logger.debug("FORIS request for abonent " + abonent);
            Message abonentRequest = new Message();
            abonentRequest.setSourceAddress(state.getAbonentRequest().getSourceAddress());
            abonentRequest.setDestinationAddress(smeEngine.getMgAddress());
            abonentRequest.setMessageString("");
            abonentRequest.setEsmClass((byte) (Data.SM_FORWARD_MODE));
            abonentRequest.setType(Message.TYPE_SUBMIT);
            state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_FORIS_MG);
            smeEngine.sendMgRequest(state, abonentRequest);
            return;
          }
          break;
      }

    if (balance != null) {
      Message abonentRequest = new Message();
      abonentRequest.setSourceAddress(state.getAbonentRequest().getDestinationAddress());
      abonentRequest.setDestinationAddress(state.getAbonentRequest().getSourceAddress());
      if (state.getAbonentRequest().hasUssdServiceOp()) {
        abonentRequest.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
      }
      abonentRequest.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
      abonentRequest.setMessageString(balance);
      abonentRequest.setType(Message.TYPE_SUBMIT);
      state.setAbonentResponse(abonentRequest);
      synchronized (state) {
        state.setBalanceReady(true);
      }
    } else {
      if (logger.isInfoEnabled())
        logger.info("Can not get balance for " + abonent);
      synchronized (state) {
        state.setError(true);
      }
    }
    smeEngine.closeRequestState(state);
  }

  private String getCbossBalance(String abonent) {
    double balance = Double.NaN;
    //long balanceDate = O;
    String currency = null;
    CallableStatement stmt = null;
    try {
      stmt = smeEngine.getCbossStatement();
      if (stmt == null) {
        logger.error("Couldn't get CBOSS statement");
        return null;
      }
      synchronized (stmt) {
        stmt.registerOutParameter(1, java.sql.Types.VARCHAR);
        stmt.setString(2, cutAbonentAddress(abonent));
        stmt.registerOutParameter(3, java.sql.Types.DATE);
        stmt.registerOutParameter(4, java.sql.Types.VARCHAR);
        stmt.registerOutParameter(5, java.sql.Types.VARCHAR);
        stmt.execute();
        if (stmt.getString(4).equalsIgnoreCase("ACCURATE") || stmt.getString(4).equalsIgnoreCase("CACHED")) {
          balance = Double.parseDouble(stmt.getString(1));
          currency = smeEngine.getCurrency(stmt.getString(5));
          //balanceDate = stmt.getDate(3).getTime();
        } else {
          logger.warn("Abonent " + abonent + " CBOSS balance corrupted: " + stmt.getString(4));
          return null;
        }
      }
    } catch (Exception temporalError) {
      try {
        smeEngine.closeCbossStatement(stmt);
        if (!smeEngine.isCbossConnectionError(temporalError)) {
          if (temporalError instanceof SQLException) {
            throw (SQLException) temporalError;
          } else {
            logger.error("Unexpected exception: " + temporalError, temporalError);
            throw new SQLException(temporalError.getMessage());
          }
        }
        stmt = smeEngine.getCbossStatement();
        if (stmt == null) {
          logger.error("Couldn't get CBOSS statement");
          return null;
        }
        synchronized (stmt) {
          stmt.registerOutParameter(1, java.sql.Types.VARCHAR);
          stmt.setString(2, cutAbonentAddress(abonent));
          stmt.registerOutParameter(3, java.sql.Types.DATE);
          stmt.registerOutParameter(4, java.sql.Types.VARCHAR);
          stmt.registerOutParameter(5, java.sql.Types.VARCHAR);
          stmt.execute();
          if (stmt.getString(4).equalsIgnoreCase("ACCURATE") || stmt.getString(4).equalsIgnoreCase("CACHED")) {
            balance = Double.parseDouble(stmt.getString(1));
            currency = smeEngine.getCurrency(stmt.getString(5));
            //balanceDate = stmt.getDate(3).getTime();
          } else {
            logger.warn("Abonent " + abonent + " CBOSS balance corrupted: " + stmt.getString(4));
            return null;
          }
        }
      } catch (SQLException permanentError) {
        logger.error("Could not get balance from CBOSS database: " + permanentError, permanentError);
        smeEngine.closeCbossStatement(stmt);
        return null;
      }
    }

    if (currency == null) {
      currency = smeEngine.getCurrency("default");
    }
    MessageFormat messageFormat = smeEngine.getMessageFormat(balance);
    return messageFormat.format(new String[]{smeEngine.getNumberFormat(balance).format(balance), currency});
  }

    private static String cutAbonentAddress(String abonent) {
    if (abonent.length() <= 10) {
      return abonent;
    } else {
      return abonent.substring(abonent.length() - 10);
    }
  }

  public void run() {
    try {
      process();
    } catch (Throwable t) {
      synchronized (state) {
        state.setError(true);
      }
      if (logger.isInfoEnabled())
        logger.info("Can not get balance for " + state.getAbonentRequest().getSourceAddress());
      logger.error("Unexpected exception occured during processing request.", t);
    }
  }
  */

    public void run() {

    }
}

