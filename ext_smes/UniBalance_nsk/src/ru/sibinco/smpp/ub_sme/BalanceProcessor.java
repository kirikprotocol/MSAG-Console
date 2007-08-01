package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;
import ru.sibinco.smpp.ub_sme.inman.AbonentContractResult;
import ru.sibinco.smpp.ub_sme.inman.InManPDUException;
import ru.sibinco.smpp.ub_sme.inbalance.InBalanceResult;
import ru.sibinco.smpp.ub_sme.inbalance.InBalancePDUException;
import ru.sibinco.util.threads.ThreadsPool;

import java.sql.*;
import java.math.BigDecimal;
import java.text.MessageFormat;

public class BalanceProcessor implements Runnable {
  private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(BalanceProcessor.class);

  private SmeEngine smeEngine = null;
  private RequestState state = null;

  private final static byte STORAGE_REGIONAL = 0;
  private final static byte STORAGE_SS = -1;
  private final static byte STORAGE_IN = 1;
  private final static byte STORAGE_IN_COMVERS = 2;
  private final static byte STORAGE_UNKNOWN = -2;

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
          if (state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX) || !smeEngine.isBillingSystemEnabled(state.getAbonentContractType(), SmeEngine.BILLING_SYSTEM_IN_MAN_INFORMIX))
          {
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
          if (state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE) || !smeEngine.isBillingSystemEnabled(state.getAbonentContractType(), SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE)){
            state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
            currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
          } else {
            if (state.getRegionStorageName() == null) {
              if (logger.isDebugEnabled())
                logger.debug("Get balance storage for " + abonent);
              state.setBillingSystemRequestTime(SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE, System.currentTimeMillis());
              int storage = getCbossStorage(abonent);
              state.setBillingSystemResponseTime(SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE, System.currentTimeMillis());
              switch (storage) {
                case STORAGE_REGIONAL:
                  if (logger.isDebugEnabled()) {
                    logger.debug("Balance storage for " + abonent + "is regional DB");
                    logger.debug("Get balance hostname for " + abonent);
                  }
                  String hostname = getCbossHostname(abonent);
                  state.setBillingSystemResponseTime(SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE, System.currentTimeMillis());
                  RegionInfo regionInfo;
                  if (hostname == null) {
                    logger.warn("Coldn't get abonent " + abonent + " balance hostname, using default");
                    regionInfo = smeEngine.getRegionInfo("default");
                  } else {
                    if (logger.isDebugEnabled())
                      logger.debug("Abonent " + abonent + " balance hostname: " + hostname);
                    regionInfo = smeEngine.getRegionInfo(hostname);
                    if (regionInfo == null) {
                      logger.warn("Unknown abonent " + abonent + " balance hostname: " + hostname + ", using default");
                      regionInfo = smeEngine.getRegionInfo("default");
                    }
                    state.setRegionStorageName(regionInfo.getName());
                    ThreadsPool pool=smeEngine.getRegionThreadPool(regionInfo.getName());
                    if(pool==null){
                      logger.error("Can't get thread pool \""+regionInfo.getName()+"\"");
                      state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
                      currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
                    } else {
                      pool.execute(this);
                    }
                  }
                  break;
                case STORAGE_IN:
                  if (logger.isDebugEnabled())
                    logger.debug("Balance storage for " + abonent + "is IN");
                  state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
                  currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
                  break;
                default:
                  logger.warn("Abonent " + abonent + " CBOSS balance corrupted, unknown balance storage: " + storage);
                  state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
                  currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
              }
            } else {
              if (logger.isDebugEnabled())
                logger.debug("CBOSS request for abonent " + abonent);
              state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE);
              balance = getCbossBalance(state.getAbonentRequest().getSourceAddress());
              state.setBillingSystemResponseTime(SmeEngine.BILLING_SYSTEM_CBOSS_ORACLE, System.currentTimeMillis());
              if (balance != null) {
                logger.debug("This is CBOSS abonent");
                state.setCurrentBillingSystemIndex(smeEngine.getBillingSystemCount(state.getAbonentContractType()));
              } else {
                logger.debug("This is not CBOSS abonent");
                state.setCurrentBillingSystemIndex(state.getCurrentBillingSystemIndex() + 1);
                currentBillingSystemId = smeEngine.getBillingSystemByOrder(state.getAbonentContractType(), state.getCurrentBillingSystemIndex());
              }
            }
          }
          break;
        case SmeEngine.BILLING_SYSTEM_IN_BALANCE:
          if (state.isBillingSystemQueried(SmeEngine.BILLING_SYSTEM_IN_BALANCE)) {
            InBalanceResult result = state.getInBalanceResult();
            if (result != null) {
              try {
                if (result.getStatus() != InBalanceResult.STATUS_OK) {
                  balance = result.getUssData();
                  break;
                } else {
                  logger.warn("InBalance status for abonent " + abonent + " is " + result.getStatus());
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

  private int getCbossStorage(String abonent) {
    int result = STORAGE_UNKNOWN;
    CallableStatement stmt = null;
    try {
      stmt = smeEngine.getCbossStatement("GetStorage");
      if (stmt == null) {
        logger.error("Couldn't get CBOSS statement");
        return result;
      }
      synchronized (stmt) {
        stmt.registerOutParameter(1, java.sql.Types.VARCHAR);
        stmt.setString(2, cutAbonentAddress(abonent));
        stmt.registerOutParameter(3, java.sql.Types.VARCHAR);
        stmt.registerOutParameter(4, java.sql.Types.DATE);
        stmt.registerOutParameter(5, java.sql.Types.VARCHAR);
        stmt.execute();
        result = stmt.getInt(1);
      }
    } catch (Exception temporalError) {
      try {
        smeEngine.closeCbossStatement(stmt, "GetStorage");
        if (!smeEngine.isCbossConnectionError(temporalError)) {
          if (temporalError instanceof SQLException) {
            throw (SQLException) temporalError;
          } else {
            logger.error("Unexpected exception: " + temporalError, temporalError);
            throw new SQLException(temporalError.getMessage());
          }
        }
        stmt = smeEngine.getCbossStatement("GetStorage");
        if (stmt == null) {
          logger.error("Couldn't get CBOSS statement");
          return result;
        }
        synchronized (stmt) {
          stmt.setString(1, cutAbonentAddress(abonent));
          stmt.registerOutParameter(2, java.sql.Types.VARCHAR);
          stmt.registerOutParameter(3, java.sql.Types.DATE);
          stmt.registerOutParameter(4, java.sql.Types.VARCHAR);
          stmt.execute();
          result = stmt.getInt(1);
        }
      } catch (SQLException permanentError) {
        logger.error("Could not get storage from CBOSS database: " + permanentError, permanentError);
        smeEngine.closeCbossStatement(stmt, "GetStorage");
        return result;
      }
    }

    return result;
  }

  private String getCbossHostname(String abonent) {
    String result = null;
    CallableStatement stmt = null;
    try {
      stmt = smeEngine.getCbossStatement("GetHostname");
      if (stmt == null) {
        logger.error("Couldn't get CBOSS statement");
        return result;
      }
      synchronized (stmt) {
        stmt.registerOutParameter(1, java.sql.Types.VARCHAR);
        stmt.setString(2, cutAbonentAddress(abonent));
        stmt.execute();
        result = stmt.getString(1);
      }
    } catch (Exception temporalError) {
      try {
        smeEngine.closeCbossStatement(stmt, "GetHostname");
        if (!smeEngine.isCbossConnectionError(temporalError)) {
          if (temporalError instanceof SQLException) {
            throw (SQLException) temporalError;
          } else {
            logger.error("Unexpected exception: " + temporalError, temporalError);
            throw new SQLException(temporalError.getMessage());
          }
        }
        stmt = smeEngine.getCbossStatement("GetHostname");
        if (stmt == null) {
          logger.error("Couldn't get CBOSS statement");
          return result;
        }
        synchronized (stmt) {
          stmt.setString(1, cutAbonentAddress(abonent));
          stmt.registerOutParameter(2, java.sql.Types.VARCHAR);
          stmt.registerOutParameter(3, java.sql.Types.DATE);
          stmt.registerOutParameter(4, java.sql.Types.VARCHAR);
          stmt.execute();
          result = stmt.getString(1);
        }
      } catch (SQLException permanentError) {
        logger.error("Could not get hostname from CBOSS database: " + permanentError, permanentError);
        smeEngine.closeCbossStatement(stmt, "GetHostname");
        return result;
      }
    }

    return result;
  }

  private String getCbossBalance(String abonent) {

    double balance = Double.NaN;
    //long balanceDate = 0;
    String currency = null;
    String accumulator = null;
    CallableStatement stmt = null;
    try {
      stmt = smeEngine.getCbossStatement(state.getRegionStorageName());
      if (stmt == null) {
        logger.error("Couldn't get CBOSS statement");
        return null;
      }
      synchronized (stmt) {
        stmt.registerOutParameter(1, java.sql.Types.VARCHAR);
        stmt.setString(2, cutAbonentAddress(abonent));
        stmt.registerOutParameter(3, java.sql.Types.DATE);
        stmt.registerOutParameter(4, java.sql.Types.VARCHAR);
        try {
          stmt.registerOutParameter(5, Types.VARCHAR);
          stmt.registerOutParameter(6, Types.VARCHAR);
        } catch (SQLException e) {
          ;
        }
        stmt.execute();
        if (stmt.getString(4).equalsIgnoreCase("ACCURATE") || stmt.getString(4).equalsIgnoreCase("CACHED")) {
          balance = Double.parseDouble(stmt.getString(1));
          RegionInfo regionInfo = smeEngine.getRegionInfo(state.getRegionStorageName());
          if (regionInfo == null) {
            logger.warn("Unknown abonent " + abonent + " balance storage: " + state.getRegionStorageName() + ", using default");
            regionInfo = smeEngine.getRegionInfo("default");
          }
          if(regionInfo.isPS()){
            balance = balance * (-1D);
          }
          String currencyCode = null;
          try {
            currencyCode = stmt.getString(5);
            accumulator = stmt.getString(6);
          } catch (SQLException e) {
            ;
          }
          currency = smeEngine.getCurrency(currencyCode);
        } else {
          logger.warn("Abonent " + abonent + " CBOSS balance corrupted: " + stmt.getString(4));
          return null;
        }
      }
    } catch (Exception temporalError) {
      try {
        smeEngine.closeCbossStatement(stmt, state.getRegionStorageName());
        if (!smeEngine.isCbossConnectionError(temporalError)) {
          if (temporalError instanceof SQLException) {
            throw (SQLException) temporalError;
          } else {
            logger.error("Unexpected exception: " + temporalError, temporalError);
            throw new SQLException(temporalError.getMessage());
          }
        }
        stmt = smeEngine.getCbossStatement(state.getRegionStorageName());
        if (stmt == null) {
          logger.error("Couldn't get CBOSS statement");
          return null;
        }
        synchronized (stmt) {
          stmt.registerOutParameter(1, java.sql.Types.VARCHAR);
          stmt.setString(2, cutAbonentAddress(abonent));
          stmt.registerOutParameter(3, java.sql.Types.DATE);
          stmt.registerOutParameter(4, java.sql.Types.VARCHAR);
          try {
            stmt.registerOutParameter(5, Types.VARCHAR);
            stmt.registerOutParameter(6, Types.VARCHAR);
          } catch (SQLException e) {
            ;
          }
          stmt.execute();
          if (stmt.getString(4).equalsIgnoreCase("ACCURATE") || stmt.getString(4).equalsIgnoreCase("CACHED")) {
            balance = Double.parseDouble(stmt.getString(1));
            RegionInfo regionInfo = smeEngine.getRegionInfo(state.getRegionStorageName());
            if (regionInfo == null) {
              logger.warn("Unknown abonent " + abonent + " balance storage: " + state.getRegionStorageName() + ", using default");
              regionInfo = smeEngine.getRegionInfo("default");
            }
            if(regionInfo.isPS()){
              balance = balance * (-1D);
            }
            String currencyCode = null;
            try {
              currencyCode = stmt.getString(5);
              accumulator = stmt.getString(6);
            } catch (SQLException e) {
              ;
            }
            currency = smeEngine.getCurrency(currencyCode);
          } else {
            logger.warn("Abonent " + abonent + " CBOSS balance corrupted: " + stmt.getString(4));
            return null;
          }
        }
      } catch (SQLException permanentError) {
        logger.error("Could not get balance from CBOSS database: " + permanentError, permanentError);
        smeEngine.closeCbossStatement(stmt, state.getRegionStorageName());
        return null;
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
    if (balance == null) {
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

