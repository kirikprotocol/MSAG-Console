package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;

import java.sql.*;
import java.util.Random;

import oracle.jdbc.OracleTypes;

public class BalanceProcessor implements Runnable {
  private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(BalanceProcessor.class);

  private SmeEngine smeEngine = null;
  private RequestState state = null;

  public BalanceProcessor(SmeEngine smeEngine, RequestState state) {
    this.smeEngine = smeEngine;
    this.state = state;
  }

  public BalanceProcessor(SmeEngine smeEngine) {
    this.smeEngine = smeEngine;
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
    String balance = null;
    CallableStatement stmt = null;
    ResultSet rs = null;
    try {
      stmt = smeEngine.getCbossStatement();
      if (stmt == null) {
        logger.error("Couldn't get CBOSS statement");
        return null;
      }

      synchronized (stmt) {
        stmt.registerOutParameter(1, OracleTypes.INTEGER);
        //stmt.setString(2, cutAbonentAddress(abonent));
        stmt.setString(2, abonent);
        stmt.registerOutParameter(3, OracleTypes.VARCHAR);
        rs=stmt.executeQuery();
        if (rs.next()) {
          int result = stmt.getInt(1);
          if (result == 1) {
            balance = stmt.getString(3);
          } else {
            logger.warn("Abonent " + abonent + " CBOSS balance corrupted: " + result);
          }
        }
      }
    } catch (Exception temporalError) {
      try {
        if (rs != null)
          try {
            rs.close();
            rs = null;
          } catch (SQLException e1) {
            logger.warn("Could not close oracle ResultSet: " + e1);
          }
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
          stmt.registerOutParameter(1, OracleTypes.INTEGER);
          //stmt.setString(2, cutAbonentAddress(abonent));
          stmt.setString(2, abonent);
          stmt.registerOutParameter(3, OracleTypes.VARCHAR);
          rs=stmt.executeQuery();
          if (rs.next()) {
            int result = stmt.getInt(1);
            if (result == 1) {
              balance = stmt.getString(3);
            } else {
              logger.warn("Abonent " + abonent + " CBOSS balance corrupted: " + result);
            }
          }
        }
      } catch (SQLException permanentError) {
        logger.error("Could not get balance from CBOSS database: " + permanentError, permanentError);
        if (rs != null)
          try {
            rs.close();
            rs = null;
          } catch (SQLException e1) {
            logger.warn("Could not close oracle ResultSet: " + e1);
          }
        smeEngine.closeCbossStatement(stmt);
        return null;
      }
    } finally {
      if (rs != null)
        try {
          rs.close();
          rs = null;
        } catch (SQLException e1) {
          logger.warn("Could not close oracle ResultSet: " + e1);
        }
    }
    return new GetBalanceResult(balance);
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
      state.setError(true);
      if (logger.isInfoEnabled())
        logger.info("Can not get balance for " + state.getAbonentRequest().getSourceAddress());
      logger.error("Unexpected exception occured during processing request.", t);
    }
  }

}

