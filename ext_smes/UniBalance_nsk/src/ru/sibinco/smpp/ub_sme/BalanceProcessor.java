package ru.sibinco.smpp.ub_sme;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smpp.ub_sme.inman.AbonentContractResult;
import ru.sibinco.smpp.ub_sme.inman.PDUException;

import java.sql.*;
import java.math.BigDecimal;

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

    AbonentContractResult inManContractResult = state.getInManContractResult();
    if (smeEngine.isBillingSystemEnabled(SmeEngine.BILLING_SYSTEM_IN_MAN) && !state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_MAN))
    {
      state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_MAN);
      smeEngine.requestAbonentContractType(state);
      return;
    }
    GetBalanceResult balance = null;
    byte contractType = AbonentContractResult.CONTRACT_UNKNOWN;
    if (inManContractResult != null) {
      try {
        contractType = inManContractResult.getContractType();
      } catch (PDUException e) {
        logger.error("InMan result PDU Exception: " + e, e);
        return;
      }
    }
    if (logger.isDebugEnabled()) {
      switch (contractType) {
        case AbonentContractResult.CONTRACT_PREPAID:
          logger.debug("This is PREPAID abonent contract");
          break;
        case AbonentContractResult.CONTRACT_POSTPAID:
          logger.debug("This is POSTPAID abonent contract");
          break;
        case AbonentContractResult.CONTRACT_UNKNOWN:
          logger.debug("Abonent contract type unknown");
          break;
      }
    }
    if (contractType != AbonentContractResult.CONTRACT_POSTPAID &&
        !state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX)) {
      if (logger.isDebugEnabled())
        logger.debug("InMan request for abonent " + abonent);
      state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX);
      balance = getInManBalance(state.getAbonentRequest().getSourceAddress());
      state.setBillingSystemResponseTime(SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX, System.currentTimeMillis());
      if (logger.isDebugEnabled()) {
        if (balance != null) {
          logger.debug("This is InMan abonent");
        } else {
          logger.debug("This is not InMan abonent");
        }
      }
    }
    if (balance == null) {
      if (smeEngine.isBillingSystemEnabled(SmeEngine.BILLING_SYSTEM_CBOSS) &&
          (smeEngine.getBillingSystemOrder(SmeEngine.BILLING_SYSTEM_CBOSS) < smeEngine.getBillingSystemOrder(SmeEngine.BILLING_SYSTEM_FORIS)
              || state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_FORIS))) {
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
      }
      if (balance == null && smeEngine.isBillingSystemEnabled(SmeEngine.BILLING_SYSTEM_FORIS)) {
        if (logger.isDebugEnabled())
          logger.debug("FORIS request for abonent " + abonent);
        Message message = new Message();
        message.setSourceAddress(state.getAbonentRequest().getSourceAddress());
        message.setDestinationAddress(smeEngine.getMgAddress());
        message.setMessageString("");
        message.setEsmClass((byte) (Data.SM_FORWARD_MODE));
        message.setType(Message.TYPE_SUBMIT);
        state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_FORIS);
        smeEngine.sendMgRequest(state, message);
        return;
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
      String currency = balance.getCurrency();
      if (currency == null) {
        currency = smeEngine.getCurrency("default");
      }
      message.setMessageString(smeEngine.getMessageFormat().format(new String[]{balance.getBalance(), currency}));
      message.setType(Message.TYPE_SUBMIT);
      state.setAbonentResponse(message);
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

  private GetBalanceResult getCbossBalance(String abonent) {
    String balance = null;
    String currency = null;
    long balanceDate = 0L;
    Connection connection = null;
    CallableStatement stmt = null;
    try {
      //connection = smeEngine.getCbossConnection();
      //stmt = connection.prepareCall(smeEngine.getCbossQuery());
      stmt = smeEngine.getCbossStatement();
      if (stmt == null) {
        logger.error("Couldn't get CBOSS statement");
        return null;
      }
      connection = stmt.getConnection();
      synchronized (stmt) {
        stmt.registerOutParameter(1, java.sql.Types.VARCHAR);
        stmt.setString(2, cutAbonentAddress(abonent));
        stmt.registerOutParameter(3, java.sql.Types.DATE);
        stmt.registerOutParameter(4, java.sql.Types.VARCHAR);
        stmt.registerOutParameter(5, java.sql.Types.VARCHAR);
        stmt.execute();
        if (stmt.getString(4).equalsIgnoreCase("ACCURATE") || stmt.getString(4).equalsIgnoreCase("CACHED")) {
          balance = smeEngine.getNumberFormat().format(Double.parseDouble(stmt.getString(1)));
          currency = smeEngine.getCurrency(stmt.getString(5));
          balanceDate = stmt.getDate(3).getTime();
        } else {
          logger.warn("Abonent " + abonent + " CBOSS balance corrupted: " + stmt.getString(4));
          return null;
        }
      }
    } catch (Exception temporalError) {
      try {
        if (stmt != null)
          try {
            stmt.close();
            stmt = null;
          } catch (Exception e1) {
            logger.warn("Could not close oracle CallableStatement: "+ e1);
          }
        if (connection != null)
          try {
            connection.close();
            connection = null;
          } catch (Exception e1) {
            logger.warn("Could not close oracle Connection: " + e1);
          }
        if (!smeEngine.isCbossConnectionError(temporalError)) {
          if(temporalError instanceof SQLException) {
            throw (SQLException)temporalError;
          } else {
            logger.error("Unexpected exception: "+temporalError, temporalError);
            throw new SQLException(temporalError.getMessage());
          }
        }
        stmt = smeEngine.getCbossStatement();
        if (stmt == null) {
          logger.error("Couldn't get CBOSS statement");
          return null;
        }
        connection = stmt.getConnection();
        synchronized (stmt) {
          stmt.registerOutParameter(1, java.sql.Types.VARCHAR);
          stmt.setString(2, cutAbonentAddress(abonent));
          stmt.registerOutParameter(3, java.sql.Types.DATE);
          stmt.registerOutParameter(4, java.sql.Types.VARCHAR);
          stmt.registerOutParameter(5, java.sql.Types.VARCHAR);
          stmt.execute();
          if (stmt.getString(4).equalsIgnoreCase("ACCURATE") || stmt.getString(4).equalsIgnoreCase("CACHED")) {
            balance = smeEngine.getNumberFormat().format(Double.parseDouble(stmt.getString(1)));
            currency = smeEngine.getCurrency(stmt.getString(5));
            balanceDate = stmt.getDate(3).getTime();
          } else {
            logger.warn("Abonent " + abonent + " CBOSS balance corrupted: " + stmt.getString(4));
            return null;
          }
        }
      } catch (SQLException permanentError) {
        logger.error("Could not get balance from CBOSS database: " + permanentError, permanentError);
        if (stmt != null)
          try {
            stmt.close();
          } catch (Exception e1) {
            logger.warn("Could not close CallableStatement.", e1);
          }
        if (connection != null)
          try {
            connection.close();
          } catch (Exception e1) {
            logger.warn("Could not close Connection.", e1);
          }
        return null;
      }
    }
    return new GetBalanceResult(balance, currency, balanceDate);
  }

  private GetBalanceResult getInManBalance(String abonent) {
    GetBalanceResult result = null;
    Double balance = null;
    String currency = null;
    Connection connection = null;
    CallableStatement stmt = null;
    ResultSet rs = null;
    try {
      //connection = smeEngine.getInManConnection();
      //stmt = connection.prepareCall(smeEngine.getInManQuery());
      stmt = smeEngine.getInManStatement();
      if (stmt == null) {
        logger.error("Couldn't get InMan statement");
        return null;
      }
      synchronized (stmt) {
        connection = stmt.getConnection();
        stmt.setString(1, cutAbonentAddress(abonent));
        rs = stmt.executeQuery();
        if (rs.next()) {
          BigDecimal r = rs.getBigDecimal(1);
          currency = smeEngine.getCurrency(rs.getString(2));
          if (r != null) {
            balance = new Double(r.doubleValue());
          }
        }
      }
    } catch (SQLException temporalError) {
      if (rs != null)
        try {
          rs.close();
          rs = null;
        } catch (Exception e1) {
          logger.warn("Could not close ifx ResultSet: " + e1);
        }
      if (stmt != null)
        try {
          stmt.close();
          stmt = null;
        } catch (Exception e1) {
          logger.warn("Could not close ifx CallableStatement: " + e1);
        }
      if (connection != null)
        try {
          connection.close();
          connection = null;
        } catch (Exception e1) {
          logger.warn("Could not close ifx Connection: "+e1);
        }

      try {
        if (!smeEngine.isInManConnectionError(temporalError)) {
          throw temporalError;
        }
        stmt = smeEngine.getInManStatement();
        if (stmt == null) {
          logger.error("Couldn't get InMan statement");
          return null;
        }
        connection = stmt.getConnection();
        synchronized (stmt) {
          stmt.setString(1, cutAbonentAddress(abonent));
          rs = stmt.executeQuery();
          if (rs.next()) {
            BigDecimal r = rs.getBigDecimal(1);
            if (r != null) {
              balance = new Double(r.doubleValue());
              currency = smeEngine.getCurrency(stmt.getString(2));
            }
          }
        }
      } catch (SQLException permanentError) {
        logger.error("Could not get balance from database: " + permanentError, permanentError);
        if (rs != null)
          try {
            rs.close();
          } catch (Exception e1) {
            logger.warn("Could not close ResultSet: ", e1);
          }
        if (stmt != null)
          try {
            stmt.close();
          } catch (Exception e1) {
            logger.warn("Could not close CallableStatement.", e1);
          }
        if (connection != null)
          try {
            connection.close();
          } catch (Exception e1) {
            logger.warn("Could not close Connection.", e1);
          }
      }
    } finally {
      if (rs != null)
        try {
          rs.close();
        } catch (Exception e) {
          logger.warn("Could not close ResultSet.", e);
        }
    }
    if (balance != null) {
      result = new GetBalanceResult(smeEngine.getNumberFormat().format(balance), currency);
    }
    return result;
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

}

