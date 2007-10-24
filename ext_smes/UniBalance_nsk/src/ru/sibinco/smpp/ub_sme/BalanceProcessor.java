package ru.sibinco.smpp.ub_sme;

import com.logica.smpp.Data;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smpp.ub_sme.inman.AbonentContractResult;
import ru.sibinco.smpp.ub_sme.inman.InManPDUException;
import ru.sibinco.smpp.ub_sme.inbalance.InBalanceResult;
import ru.sibinco.smpp.ub_sme.inbalance.InBalancePDUException;

import java.sql.*;
import java.math.BigDecimal;
import java.text.MessageFormat;

public class BalanceProcessor implements Runnable {
  private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(BalanceProcessor.class);

  private SmeEngine smeEngine = null;
  private RequestState state = null;

  public BalanceProcessor(SmeEngine smeEngine, RequestState state) {
    this.smeEngine = smeEngine;
    this.state = state;
  }

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

      switch (currentBillingSystemId) {
        case SmeEngine.BILLING_SYSTEM_IN_MAN_CONTRACT_TYPE:
          if (state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_MAN_CONTRACT_TYPE)) {
            AbonentContractResult inManContractResult = state.getInManContractResult();

            byte contractType = SmeEngine.CONTRACT_TYPE_UNKNOWN;
            if (inManContractResult != null) {
              try {
                contractType = inManContractResult.getContractType();
              } catch (InManPDUException e) {
                logger.error("InMan result Exception: " + e, e);
              }
            }
            switch (contractType) {
              case SmeEngine.CONTRACT_TYPE_PREPAID:
                logger.debug("This is PREPAID abonent contract");
                break;
              case SmeEngine.CONTRACT_TYPE_POSTPAID:
                logger.debug("This is POSTPAID abonent contract");
                break;
              default:
                logger.debug("Abonent contract type UNKNOWN");
            }
            state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
            currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
          } else {
            state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_MAN_CONTRACT_TYPE);
            smeEngine.requestAbonentContractType(state);
            return;
          }
          break;
        case SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX:
          if (state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX) || !smeEngine.isBillingSystemEnabled(state.getAbonentContractType(), SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX)) {
            state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
            currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
          } else {
            if (logger.isDebugEnabled())
              logger.debug("InMan request for abonent " + abonent);
            state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX);
            balance = getInManBalance(state.getAbonentRequest().getSourceAddress());
            state.setBillingSystemResponseTime(SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX, System.currentTimeMillis());
            if (balance != null) {
              logger.debug("This is InMan abonent");
              state.setCurrentBillingSystemIndex(smeEngine.getBillingSystemCount(state.getAbonentContractType()));
              break;
            } else {
              logger.debug("This is not InMan abonent");
              state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
              currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
            }
          }
          break;
        case SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE:
          if (state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE) || !smeEngine.isBillingSystemEnabled(state.getAbonentContractType(), SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE)) {
            state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
            currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
          } else {
            if (logger.isDebugEnabled())
              logger.debug("CBOSS request for abonent " + abonent);
            state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE);
            balance = getCbossBalance(state.getAbonentRequest().getSourceAddress());
            state.setBillingSystemResponseTime(SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE, System.currentTimeMillis());
            if (balance != null) {
              logger.debug("This is CBOSS abonent");
              state.setCurrentBillingSystemIndex(smeEngine.getBillingSystemCount(state.getAbonentContractType()));
              break;
            } else {
              logger.debug("This is not CBOSS abonent");
              state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
              currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
            }
          }
          break;
        case SmeEngine.BILLING_SYSTEM_FORIS_MG:
          if (state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_FORIS_MG)) {
            state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
            currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
          } else {
            if (logger.isDebugEnabled())
              logger.debug("FORIS request for abonent " + abonent);
            Message message = new Message();
            message.setSourceAddress(state.getAbonentRequest().getSourceAddress());
            message.setDestinationAddress(smeEngine.getMgAddress());
            message.setMessageString("");
            message.setEsmClass((byte) (Data.SM_FORWARD_MODE));
            message.setType(Message.TYPE_SUBMIT);
            state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_FORIS_MG);
            smeEngine.sendMgRequest(state, message);
            return;
          }
          break;
        case SmeEngine.BILLING_SYSTEM_IN_BALANCE:
          if (state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_BALANCE)) {
            InBalanceResult result = state.getInBalanceResult();
            if(result!=null){
              try {
                if(result.getStatus()!=InBalanceResult.STATUS_OK){
                  balance = result.getUssData();
                  break;
                } else {
                  logger.warn("InBalance status for abonent "+abonent+" is "+result.getStatus());
                }
              } catch (InBalancePDUException e) {
                logger.error(e.getMessage(), e);
              }
            }
            state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
            currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
          } else {
            state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_BALANCE);
            smeEngine.requestInBalance(state);
            return;
          }
          break;
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
      message.setMessageString(balance);
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

  private String getCbossBalance(String abonent) {
    double balance = Double.NaN;
    //long balanceDate = 0;
    String currency = null;
    String accumulator = null;
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
        stmt.registerOutParameter(6, java.sql.Types.VARCHAR);
        stmt.execute();
        if (stmt.getString(4).equalsIgnoreCase("ACCURATE") || stmt.getString(4).equalsIgnoreCase("CACHED")) {
          balance = Double.parseDouble(stmt.getString(1));
          currency = smeEngine.getCurrency(stmt.getString(5));
          accumulator = stmt.getString(6);
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
          stmt.registerOutParameter(6, java.sql.Types.VARCHAR);
          stmt.execute();
          if (stmt.getString(4).equalsIgnoreCase("ACCURATE") || stmt.getString(4).equalsIgnoreCase("CACHED")) {
            balance = Double.parseDouble(stmt.getString(1));
            currency = smeEngine.getCurrency(stmt.getString(5));
            accumulator = stmt.getString(6);
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
    MessageFormat messageFormat;
    if(accumulator!=null){
      messageFormat = smeEngine.getMessageFormatWithAccumulator(balance);
    } else {
      messageFormat = smeEngine.getMessageFormat(balance);
    }
    return messageFormat.format(new String[]{smeEngine.getNumberFormat(balance).format(balance), currency, accumulator});
  }

  private String getInManBalance(String abonent) {
    Double balance = null;
    String currency = null;
    CallableStatement stmt = null;
    ResultSet rs = null;
    try {
      stmt = smeEngine.getInManStatement();
      if (stmt == null) {
        logger.error("Couldn't get InMan statement");
        return null;
      }
      synchronized (stmt) {
        stmt.setString(1, cutAbonentAddress(abonent));
        rs = stmt.executeQuery();
        if (rs.next()) {
          BigDecimal r = rs.getBigDecimal(1);
          if (r != null) {
            balance = new Double(r.doubleValue());
            currency = smeEngine.getCurrency(rs.getString(2));
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
      smeEngine.closeInManStatement(stmt);
      try {
        if (!smeEngine.isInManConnectionError(temporalError)) {
          throw temporalError;
        }
        stmt = smeEngine.getInManStatement();
        if (stmt == null) {
          logger.error("Couldn't get InMan statement");
          return null;
        }
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
        smeEngine.closeInManStatement(stmt);
      }
    } finally {
      if (rs != null)
        try {
          rs.close();
        } catch (Exception e) {
          logger.warn("Could not close ResultSet.", e);
        }
    }
    if(balance==null){
      return null;
    }
    if (currency == null) {
      currency = smeEngine.getCurrency("default");
    }
    return smeEngine.getMessageFormat(balance.doubleValue()).format(new String[]{smeEngine.getNumberFormat(balance.doubleValue()).format(balance), currency});
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

