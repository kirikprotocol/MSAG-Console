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
    if (state == null) {
      logger.error("Call BalanceProcessor.process() with null state!");
      return;
    }

    String abonent = state.getAbonentRequest().getSourceAddress();

    if (logger.isDebugEnabled())
      logger.debug("BalanceProcessor.process(" + abonent + ")");

    String balance = null;


    int currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());

    int loopDetector = 0;
    while (state.getCurrentBillingSystemIndex() < smeEngine.getBillingSystemCount(state.getAbonentContractType())) {
      if (loopDetector == 10) {
        logger.error("Breaking on loop_detector=" + loopDetector);
        break;
      }
      loopDetector++;

      if (logger.isDebugEnabled())
        logger.debug("currentBillingSystemId=" + currentBillingSystemId);

      switch (currentBillingSystemId) {
        case SmeEngine.BILLING_SYSTEM_IN_MAN_CONTRACT_TYPE:
          if (state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_MAN_CONTRACT_TYPE)) {
            AbonentContractResult inManContractResult = state.getInManContractResult();
            byte contractType = SmeEngine.CONTRACT_TYPE_UNKNOWN;
            if (inManContractResult != null) {
              try {
                contractType = inManContractResult.getContractType();
                if (logger.isDebugEnabled())
                  logger.debug("InMan response for " + state.getAbonentRequest().getSourceAddress() + ": " + inManContractResult);
              } catch (InManPDUException e) {
                logger.error("InMan result Exception: " + e, e);
              }
            }
            switch (contractType) {
              case SmeEngine.CONTRACT_TYPE_PREPAID:
                if (logger.isDebugEnabled())
                  logger.debug("This is PREPAID abonent contract");
                break;
              case SmeEngine.CONTRACT_TYPE_POSTPAID:
                if (logger.isDebugEnabled())
                  logger.debug("This is POSTPAID abonent contract");
                break;
              default:
                if (logger.isDebugEnabled())
                  logger.debug("Abonent contract type is UNKNOWN");
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
            if (smeEngine.getInManInIsdnPattern() != null) {
              String InIsdn = null;
              if (state.getInManContractResult() != null) {
                try {
                  InIsdn = state.getInManContractResult().getGsmSCFAddress();
                } catch (InManPDUException e) {
                  logger.error("InMan result Exception: " + e, e);
                }
              }
              if (InIsdn == null) {
                InIsdn = "unknown";
              }
              if (!InIsdn.matches(smeEngine.getInManInIsdnPattern())) {
                if (logger.isDebugEnabled())
                  logger.debug("InMan Informix request canceled by IN ISDN (" + InIsdn + ") for abonent " + abonent);
                state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
                currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
                break;
              }
            }
            if (logger.isDebugEnabled())
              logger.debug("InMan Informix request for abonent " + abonent);
            state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX);
            if (smeEngine.getInManThreadConroller().start()) {
              try {
                balance = getInManBalance(state.getAbonentRequest().getSourceAddress());
              } finally {
                smeEngine.getInManThreadConroller().end();
              }
            } else {
              logger.warn("Max InMan Informix thread limit reached. Abonent " + abonent + " request aborted.");
              state.setCurrentBillingSystemIndex(smeEngine.getBillingSystemCount(state.getAbonentContractType()));
              break;
            }
            state.setBillingSystemResponseTime(SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX, System.currentTimeMillis());
            if (balance != null) {
              if (logger.isInfoEnabled() && !logger.isDebugEnabled())
                logger.info(abonent + " type=IFX");
              else if (logger.isDebugEnabled())
                logger.debug("This is InMan Informix abonent");
              state.setCurrentBillingSystemIndex(smeEngine.getBillingSystemCount(state.getAbonentContractType()));
              break;
            } else {
              if (logger.isDebugEnabled())
                logger.debug("This is not InMan Informix abonent");
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
            if (smeEngine.getCbossThreadConroller().start()) {
              try {
                balance = getCbossBalance(state.getAbonentRequest().getSourceAddress());
              } finally {
                smeEngine.getCbossThreadConroller().end();
              }
            } else {
              logger.warn("Max CBOSS thread limit reached. Abonent " + abonent + " request aborted.");
              state.setCurrentBillingSystemIndex(smeEngine.getBillingSystemCount(state.getAbonentContractType()));
              break;
            }
            state.setBillingSystemResponseTime(SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE, System.currentTimeMillis());
            if (balance != null) {
              if (logger.isInfoEnabled() && !logger.isDebugEnabled())
                logger.info(abonent + " type=CBOSS");
              else if (logger.isDebugEnabled())
                logger.debug("This is CBOSS abonent");
              state.setCurrentBillingSystemIndex(smeEngine.getBillingSystemCount(state.getAbonentContractType()));
              break;
            } else {
              if (logger.isDebugEnabled())
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
            if (result != null) {
              try {
                if (result.getStatus() == InBalanceResult.STATUS_OK) {
                  String inBalance = smeEngine.parseInBalanceResponse(result.getUssData());
                  if (logger.isDebugEnabled())
                    logger.debug("InBalance response for " + state.getAbonentRequest().getSourceAddress() + ": " + result);

                  if (inBalance != null) {
                    double balanceValue = Double.parseDouble(inBalance);
                    if (logger.isInfoEnabled() && !logger.isDebugEnabled()) {
                      String IN_ISDN = null;
                      if (state.getInManContractResult() != null && state.getAbonentContractType() != SmeEngine.CONTRACT_TYPE_UNKNOWN) {
                        try {
                          IN_ISDN = state.getInManContractResult().getGsmSCFAddress();
                        } catch (InManPDUException e) {
                          logger.error(e.getMessage(), e);
                        }
                      } else {
                        int IN_ISDNIndex = state.getCurrentInBalanceIN_ISDNIndex();
                        IN_ISDN = smeEngine.getInBalanceIN_ISDN(IN_ISDNIndex);
                      }
                      logger.info(abonent + " type=IN;GT=" + IN_ISDN);
                    }
                    if (logger.isDebugEnabled())
                      logger.debug("Numeric balance value: " + balanceValue);
                    balance = smeEngine.getMessageFormat(balanceValue).format(new Object[]{smeEngine.getNumberFormat(balanceValue).format(balanceValue), smeEngine.getCurrency("default")});
                    state.setCurrentBillingSystemIndex(smeEngine.getBillingSystemCount(state.getAbonentContractType()));
                    break;
                  } else {
                    if (logger.isDebugEnabled())
                      logger.debug("InBalance responce not matches to patterns");
                  }
                } else {
                  logger.warn("InBalance status for abonent " + abonent + " is " + result.getStatus());
                }
              } catch (InBalancePDUException e) {
                logger.error(e.getMessage(), e);
              }
            }
            if (state.getCurrentInBalanceIN_ISDNIndex() >= smeEngine.getInBalanceIN_ISDNSCount() - 1) {
              // go next billing sys
              state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
              currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
              break;
            } else {
              state.setCurrentInBalanceIN_ISDNIndex(state.getCurrentInBalanceIN_ISDNIndex() + 1);
            }
          }
          // do request
          String IN_ISDN = null;
          if (state.getInManContractResult() != null && state.getAbonentContractType() != SmeEngine.CONTRACT_TYPE_UNKNOWN) {
            try {
              IN_ISDN = state.getInManContractResult().getGsmSCFAddress();
            } catch (InManPDUException e) {
              logger.error(e.getMessage(), e);
            }

            if (smeEngine.getInBalanceInIsdnPattern() != null && !IN_ISDN.matches(smeEngine.getInBalanceInIsdnPattern())) {
              if (logger.isDebugEnabled())
                logger.debug("InBalance  request canceled by IN ISDN (" + IN_ISDN + ") for abonent " + abonent);
              state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
              currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
              break;
            }

            state.setCurrentInBalanceIN_ISDNIndex(smeEngine.getInBalanceIN_ISDNSCount());
          } else {
            int IN_ISDNIndex = state.getCurrentInBalanceIN_ISDNIndex();
            if (logger.isDebugEnabled())
              logger.debug("Abonent " + abonent + " contract type unknown, try IN ISDN #" + IN_ISDNIndex);
            IN_ISDN = smeEngine.getInBalanceIN_ISDN(IN_ISDNIndex);
          }
          if (IN_ISDN == null) {
            logger.warn("Abonent " + abonent + " inbalance request: IN_ISDN is null");
            state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
            currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
            break;
          }
          if (!state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_BALANCE)) {
            state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_BALANCE);
          }
          smeEngine.requestInBalance(state, IN_ISDN);
          return;
        case SmeEngine.BILLING_SYSTEM_MEDIO_SCP:
          if (state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_MEDIO_SCP) || !smeEngine.isBillingSystemEnabled(state.getAbonentContractType(), SmeEngine.BILLING_SYSTEM_MEDIO_SCP)) {
            state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
            currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
          } else {
            if (smeEngine.getMedioScpInIsdnPattern() != null) {
              String InIsdn = null;
              if (state.getInManContractResult() != null) {
                try {
                  InIsdn = state.getInManContractResult().getGsmSCFAddress();
                } catch (InManPDUException e) {
                  logger.error("InMan result Exception: " + e, e);
                }
              }
              if (InIsdn == null) {
                InIsdn = "unknown";
              }
              if (!InIsdn.matches(smeEngine.getMedioScpInIsdnPattern())) {
                if (logger.isDebugEnabled())
                  logger.debug("MedioSCP  request canceled by IN ISDN (" + InIsdn + ") for abonent " + abonent);
                state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
                currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
                break;
              }
            }

            if (logger.isDebugEnabled())
              logger.debug("MedioSCP request for abonent " + abonent);
            state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_MEDIO_SCP);
            if (smeEngine.getMedioScpThreadConroller().start()) {
              try {
                balance = getMedioScpBalance(state.getAbonentRequest().getSourceAddress());
              } finally {
                smeEngine.getMedioScpThreadConroller().end();
              }
            } else {
              logger.warn("Max MedioSCP thread limit reached. Abonent " + abonent + " request aborted.");
              state.setCurrentBillingSystemIndex(smeEngine.getBillingSystemCount(state.getAbonentContractType()));
              break;
            }
            state.setBillingSystemResponseTime(SmeEngine.BILLING_SYSTEM_MEDIO_SCP, System.currentTimeMillis());
            if (balance != null) {
              if (logger.isInfoEnabled() && !logger.isDebugEnabled())
                logger.info(abonent + " type=MEDIO");
              else if (logger.isDebugEnabled())
                logger.debug("This is MedioSCP abonent");
              state.setCurrentBillingSystemIndex(smeEngine.getBillingSystemCount(state.getAbonentContractType()));
              break;
            } else {
              if (logger.isDebugEnabled())
                logger.debug("This is not MedioSCP abonent");
              state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
              currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
            }
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
    String currency = null;
    Integer accumulator = null;
    Connection con = null;
    CallableStatement stmt = null;
    boolean denied = false;
    try {
      con = smeEngine.getCbossConnection();
      stmt = con.prepareCall(smeEngine.getCbossQuery());
      if (stmt == null) {
        logger.error("Couldn't get CBOSS statement");
        return null;
      }
      stmt.registerOutParameter(1, java.sql.Types.VARCHAR);
      stmt.setString(2, cutAbonentAddress(abonent));
      stmt.registerOutParameter(3, java.sql.Types.DATE);
      stmt.registerOutParameter(4, java.sql.Types.VARCHAR);
      stmt.registerOutParameter(5, java.sql.Types.VARCHAR);
      stmt.registerOutParameter(6, java.sql.Types.NUMERIC);
      stmt.execute();
      String balanceResult = stmt.getString(4);
      if (balanceResult.equalsIgnoreCase("ACCURATE") || balanceResult.equalsIgnoreCase("CACHED")) {
        balance = Double.parseDouble(stmt.getString(1));
        currency = smeEngine.getCurrency(stmt.getString(5));
        accumulator = new Integer(stmt.getInt(6));
        if (stmt.wasNull()) {
          accumulator = null;
        }
      } else
      if (balanceResult.equalsIgnoreCase("BALANCE ACCESS DENIED") && smeEngine.getAccessDeniedMessage() != null) {
        logger.info("Abonent " + abonent + " CBOSS balance access denied: " + balanceResult);
        denied = true;
      } else {
        logger.warn("Abonent " + abonent + " CBOSS balance corrupted: " + balanceResult);
        return null;
      }
    } catch (SQLException permanentError) {
      logger.error("Could not get balance for abonent "+abonent+" from CBOSS database: " + permanentError, permanentError);
      return null;
    } finally {
      if (stmt != null) {
        try {
          stmt.close();
        } catch (SQLException e) {
          logger.warn("Could not close Statement: " + e);
        }
      }
      if (con != null) {
        try {
          con.close();
        } catch (SQLException e) {
          logger.warn("Could not close Connection: " + e);
        }
      }
    }
    if (denied) {
      return smeEngine.getAccessDeniedMessage();
    }

    if (currency == null) {
      currency = smeEngine.getCurrency("default");
    }
    MessageFormat messageFormat;
    if (accumulator != null) {
      messageFormat = smeEngine.getMessageFormatWithAccumulator(balance);
    } else {
      messageFormat = smeEngine.getMessageFormat(balance);
    }
    return messageFormat.format(new Object[]{smeEngine.getNumberFormat(balance).format(balance), currency, accumulator});
  }

  private String getInManBalance(String abonent) {
    Double balance = null;
    String currency = null;
    Connection con = null;
    CallableStatement stmt = null;
    ResultSet rs = null;
    try {
      con = smeEngine.getInManConnection();
      stmt = con.prepareCall(smeEngine.getInManQuery());
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
    } catch (SQLException permanentError) {
      logger.error("Could not get balance for abonent "+abonent+" from database: " + permanentError, permanentError);
      return null;
    } finally {
      if (rs != null)
        try {
          rs.close();
        } catch (Exception e) {
          logger.warn("Could not close ResultSet.", e);
        }
      if (stmt != null) {
        try {
          stmt.close();
        } catch (SQLException e) {
          logger.warn("Could not close Statement: " + e);
        }
      }
      if (con != null) {
        try {
          con.close();
        } catch (SQLException e) {
          logger.warn("Could not close Connection: " + e);
        }
      }

    }
    if (balance == null) {
      return null;
    }
    if (currency == null) {
      currency = smeEngine.getCurrency("default");
    }
    return smeEngine.getMessageFormat(balance.doubleValue()).format(new String[]{smeEngine.getNumberFormat(balance.doubleValue()).format(balance), currency});
  }

  private String getMedioScpBalance(String abonent) {
    double balance;
    String currency;
    Connection con = null;
    Integer accumulator = null;
    CallableStatement stmt = null;
    try {
      con = smeEngine.getMedioScpConnection();
      stmt = con.prepareCall(smeEngine.getMedioScpQuery());
      if (stmt == null) {
        logger.error("Couldn't get MedioSCP statement");
        return null;
      }

      //FUNCTION get_accurate_date_by_num (num VARCHAR2, balance_date OUT date, result_type OUT VARCHAR2, currency_code OUT VARCHAR2, acc_value OUT NUMBER) RETURN NUMBER;
      stmt.registerOutParameter(1, java.sql.Types.VARCHAR);
      stmt.setString(2, cutAbonentAddress(abonent));
      stmt.registerOutParameter(3, java.sql.Types.DATE);
      stmt.registerOutParameter(4, java.sql.Types.VARCHAR);
      stmt.registerOutParameter(5, java.sql.Types.VARCHAR);
      stmt.registerOutParameter(6, java.sql.Types.NUMERIC);
      stmt.execute();
      if ("ACCURATE".equalsIgnoreCase(stmt.getString(4)) || "CACHED".equalsIgnoreCase(stmt.getString(4))) {
        balance = Double.parseDouble(stmt.getString(1));
        currency = smeEngine.getCurrency(stmt.getString(5));
        accumulator = new Integer(stmt.getInt(6));
        if (stmt.wasNull()) {
          accumulator = null;
        }
      } else {
        logger.warn("Abonent " + abonent + " MedioSCP balance corrupted: " + stmt.getString(4));
        return null;
      }
    } catch (SQLException permanentError) {
      logger.error("Could not get balance for abonent "+abonent+" from MedioSCP database: " + permanentError, permanentError);
      return null;
    } finally {
      if (stmt != null) {
        try {
          stmt.close();
        } catch (SQLException e) {
          logger.warn("Could not close Statement: " + e);
        }
      }
      if (con != null) {
        try {
          con.close();
        } catch (SQLException e) {
          logger.warn("Could not close Connection: " + e);
        }
      }
    }

    if (currency == null) {
      currency = smeEngine.getCurrency("default");
    }
    MessageFormat messageFormat;
    if (accumulator != null) {
      messageFormat = smeEngine.getMessageFormatWithAccumulator(balance);
    } else {
      messageFormat = smeEngine.getMessageFormat(balance);
    }
    return messageFormat.format(new Object[]{smeEngine.getNumberFormat(balance).format(balance), currency, accumulator});

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
      smeEngine.closeRequestState(state);
    }
  }

}

