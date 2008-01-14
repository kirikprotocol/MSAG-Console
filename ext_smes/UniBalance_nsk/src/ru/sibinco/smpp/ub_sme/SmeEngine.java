package ru.sibinco.smpp.ub_sme;

import com.logica.smpp.Data;
import com.lorissoft.advertising.syncclient.IAdvertisingClient;
import com.lorissoft.advertising.syncclient.AdvertisingClientImpl;
import ru.aurorisoft.smpp.*;
import ru.sibinco.util.threads.ThreadsPool;
import ru.sibinco.smpp.ub_sme.util.DBConnectionManager;
import ru.sibinco.smpp.ub_sme.util.Utils;
import ru.sibinco.smpp.ub_sme.util.Matcher;
import ru.sibinco.smpp.ub_sme.inman.*;
import ru.sibinco.smpp.ub_sme.inbalance.*;

import java.util.*;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.CallableStatement;
import java.sql.Statement;
import java.text.DecimalFormatSymbols;
import java.text.NumberFormat;
import java.text.DecimalFormat;
import java.text.MessageFormat;
import java.io.UnsupportedEncodingException;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;

public class SmeEngine implements MessageListener, ResponseListener, InManPDUHandler, InBalancePDUHandler {

  private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(SmeEngine.class);

  public final static byte BILLING_SYSTEM_IN_MAN_CONTRACT_TYPE = 0;
  public final static byte BILLING_SYSTEM_IN_MAN_INFORMIX = 1;
  public final static byte BILLING_SYSTEM_CBOSS_ORACLE = 2;
  public final static byte BILLING_SYSTEM_FORIS_MG = 3;
  public final static byte BILLING_SYSTEM_IN_BALANCE = 4;
  public final static byte BILLING_SYSTEM_MEDIO_SCP = 5;

  public final static int BILLING_SYSTEMS_COUNT = 6;

  public final static byte CONTRACT_TYPE_UNKNOWN = 0x00;
  public final static byte CONTRACT_TYPE_POSTPAID = 0x01;
  public final static byte CONTRACT_TYPE_PREPAID = 0x02;

  public final static int CONTRACT_TYPES_COUNT = 3;

  //public final static String[] BILLING_SYSTEMS = {"IN_MAN_CONTRACT_TYPE", "IN_MAN_INFORMIX", "CBOSS_ORACLE", "FORIS_MG", "IN_BALANCE", "MEDIO_SCP"};
  public final static String[] BILLING_SYSTEMS = {"CT", "IFX", "CBOSS", "MG", "INB", "MEDIO"};

  private String mgAddress = null;
  private String cbossPoolName = null;
  private String inManPoolName = null;
  private String medioScpPoolName = null;

  private String cbossQuery = null;
  private String inManQuery = null;
  private String medioScpQuery = null;

  private String cbossConnectionErrorPattern = "Connection,NullPointerException";
  private String inManConnectionErrorPattern = "System or internal error,already closed";
  private String medioScpConnectionErrorPattern = "Connection,NullPointerException";
  private String inManInIsdnPattern = null;
  private String medioScpInIsdnPattern = null;
  private String inBalanceInIsdnPattern = null;

  private String inManHost = "localhost";
  private String inManPort = "5678";
  private boolean inManUseCache = true;

  private String inBalanceHost = "localhost";
  private String inBalancePort = "9687";
  private String inBalanceUssData = "100";
  private byte inBalanceIN_SSN = (byte) 147;
  private String[] inBalanceIN_ISDNS;
  private Matcher[] inBalanceResponsePatterns;

  private byte[][] billingSystemOrder = new byte[CONTRACT_TYPES_COUNT][];
  private boolean[][] billingSystemEnabled = new boolean[CONTRACT_TYPES_COUNT][BILLING_SYSTEMS.length];

  private String bannerEngineServiceName = "UniBalance";
  private int bannerEngineClientID = 1;
  private int bannerEngineTransportType = 1;
  private int bannerEngineCharSet = 1;

  private boolean bannerEngineClientEnabled = false;
  private IAdvertisingClient bannerEngineClient;
  private String bannerEngineClientHost = "localhost";
  private String bannerEngineClientPort = "8555";
  private String bannerEngineClientTimeout = "1000";
  private String bannerEngineClientReconnectTimeout = "1000";

  private boolean smsResponseMode = false;

  private DecimalFormatSymbols decimalFormatSymbols;
  private String numberFormatPattern = "#.##;-#.##";
  private String numberFormatNegativePattern = numberFormatPattern;

  private File responsePatternConfigFile;

  private String balanceResponsePattern = "{0} {1}";
  private String balanceNegativeResponsePattern;
  private String balanceWithAccumulatorResponsePattern = "{0} {1}";
  private String balanceWithAccumulatorNegativeResponsePattern;
  private String waitForSmsResponsePattern = "{0}";
  private String bannerAddPattern = "{0}\n{1}";
  private String errorPattern = "Error occurred";
  private String accessDeniedPattern = "Access denied";
  private boolean flashSmsEnabled = false;

  private Map currency;

  private int ussdMaxLength = 67;
  private long ussdSessionTimeout = 5000L;

  private int maxProcessingRequests = 10000;

  boolean contractTypeDetectionEnabled = false;
  private InManClient inManClient = null;
  private InBalanceClient inBalanceClient = null;

  private ThreadsPool threadsPool = null;
  private Multiplexor multiplexor = null;
  private OutgoingQueue outgoingQueue;
  private DBConnectionManager connectionManager = DBConnectionManager.getInstance();
  private List states = new ArrayList();
  private Map mgRequests = new HashMap();
  private Map inManRequests = new HashMap();
  private Map inBalanceRequests = new HashMap();

  private Map cbossStatements = new HashMap();
  private Map inManStatements = new HashMap();
  private Map medioScpStatements = new HashMap();

  private ProductivityControlObject requests;
  private ProductivityControlObject responses;
  private ProductivityControlObject waitForSmsResponses;

  private int bannerEngineTransactionId;
  private final Object bannerEngineTransactionIdSyncMonitor = new Object();

  public void init(Properties config) throws InitializationException {
    if (logger.isDebugEnabled()) logger.debug("UniBalance SME init started");

    contractTypeDetectionEnabled = Boolean.valueOf(config.getProperty("contract.type.detection.enabled", "false")).booleanValue();

    List billingSystemsOrderList = new ArrayList(5);
    int i = 1;
    String s = null;
    while ((s = config.getProperty("billing.system." + i)) != null) {
      if (s.length() > 0) {
        try {
          billingSystemsOrderList.add(new Byte(s));
        } catch (NumberFormatException e) {
          throw new InitializationException("Invalid value for config parameter \"billing.system." + i + ": " + s);
        }
      }
      i++;
    }

    billingSystemOrder[CONTRACT_TYPE_UNKNOWN] = new byte[(contractTypeDetectionEnabled ? 1 : 0) + billingSystemsOrderList.size()];
    if (contractTypeDetectionEnabled) {
      billingSystemOrder[CONTRACT_TYPE_UNKNOWN][0] = BILLING_SYSTEM_IN_MAN_CONTRACT_TYPE;
    }
    for (int j = 0; j < billingSystemsOrderList.size(); j++) {
      Byte bs = (Byte) billingSystemsOrderList.get(j);
      billingSystemOrder[CONTRACT_TYPE_UNKNOWN][j + (contractTypeDetectionEnabled ? 1 : 0)] = bs.byteValue();
      billingSystemEnabled[CONTRACT_TYPE_UNKNOWN][billingSystemOrder[CONTRACT_TYPE_UNKNOWN][j + (contractTypeDetectionEnabled ? 1 : 0)]] = true;
    }


    for (int c = 1; c < CONTRACT_TYPES_COUNT; c++) {
      i = 1;
      billingSystemsOrderList = new ArrayList(5);
      while ((s = config.getProperty("contract.type." + c + ".billing.system." + i)) != null) {
        if (s.length() > 0) {
          try {
            billingSystemsOrderList.add(new Byte(s));
          } catch (NumberFormatException e) {
            throw new InitializationException("Invalid value for config parameter \"contract.type." + c + ".billing.system." + i + ": " + s);
          }
        }
        i++;
      }
      billingSystemOrder[c] = new byte[(contractTypeDetectionEnabled ? 1 : 0) + billingSystemsOrderList.size()];
      if (contractTypeDetectionEnabled) {
        billingSystemOrder[c][0] = BILLING_SYSTEM_IN_MAN_CONTRACT_TYPE;
      }
      for (int j = 0; j < billingSystemsOrderList.size(); j++) {
        Byte bs = (Byte) billingSystemsOrderList.get(j);
        billingSystemOrder[c][j + (contractTypeDetectionEnabled ? 1 : 0)] = bs.byteValue();
        billingSystemEnabled[c][billingSystemOrder[c][j + (contractTypeDetectionEnabled ? 1 : 0)]] = true;
      }
    }

    if (contractTypeDetectionEnabled) {
      inManHost = config.getProperty("inman.host", inManHost);
      if (inManHost.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"inman.host\" is missed");
      }
      inManPort = config.getProperty("inman.port", inManPort);
      if (inManPort.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"inman.port\" is missed");
      }
      inManUseCache = Boolean.valueOf(config.getProperty("inman.use.cache", "true")).booleanValue();
    }

    if (isBillingSystemEnabled(CONTRACT_TYPE_UNKNOWN, BILLING_SYSTEM_IN_MAN_INFORMIX)) {
      inManPoolName = config.getProperty("inman.pool.name", "");
      if (inManPoolName.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"inman.pool.name\" is missed");
      }
      inManQuery = config.getProperty("inman.query", "");
      if (inManQuery.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"inman.query\" is missed");
      }
      inManConnectionErrorPattern = config.getProperty("inman.connection.error.pattern", inManConnectionErrorPattern);
      if (inManConnectionErrorPattern.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"inman.connection.error.pattern\" is missed");
      }
      try {
        inManConnectionErrorPattern = Utils.aggregateRegexp(inManConnectionErrorPattern);
      } catch (IllegalArgumentException e) {
        throw new InitializationException(e.getMessage(), e);
      }

      inManInIsdnPattern = config.getProperty("inman.in.isdn", inManInIsdnPattern);
      if (inManInIsdnPattern != null) {
        inManInIsdnPattern = Utils.aggregateRegexp(inManInIsdnPattern);
      }

    }

    if (isBillingSystemEnabled(CONTRACT_TYPE_UNKNOWN, BILLING_SYSTEM_CBOSS_ORACLE)) {
      cbossPoolName = config.getProperty("cboss.pool.name", "");
      if (cbossPoolName.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"cboss.pool.name\" is missed");
      }
      cbossQuery = config.getProperty("cboss.query", "");
      if (cbossQuery.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"cboss.query\" is missed");
      }
      cbossConnectionErrorPattern = config.getProperty("cboss.connection.error.pattern", cbossConnectionErrorPattern);
      if (cbossConnectionErrorPattern.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"cboss.connection.error.pattern\" is missed");
      }
      try {
        cbossConnectionErrorPattern = Utils.aggregateRegexp(cbossConnectionErrorPattern);
      } catch (IllegalArgumentException e) {
        throw new InitializationException(e.getMessage(), e);
      }
    }

    if (isBillingSystemEnabled(CONTRACT_TYPE_UNKNOWN, BILLING_SYSTEM_MEDIO_SCP)) {
      medioScpPoolName = config.getProperty("medioscp.pool.name", "");
      if (medioScpPoolName.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"medioscp.pool.name\" is missed");
      }
      medioScpQuery = config.getProperty("medioscp.query", "");
      if (medioScpQuery.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"medioscp.query\" is missed");
      }
      medioScpConnectionErrorPattern = config.getProperty("medioscp.connection.error.pattern", medioScpConnectionErrorPattern);
      if (medioScpConnectionErrorPattern.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"medioscp.connection.error.pattern\" is missed");
      }
      try {
        medioScpConnectionErrorPattern = Utils.aggregateRegexp(medioScpConnectionErrorPattern);
      } catch (IllegalArgumentException e) {
        throw new InitializationException(e.getMessage(), e);
      }

      medioScpInIsdnPattern = config.getProperty("medioscp.in.isdn", medioScpInIsdnPattern);
      if (medioScpInIsdnPattern != null) {
        medioScpInIsdnPattern = Utils.aggregateRegexp(medioScpInIsdnPattern);
      }

    }

    if (isBillingSystemEnabled(CONTRACT_TYPE_UNKNOWN, BILLING_SYSTEM_FORIS_MG)) {
      mgAddress = config.getProperty("mg.address", "");
      if (mgAddress.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"mg.address\" is missed");
      }
    }

    if (isBillingSystemEnabled(CONTRACT_TYPE_UNKNOWN, BILLING_SYSTEM_IN_BALANCE)) {
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

      inBalanceInIsdnPattern = config.getProperty("inbalance.in.isdn", inBalanceInIsdnPattern);
      if (inBalanceInIsdnPattern != null) {
        inBalanceInIsdnPattern = Utils.aggregateRegexp(inBalanceInIsdnPattern);
      }

      List IN_ISDNS = new ArrayList();
      i = 1;
      while ((s = config.getProperty("inbalance.in.isdn." + i)) != null) {
        IN_ISDNS.add(s);
        i++;
      }
      inBalanceIN_ISDNS = new String[IN_ISDNS.size()];
      for (int j = 0; j < IN_ISDNS.size(); j++) {
        inBalanceIN_ISDNS[j] = (String) IN_ISDNS.get(j);
      }
      List responsePatterns = new ArrayList();
      i = 1;
      while ((s = config.getProperty("inbalance.response.pattern." + i)) != null) {
        responsePatterns.add(s);
        i++;
      }
      inBalanceResponsePatterns = new Matcher[responsePatterns.size()];
      for (int j = 0; j < responsePatterns.size(); j++) {
        try {
          inBalanceResponsePatterns[j] = new Matcher((String) responsePatterns.get(j));
        } catch (Exception e) {
          throw new InitializationException("Illegal value of inbalance.response.pattern."+j+" property: "+e, e);
        }
      }

    }

    numberFormatPattern = config.getProperty("balance.number.format.pattern", numberFormatPattern);
    numberFormatNegativePattern = config.getProperty("balance.number.format.negative.pattern", numberFormatNegativePattern);
    String decimalSeparator = config.getProperty("balance.number.format.decimal.separator", ".");
    if (decimalSeparator.length() == 0 || decimalSeparator.length() > 1) {
      throw new InitializationException("Invalid value for config parameter \"balance.number.format.decimal.separator\": " + decimalSeparator);
    }
    decimalFormatSymbols = new DecimalFormatSymbols(Locale.US);
    decimalFormatSymbols.setDecimalSeparator(decimalSeparator.charAt(0));

    String responsePatternConfigFileName = config.getProperty("response.pattern.config.file", "response.pattern.properties");
    responsePatternConfigFile = new File(responsePatternConfigFileName);
    if (!responsePatternConfigFile.exists()) {
      responsePatternConfigFile = new File("config/" + responsePatternConfigFileName);
      if (!responsePatternConfigFile.exists()) {
        throw new InitializationException("Invalid value for config parameter \"response.pattern.config.file\": file not found");
      }
    }
    initResponsePatterns();

    long responsePatternConfigFileCheckInterval = 60000L;
    try {
      responsePatternConfigFileCheckInterval = Long.parseLong(config.getProperty("response.pattern.config.check.interval", Long.toString(60000L)));
    } catch (NumberFormatException e) {
      throw new InitializationException("Invalid value for config parameter \"response.pattern.config.check.interval\": " + config.getProperty("response.pattern.config.check.interval"));
    }
    (new ResponsePatternConfigController(responsePatternConfigFileCheckInterval)).startService();

    smsResponseMode = Boolean.valueOf(config.getProperty("sms.response.mode", "false")).booleanValue();
    flashSmsEnabled = Boolean.valueOf(config.getProperty("sms.response.flash", Boolean.toString(flashSmsEnabled))).booleanValue();

    try {
      ussdMaxLength = Integer.parseInt(config.getProperty("ussd.message.max.length", Integer.toString(ussdMaxLength)));
    } catch (NumberFormatException e) {
      throw new InitializationException("Invalid value for config parameter \"ussd.message.max.length\": " + config.getProperty("ussd.message.max.length"));
    }
    try {
      ussdSessionTimeout = Long.parseLong(config.getProperty("ussd.session.timeout", Long.toString(ussdSessionTimeout)));
    } catch (NumberFormatException e) {
      throw new InitializationException("Invalid value for config parameter \"ussd.session.timeout\": " + config.getProperty("ussd.session.timeout"));
    }
    long requestStatesControllerPollingInterval;
    try {
      requestStatesControllerPollingInterval = Long.parseLong(config.getProperty("request.states.controller.polling.interval", Long.toString(ussdSessionTimeout)));
    } catch (NumberFormatException e) {
      throw new InitializationException("Invalid value for config parameter \"request.states.controller.polling.interval\": " + config.getProperty("request.states.controller.polling.interval"));
    }
    try {
      maxProcessingRequests = Integer.parseInt(config.getProperty("max.processing.requests.count", Integer.toString(maxProcessingRequests)));
    } catch (NumberFormatException e) {
      throw new InitializationException("Invalid value for config parameter \"max.processing.requests.count\": " + config.getProperty("max.processing.requests.count"));
    }

    bannerEngineClientEnabled = Boolean.valueOf(config.getProperty("banner.engine.client.enabled", Boolean.toString(bannerEngineClientEnabled))).booleanValue();
    if (bannerEngineClientEnabled) {

      bannerEngineServiceName = config.getProperty("banner.engine.service.name", bannerEngineServiceName);
      if (bannerEngineServiceName.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"banner.engine.service.name\" is missed");
      }
      try {
        bannerEngineClientID = Integer.parseInt(config.getProperty("banner.engine.client.id", Integer.toString(bannerEngineClientID)));
      } catch (NumberFormatException e) {
        throw new InitializationException("Invalid value for config parameter \"banner.engine.client.id\": " + config.getProperty("banner.engine.client.id"));
      }
      try {
        bannerEngineTransportType = Integer.parseInt(config.getProperty("banner.engine.transport.type", Integer.toString(bannerEngineTransportType)));
      } catch (NumberFormatException e) {
        throw new InitializationException("Invalid value for config parameter \"banner.engine.transport.type\": " + config.getProperty("banner.engine.transport.type"));
      }
      try {
        bannerEngineCharSet = Integer.parseInt(config.getProperty("banner.engine.charset", Integer.toString(bannerEngineCharSet)));
      } catch (NumberFormatException e) {
        throw new InitializationException("Invalid value for config parameter \"banner.engine.charset\": " + config.getProperty("banner.engine.charset"));
      }

      bannerEngineClientHost = config.getProperty("banner.engine.client.host", bannerEngineClientHost);
      if (bannerEngineClientHost.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"banner.engine.client.host\" is missed");
      }
      bannerEngineClientPort = config.getProperty("banner.engine.client.port", bannerEngineClientPort);
      if (bannerEngineClientPort.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"banner.engine.client.port\" is missed");
      }
      bannerEngineClientTimeout = config.getProperty("banner.engine.client.timeout", bannerEngineClientTimeout);
      if (bannerEngineClientTimeout.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"banner.engine.client.timeout\" is missed");
      }
      bannerEngineClientReconnectTimeout = config.getProperty("banner.engine.client.reconnect.timeout", bannerEngineClientReconnectTimeout);
      if (bannerEngineClientReconnectTimeout.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"banner.engine.client.reconnect.timeout\" is missed");
      }
      Properties bannerEngineClientConfig = new Properties();
      bannerEngineClientConfig.put("ip", bannerEngineClientHost);
      bannerEngineClientConfig.put("port", bannerEngineClientPort);
      bannerEngineClientConfig.put("CLIENTTIMEOUT", bannerEngineClientTimeout);
      bannerEngineClientConfig.put("WATCHDOGSLEEP", bannerEngineClientReconnectTimeout);

      bannerEngineClient = new AdvertisingClientImpl();
      bannerEngineClient.init(bannerEngineClientConfig);
    }

    boolean productivityControllerEnabled = Boolean.valueOf(config.getProperty("productivity.controller.enabled", "true")).booleanValue();
    if (productivityControllerEnabled) {
      ProductivityController productivityController = new ProductivityController();
      productivityController.init(config);

      requests = new ProductivityControlObject("Requests");
      responses = new ProductivityControlObject("Responses");
      waitForSmsResponses = new ProductivityControlObject("Wait messages");

      productivityController.addControlObject(requests);
      productivityController.addControlObject(responses);
      productivityController.addControlObject(waitForSmsResponses);

      if (outgoingQueue != null && outgoingQueue.getMessageSender() != null) {
        productivityController.addControlObject(outgoingQueue.getMessageSender());
      }

      productivityController.startService();
    }

    if (contractTypeDetectionEnabled) {
      inManClient = new InManClient(inManHost, inManPort, this);
      inManClient.connect();
    }

    if (isBillingSystemEnabled(CONTRACT_TYPE_UNKNOWN, BILLING_SYSTEM_IN_BALANCE)) {
      inBalanceClient = new InBalanceClient(inBalanceHost, inBalancePort, this);
      inBalanceClient.connect();
    }

    new RequestStatesController(requestStatesControllerPollingInterval).startService();

    if (logger.isDebugEnabled()) logger.debug("UniBalance SME init fineshed");
  }

  private void initResponsePatterns() throws InitializationException {
    Properties patternConfig;
    try {
      patternConfig = new Properties();
      patternConfig.load(new FileInputStream(responsePatternConfigFile));
    } catch (IOException e) {
      logger.error("Exception occured during loading response pattern configuration from " + responsePatternConfigFile.getName(), e);
      throw new InitializationException("Exception occured during loading response pattern configuration.", e);
    }

    balanceResponsePattern = patternConfig.getProperty("balance.response.pattern", balanceResponsePattern);
    balanceNegativeResponsePattern = patternConfig.getProperty("balance.negative.response.pattern");
    balanceWithAccumulatorResponsePattern = patternConfig.getProperty("balance.with.accumulator.response.pattern", balanceResponsePattern);
    balanceWithAccumulatorNegativeResponsePattern = patternConfig.getProperty("balance.with.accumulator.negative.response.pattern");
    waitForSmsResponsePattern = patternConfig.getProperty("balance.wait.for.sms.response.pattern", waitForSmsResponsePattern);
    errorPattern = patternConfig.getProperty("balance.error.pattern", errorPattern);
    accessDeniedPattern = patternConfig.getProperty("access.denied.pattern", accessDeniedPattern);

    bannerAddPattern = patternConfig.getProperty("balance.banner.add.pattern", bannerAddPattern);

    currency = new HashMap();
    String defaultCurrency = patternConfig.getProperty("balance.currency.default", "usd");
    
    currency.put("default", defaultCurrency);

    String code;
    int n = 1;
    while ((code = patternConfig.getProperty("balance.currency." + n + ".code")) != null) {
      String value = patternConfig.getProperty("balance.currency." + n + ".name", "");
      currency.put(code, value);
      n++;
    }

  }

  public SmeEngine(Multiplexor multiplexor, OutgoingQueue messagesQueue, ThreadsPool threadsPool) {
    this.multiplexor = multiplexor;
    this.outgoingQueue = messagesQueue;
    this.threadsPool = threadsPool;
  }

  public String getName() {
    return "UniBalance SME";
  }

  public boolean handleMessage(Message msg) throws SMPPUserException {
    long requestTime = System.currentTimeMillis();
    try {
      if (msg.isReceipt()) {
        if (logger.isDebugEnabled())
          logger.debug("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: Receipt");
        sendDeliverSmResponse(msg, Data.ESME_ROK);
        return false;
      }
      if ((msg.getSourceAddress() == null || msg.getSourceAddress().trim().equals(""))) {
        if (logger.isDebugEnabled())
          logger.debug("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: Empty Source Address");
        sendDeliverSmResponse(msg, Data.ESME_ROK);
        return false;
      }
      /*
      if (msg.getEncoding() == Message.ENCODING_BINARY) {
        if (logger.isDebugEnabled())
          logger.debug("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: Unsupported encoding: BINARY");
        sendDeliverSmResponse(msg, Data.ESME_ROK);
        return true;
      }
      */
      if (logger.isDebugEnabled())
        logger.debug("MSG handled. ConnID #" + msg.getConnectionId() + "; SeqN #" + msg.getSequenceNumber() + "; USSD #" + msg.getUssdServiceOp() + "; destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; msg: " + msg.getMessageString());

      processIncomingMessage(msg, requestTime);
      return true;
    } catch (Exception e) { // logic exception (message format)
      logger.error("Cannot handle MSG", e);
      sendDeliverSmResponse(msg, Data.ESME_RSYSERR);
      return false;
    }
  }

  public void handleResponse(PDU pdu) throws SMPPException {
    String responseType = "SUBMIT_SM_RESP";
    switch (pdu.getType()) {
      case Data.ENQUIRE_LINK_RESP:
        responseType = "ENQUIRE_LINK_RESP";
        break;
      case Data.DATA_SM_RESP:
        responseType = "DATA_SM_RESP";
        break;
    }

    if (pdu.getType() == Data.SUBMIT_SM_RESP || pdu.getType() == Data.DATA_SM_RESP) { // TODO: check for ussd_service_op

      if (logger.isDebugEnabled()) {
        logger.debug(responseType + " handled. ConnID #" + pdu.getConnectionId() + "; SeqN #" + pdu.getSequenceNumber() + "; Status #" + pdu.getStatus());
      }

      if (pdu.getStatusClass() == PDU.STATUS_CLASS_TEMP_ERROR) {
        outgoingQueue.updateOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatus());
      } else {

        if (isBillingSystemEnabled(CONTRACT_TYPE_UNKNOWN, BILLING_SYSTEM_FORIS_MG)) {
          String abonent = (String) mgRequests.remove(new Long(((long) pdu.getConnectionId()) << 32 | pdu.getSequenceNumber()));
          if (abonent != null && pdu.getStatus() != PDU.STATUS_CLASS_NO_ERROR) {
            RequestState state = extractRequestState(abonent);
            synchronized (state) {
              state.setError(true);
            }
            closeRequestState(state);
          }
        }

        outgoingQueue.removeOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatusClass());
      }
    }

  }

  protected void addRequestState(RequestState state) {
    synchronized (states) {
      states.add(state);
    }
    if (logger.isDebugEnabled())
      logger.debug(state.getAbonentRequest().getSourceAddress() + " request state added.");
  }

  protected RequestState getRequestState(String abonent) {
    synchronized (states) {
      int index = states.indexOf(new RequestStateIndex(abonent));
      if (index == -1) {
        if (logger.isDebugEnabled())
          logger.debug(abonent + " request state not found.");
        return null;
      }
      return (RequestState) states.get(index);
    }
  }

  protected RequestState extractRequestState(String abonent) {
    synchronized (states) {
      int index = states.indexOf(new RequestStateIndex(abonent));
      if (index == -1) {
        if (logger.isDebugEnabled())
          logger.debug(abonent + " request state not found.");
        return null;
      }
      return (RequestState) states.remove(index);
    }
  }

  protected void closeRequestState(String abonent) {
    if (abonent == null) {
      return;
    }
    RequestState state = extractRequestState(abonent);
    if (state == null)
      return;
    closeRequestState(state);
  }

  protected void closeRequestState(RequestState state) {
    String abonent = state.getAbonentRequest().getSourceAddress();
    synchronized (state) {
      if (((state.isBalanceReady() && (state.isBannerReady() || !bannerEngineClientEnabled)) || state.isError()) && !state.isClosed())
      {
        if (state.isError() || state.getAbonentResponse() == null) {
          Message message = new Message();
          message.setSourceAddress(state.getAbonentRequest().getDestinationAddress());
          message.setDestinationAddress(state.getAbonentRequest().getSourceAddress());
          if (state.getAbonentRequest().hasUssdServiceOp() && !state.isUssdSessionClosed()) {
            message.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
          } else {
            if (flashSmsEnabled) {
              message.setDestAddrSubunit(1); // for Flash SMS
            }
          }
          message.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
          message.setMessageString(errorPattern);
          message.setType(Message.TYPE_SUBMIT);
          state.setAbonentResponse(message);

          if (logger.isDebugEnabled())
            logger.debug(abonent + " request state closed with error.");
        } else {
          if (logger.isDebugEnabled())
            logger.debug(abonent + " request state closed.");
        }

        sendResponse(state);
        state.setClosed(true);
        extractRequestState(abonent);
        if (logger.isInfoEnabled())
          logger.info(state.toString());
      }
    }
  }

  private void processRequestStates() {
    RequestState state;
    if (logger.isDebugEnabled()) {
      logger.debug("Processing requests count: " + states.size());
    }
    for (int i = 0; i < states.size(); i++) {
      try {
        state = (RequestState) states.get(i);
      } catch (IndexOutOfBoundsException e) {
        return;
      }
      /*
      }
      while (states.size() > 0) {
        state = (RequestState) states.get(0);
      */
      if (state.getAbonentRequestTime() + ussdSessionTimeout > System.currentTimeMillis()) {
        break;
      }
      boolean sendWaitMessage = false;
      synchronized (state) {
        /*
        if (state.getAbonentRequestTime() + requestLifeTime <= System.currentTimeMillis() && !state.isClosed()) {
          if (logger.isDebugEnabled()) logger.debug(state.getAbonentRequest().getSourceAddress() + " request state expired.");
          if (!state.isUssdSessionClosed()) {
            sendDeliverSmResponse(state.getAbonentRequest(), Data.ESME_RSYSERR);
          }
          state.setClosed(true);
          states.remove(0);
          continue;
        }
        */
        if (!state.isUssdSessionClosed()) {
          sendWaitMessage = true;
          state.setUssdSessionClosed(true);
        }
      }
      if (sendWaitMessage) {
        if (contractTypeDetectionEnabled && state.getAbonentContractType() == CONTRACT_TYPE_UNKNOWN) {
          synchronized (state) {
            if (logger.isInfoEnabled())
              logger.info("Abonent " + state.getAbonentRequest().getSourceAddress() + " request aborted by timeout & unknown contract type");
            state.setError(true);
          }
          closeRequestState(state);
        } else {
          sendWaitForSmsMessage(state);
        }
      }
    }
  }


  private void sendDeliverSmResponse(Message msg, int status) {
    try {
      msg.setStatus(status);
      multiplexor.sendResponse(msg);
      if (logger.isDebugEnabled())
        logger.debug("Delivery response sent, destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; status #" + msg.getStatus());
    } catch (SMPPException e) {
      logger.warn("Could not send delivery response.", e);
    }
  }

  private void sendWaitForSmsMessage(RequestState state) {
    if (smsResponseMode) {
      return;
    }
    Message message = new Message();
    message.setSourceAddress(state.getAbonentRequest().getDestinationAddress());
    message.setDestinationAddress(state.getAbonentRequest().getSourceAddress());
    message.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
    message.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
    message.setMessageString(waitForSmsResponsePattern);
    message.setType(Message.TYPE_SUBMIT);
    sendMessage(message, state.getAbonentRequest().getConnectionName());
    if (waitForSmsResponses != null) {
      waitForSmsResponses.count();
    }
  }

  private void sendResponse(RequestState state) {
    Message msg = state.getAbonentResponse();
    if (bannerEngineClientEnabled && !state.isError()) {
      String banner = state.getBanner();
      if (banner != null && banner.length() > 0) {
        MessageFormat bannerFormat = new MessageFormat(bannerAddPattern);
        String messageWithBanner = bannerFormat.format(new String[]{msg.getMessageString(), banner});
        msg.setMessageString(messageWithBanner);
      } else {
        if (logger.isDebugEnabled())
          logger.debug("Banner is empty");
      }
    }
    if (((msg.getMessageString().length() > ussdMaxLength) || smsResponseMode) && !state.isUssdSessionClosed()) {
      sendWaitForSmsMessage(state);
      state.setUssdSessionClosed(true);
    }
    if (state.isUssdSessionClosed()) {
      msg.setUssdServiceOp(-1);
      if (flashSmsEnabled) {
        msg.setDestAddrSubunit(1); // for Flash SMS
      }
    }
    state.setAbonentResponseTime(System.currentTimeMillis());
    sendMessage(msg, state.getAbonentRequest().getConnectionName());
    if (responses != null) {
      responses.count();
    }
  }

  private void sendMessage(Message msg, String connectionName) {
    try {
      if (outgoingQueue != null) {
        outgoingQueue.addOutgoingObject(new OutgoingObject(msg));
      } else {
        multiplexor.assingSequenceNumber(msg, connectionName);
        multiplexor.sendMessage(msg, false);
      }
      if (logger.isDebugEnabled())
        logger.debug("MSG sent. ConnID #" + msg.getConnectionId() + "; SeqN #" + msg.getSequenceNumber() + "; USSD #" + msg.getUssdServiceOp() + "; destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; msg: " + msg.getMessageString());
    } catch (SMPPException e) {
      logger.error("Could not send message", e);
    }
  }

  private void processIncomingMessage(Message message, long abonentRequestTime) {
    RequestState state;
    if (!message.getSourceAddress().equals(mgAddress)) { // abonent request
      if (logger.isDebugEnabled())
        logger.debug("Got request from " + message.getSourceAddress());
      state = getRequestState(message.getSourceAddress());

      if (state != null) {
        if (logger.isDebugEnabled())
          logger.debug("Request rejected because another request from this abonent is already processing");
        sendDeliverSmResponse(message, Data.ESME_RMSGQFUL);
        return;
      }

      if (states.size() >= maxProcessingRequests) {
        if (logger.isDebugEnabled())
          logger.debug("Request rejected because max processing requests count is reached");
        sendDeliverSmResponse(message, Data.ESME_RMSGQFUL);
        return;
      }

      if (requests != null) {
        requests.count();
      }

      state = new RequestState(message, abonentRequestTime);
      addRequestState(state);
      try {
        threadsPool.execute(new BalanceProcessor(this, state));
      } catch (RuntimeException e) {
        logger.error("Exception occured during creating balance processor: " + e, e);
        synchronized (state) {
          state.setError(true);
        }
        extractRequestState(message.getSourceAddress());
        sendDeliverSmResponse(message, Data.ESME_RTHROTTLED);
        return;
      }
      if (bannerEngineClientEnabled) {
        try {
          threadsPool.execute(new BannerRequestThread(state));
        } catch (RuntimeException e) {
          logger.error("Exception occured during creating banner processor: " + e, e);
        }
      }
      sendDeliverSmResponse(message, Data.ESME_ROK);
    } else {  // mg response
      if (logger.isDebugEnabled())
        logger.debug("Got FORIS response for " + message.getDestinationAddress());
      sendDeliverSmResponse(message, Data.ESME_ROK);
      state = getRequestState(message.getDestinationAddress());
      if (state == null)
        return;
      state.setBillingSystemQueried(SmeEngine.BILLING_SYSTEM_FORIS_MG);
      state.setMgResponse(message);
      String forisBalance = parseForisBalance(state.getMgResponse().getMessageString());
      if (forisBalance == null) {
        if (logger.isDebugEnabled())
          logger.debug("This is not FORIS abonent");
        threadsPool.execute(new BalanceProcessor(this, state));
      } else {
        if (logger.isDebugEnabled())
          logger.debug("This is FORIS abonent");
        double balance = Double.parseDouble(forisBalance);
        Message msg = new Message();
        msg.setSourceAddress(state.getAbonentRequest().getDestinationAddress());
        msg.setDestinationAddress(state.getAbonentRequest().getSourceAddress());
        if (state.getAbonentRequest().hasUssdServiceOp()) {
          msg.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
        }
        msg.setUserMessageReference(state.getAbonentRequest().getUserMessageReference());
        String currency = getCurrency("default");
        message.setMessageString(getMessageFormat(balance).format(new Object[]{new Double(balance), currency}));
        msg.setType(Message.TYPE_SUBMIT);
        state.setAbonentResponse(msg);
        synchronized (state) {
          state.setBalanceReady(true);
        }
        closeRequestState(state);
      }
    }
  }


  protected void sendMgRequest(RequestState state, Message msg) {
    try {
      multiplexor.assingSequenceNumber(msg, state.getAbonentRequest().getConnectionName());
      mgRequests.put(new Long(((long) msg.getConnectionId()) << 32 | msg.getSequenceNumber()), state.getAbonentRequest().getSourceAddress());
      multiplexor.sendMessage(msg, false);
      if (logger.isDebugEnabled())
        logger.debug("MSG sent. ConnID #" + msg.getConnectionId() + "; SeqN #" + msg.getSequenceNumber() + "; USSD #" + msg.getUssdServiceOp() + "; destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; msg: " + msg.getMessageString());
    } catch (SMPPException e) {
      logger.error("Could not send MG request", e);
      synchronized (state) {
        state.setError(true);
      }
      closeRequestState(state);
    }
  }

  protected void requestInBalance(RequestState state) {
    if (state.getInManContractResult() != null && state.getAbonentContractType() != CONTRACT_TYPE_UNKNOWN) {
      String IN_ISDN = null;
      try {
        IN_ISDN = state.getInManContractResult().getGsmSCFAddress();
      } catch (InManPDUException e) {
        logger.error(e.getMessage(), e);
        return;
      }
      requestInBalance(state, IN_ISDN);
    } else {
      logger.debug("Abonent " + state.getAbonentRequest().getSourceAddress() + " contract type is UNKNOWN");
    }
  }

  protected void requestInBalance(RequestState state, String IN_ISDN) {
    Integer key = null;
    try {
      int requestId = inBalanceClient.assignRequestId();
      if (logger.isDebugEnabled())
        logger.debug("Send InBalance request for abonent " + state.getAbonentRequest().getSourceAddress() + ", IN_ISDN=" + IN_ISDN + ", sn=" + requestId);
      key = new Integer(requestId);
      synchronized (inBalanceRequests) {
        inBalanceRequests.put(key, state);
      }
      inBalanceClient.sendBalanceRequest(state.getAbonentRequest().getSourceAddress(), inBalanceUssData, inBalanceIN_SSN, IN_ISDN, requestId);
    } catch (InBalanceClientException e) {
      if (key != null) {
        inBalanceRequests.remove(key);
      }
      synchronized (state) {
        state.setError(true);
      }
      closeRequestState(state);
      logger.error("InBalance Client error: " + e, e);
    }
  }

  protected void requestAbonentContractType(RequestState state) {
    Integer key = null;
    try {
      int dialogID = inManClient.assignDialogID();
      if (logger.isDebugEnabled())
        logger.debug("Send InMan request for abonent " + state.getAbonentRequest().getSourceAddress() + ", sn=" + dialogID);
      key = new Integer(dialogID);
      synchronized (inManRequests) {
        inManRequests.put(key, state);
      }
      inManClient.sendContractRequest(state.getAbonentRequest().getSourceAddress(), dialogID, inManUseCache);
    } catch (InManClientException e) {
      if (key != null) {
        inManRequests.remove(key);
      }
      synchronized (state) {
        state.setError(true);
      }
      closeRequestState(state);
      logger.error("InMan Client error: " + e, e);
    }
  }

  public void handleInManPDU(InManPDU pdu) {
    if (pdu.getTag() != AbonentContractResult.TAG) {
      logger.error("Unexpected InMan PDU: " + pdu);
      return;
    }
    AbonentContractResult result = (AbonentContractResult) pdu;
    Integer key = new Integer(result.getDialogID());
    RequestState state;
    synchronized (inManRequests) {
      state = (RequestState) inManRequests.remove(key);
    }
    if (state == null) {
      if (logger.isDebugEnabled())
        logger.debug("Got InMan response for unknown abonent sn=" + result.getDialogID());
      return;
    }
    if (logger.isDebugEnabled())
      logger.debug("Got InMan response for " + state.getAbonentRequest().getSourceAddress());
    state.setInManContractResult(result);
    threadsPool.execute(new BalanceProcessor(this, state));
  }

  public void handleInBalancePDU(InBalancePDU pdu) {
    if (pdu.getCmdId() != InBalanceResult.CMD_ID) {
      logger.error("Unexpected InBalance PDU: " + pdu);
      return;
    }
    InBalanceResult result = (InBalanceResult) pdu;
    Integer key = new Integer(result.getRequestId());
    RequestState state;
    synchronized (inBalanceRequests) {
      state = (RequestState) inBalanceRequests.remove(key);
    }
    if (state == null) {
      if (logger.isDebugEnabled())
        logger.debug("Got InBalance response for unknown abonent sn=" + result.getRequestId());
      return;
    }
    if (logger.isDebugEnabled())
      logger.debug("Got InBalance response for " + state.getAbonentRequest().getSourceAddress());
    state.setInBalanceResult(result);
    threadsPool.execute(new BalanceProcessor(this, state));
  }


  private String getBanner(RequestState state) {
    String abonent = state.getAbonentRequest().getSourceAddress();
    byte[] banner = null;

    String encoding = "UTF-16BE";
    if (abonent.startsWith("+")) {
      abonent = ".1.1." + abonent.substring(1);
    } else if (!abonent.startsWith(".")) {
      abonent = ".1.1." + abonent;
    }
    int transactionId;
    synchronized (bannerEngineTransactionIdSyncMonitor) {
      transactionId = bannerEngineTransactionId++;
    }
    state.setBannerRequested();
    banner = bannerEngineClient.getLikelyBanner(abonent.getBytes(), abonent.getBytes().length, bannerEngineServiceName.getBytes(), bannerEngineTransportType, 140, bannerEngineCharSet, bannerEngineClientID, transactionId);

    if (banner == null) {
      return null;
    }

    try {
      return new String(banner, encoding);
    } catch (UnsupportedEncodingException e) {
      logger.error("Unsupported encoding: " + encoding, e);
      return null;
    }
  }

  private String parseForisBalance(String message) {
    for (int i = 0; i < message.length(); i++) {
      char c = message.charAt(i);
      if (!Character.isDigit(c)) {
        continue;
      }
      StringBuffer result = new StringBuffer();
      while (Character.isDigit(c) || c == '.' || c == ',') {
        result.append(c);
        i++;
        if (i == message.length()) {
          break;
        }
        c = message.charAt(i);
      }
      return result.toString();
    }
    return null;
  }

  protected byte getBillingSystemByOrder(int contractType, int orderID) {
    if (contractType >= 0 && contractType < CONTRACT_TYPES_COUNT && orderID >= 0 && orderID < billingSystemOrder[contractType].length)
    {
      return billingSystemOrder[contractType][orderID];
    }
    return -1;
  }

  protected boolean isBillingSystemEnabled(int contractType, byte billingSystemID) {
    if (contractType >= 0 && contractType < CONTRACT_TYPES_COUNT && billingSystemID >= 0 && billingSystemID < billingSystemEnabled[contractType].length)
    {
      return billingSystemEnabled[contractType][billingSystemID];
    }
    return false;
  }

  protected int getBillingSystemCount(int contractType) {
    return billingSystemOrder[contractType].length;
  }

  protected int getInBalanceIN_ISDNSCount() {
    return inBalanceIN_ISDNS.length;
  }

  protected String getInBalanceIN_ISDN(int index) {
    if (index >= 0 && index < inBalanceIN_ISDNS.length) {
      return inBalanceIN_ISDNS[index];
    } else {
      return null;
    }
  }

  protected String parseInBalanceResponse(String response){
    for (int i = 0; i < inBalanceResponsePatterns.length; i++) {
      Matcher matcher = inBalanceResponsePatterns[i];
      String result = matcher.match(response);
      if(result!=null){
        return result;
      }
    }
    return null;
  }

  protected NumberFormat getNumberFormat(double balance) {
    if (balance >= 0) {
      return new DecimalFormat(numberFormatPattern, decimalFormatSymbols);
    } else {
      return new DecimalFormat(numberFormatNegativePattern, decimalFormatSymbols);
    }
  }

  protected MessageFormat getMessageFormat(double balance) {
    if (balance >= 0 || balanceNegativeResponsePattern == null) {
      return new MessageFormat(balanceResponsePattern);
    } else {
      return new MessageFormat(balanceNegativeResponsePattern);
    }
  }

  protected MessageFormat getMessageFormatWithAccumulator(double balance) {
    if (balance >= 0 || balanceWithAccumulatorNegativeResponsePattern == null) {
      return new MessageFormat(balanceWithAccumulatorResponsePattern);
    } else {
      return new MessageFormat(balanceWithAccumulatorNegativeResponsePattern);
    }
  }

  protected String getAccessDeniedMessage(){
    return accessDeniedPattern;
  }

  protected String getCurrency(String code) {
    String result = (String) currency.get(code);
    if (result != null) {
      return result;
    }
    return (String) currency.get("default");
  }

  protected String getCbossQuery() {
    return cbossQuery;
  }

  protected CallableStatement getCbossStatement() throws SQLException {
    CallableStatement result;
    synchronized (cbossStatements) {
      result = (CallableStatement) cbossStatements.get(Thread.currentThread().getName());
    }
    if (result == null) {
      logger.debug("Creating CBOSS statement for " + Thread.currentThread().getName());
      result = getCbossConnection().prepareCall(getCbossQuery());
      synchronized (cbossStatements) {
        cbossStatements.put(Thread.currentThread().getName(), result);
      }
    }
    return result;
  }

  protected void closeCbossStatement(Statement stmt) {
    synchronized (cbossStatements) {
      cbossStatements.remove(Thread.currentThread().getName());
    }
    if (stmt != null) {
      try {
        stmt.close();
      } catch (SQLException e1) {
        logger.warn("Could not close oracle CallableStatement: " + e1);
      }
      Connection connection = null;
      try {
        connection = stmt.getConnection();
        if (connection != null) {
          connection.close();
          connection = null;
        }
      } catch (SQLException e1) {
        logger.warn("Could not close oracle Connection: " + e1);
      }
    }
  }

  protected boolean isCbossConnectionError(Exception e) {
    if (e.toString().matches(cbossConnectionErrorPattern)) {
      return true;
    } else {
      return false;
    }
  }

  protected String getInManQuery() {
    return inManQuery;
  }

  protected CallableStatement getInManStatement() throws SQLException {
    CallableStatement result;
    synchronized (inManStatements) {
      result = (CallableStatement) inManStatements.get(Thread.currentThread().getName());
    }
    if (result == null) {
      logger.debug("Creating InMan statement for " + Thread.currentThread().getName());
      result = getInManConnection().prepareCall(getInManQuery());
      synchronized (inManStatements) {
        inManStatements.put(Thread.currentThread().getName(), result);
      }
    }
    return result;
  }

  protected void closeInManStatement(Statement stmt) {
    synchronized (inManStatements) {
      inManStatements.remove(Thread.currentThread().getName());
    }
    if (stmt != null) {
      try {
        stmt.close();
      } catch (SQLException e1) {
        logger.warn("Could not close oracle CallableStatement: " + e1);
      }
      Connection connection = null;
      try {
        connection = stmt.getConnection();
        if (connection != null) {
          connection.close();
          connection = null;
        }
      } catch (SQLException e1) {
        logger.warn("Could not close ifx Connection: " + e1);
      }
    }
  }

  protected boolean isInManConnectionError(Exception e) {
    if (e.toString().matches(inManConnectionErrorPattern)) {
      return true;
    } else {
      return false;
    }
  }

  protected String getMedioScpQuery() {
    return medioScpQuery;
  }

  protected CallableStatement getMedioScpStatement() throws SQLException {
    CallableStatement result;
    synchronized (medioScpStatements) {
      result = (CallableStatement) medioScpStatements.get(Thread.currentThread().getName());
    }
    if (result == null) {
      logger.debug("Creating MedioSCP statement for " + Thread.currentThread().getName());
      result = getMedioScpConnection().prepareCall(getMedioScpQuery());
      synchronized (medioScpStatements) {
        medioScpStatements.put(Thread.currentThread().getName(), result);
      }
    }
    return result;
  }

  protected void closeMedioScpStatement(Statement stmt) {
    synchronized (medioScpStatements) {
      medioScpStatements.remove(Thread.currentThread().getName());
    }
    if (stmt != null) {
      try {
        stmt.close();
      } catch (SQLException e1) {
        logger.warn("Could not close MedioSCP CallableStatement: " + e1);
      }
      Connection connection = null;
      try {
        connection = stmt.getConnection();
        if (connection != null) {
          connection.close();
          connection = null;
        }
      } catch (SQLException e1) {
        logger.warn("Could not close MedioSCP Connection: " + e1);
      }
    }
  }

  protected boolean isMedioScpConnectionError(Exception e) {
    if (e.toString().matches(medioScpConnectionErrorPattern)) {
      return true;
    } else {
      return false;
    }
  }

  protected String getMgAddress() {
    return mgAddress;
  }

  protected Connection getCbossConnection() throws SQLException {
    return connectionManager.getConnection(cbossPoolName);
  }

  protected Connection getInManConnection() throws SQLException {
    return connectionManager.getConnection(inManPoolName);
  }

  protected Connection getMedioScpConnection() throws SQLException {
    return connectionManager.getConnection(medioScpPoolName);
  }

  protected String getInManInIsdnPattern() {
    return inManInIsdnPattern;
  }

  protected String getMedioScpInIsdnPattern() {
    return medioScpInIsdnPattern;
  }

  protected String getInBalanceInIsdnPattern() {
    return inBalanceInIsdnPattern;
  }

  class RequestStatesController extends Thread {

    private boolean started = false;
    private long pollingInterval = 5000L;
    private Object monitor = new Object();
    private Object shutmonitor = new Object();

    public RequestStatesController(long pollingInterval) {
      setName("RequestStatesController");
      this.pollingInterval = pollingInterval;
      if (logger.isInfoEnabled())
        logger.info(getName() + " created.");
    }

    public void startService() {
      started = true;
      if (logger.isInfoEnabled()) logger.info(getName() + " started.");
      start();
    }

    public void stopService() {
      synchronized (shutmonitor) {
        synchronized (monitor) {
          started = false;
          monitor.notifyAll();
        }
        try {
          shutmonitor.wait();
        } catch (InterruptedException e) {
          logger.warn(getName() + " shutting down exception:", e);
        }
      }
    }

    public boolean isStarted() {
      return started;
    }

    public void run() {
      while (started) {
        processRequestStates();
        synchronized (monitor) {
          if (!started) break;
          try {
            monitor.wait(pollingInterval);
          } catch (InterruptedException e) {
            logger.warn(getName() + " was interrupted.", e);
          }
        }
      }
      synchronized (shutmonitor) {
        shutmonitor.notifyAll();
      }
    }
  }

  class BannerRequestThread implements Runnable {
    private RequestState state;

    public BannerRequestThread(RequestState state) {
      this.state = state;
    }

    public void run() {
      try {
        String banner = getBanner(state);
        synchronized (state) {
          state.setBanner(banner);
        }
        closeRequestState(state);
      } catch (Throwable t) {
        synchronized (state) {
          state.setError(true);
        }
        if (logger.isInfoEnabled())
          logger.info("Can not get banner for " + state.getAbonentRequest().getSourceAddress());
        logger.error("Unexpected exception occured during processing request.", t);
      }
    }
  }

  class ResponsePatternConfigController extends Thread {
    private boolean started = false;
    private Object monitor = new Object();
    private Object shutmonitor = new Object();

    private long pollingInterval = 60000L;
    private long fileLastModified;

    public ResponsePatternConfigController(long pollingInterval) {
      this.pollingInterval = pollingInterval;
      this.fileLastModified = responsePatternConfigFile.lastModified();
    }

    public void startService() {
      started = true;
      if (logger.isInfoEnabled()) logger.info(getName() + " started.");
      start();
    }

    public void stopService() {
      synchronized (shutmonitor) {
        synchronized (monitor) {
          started = false;
          monitor.notifyAll();
        }
        try {
          shutmonitor.wait();
        } catch (InterruptedException e) {
          logger.warn(getName() + " shutting down exception:", e);
        }
      }
    }

    public void run() {
      while (started) {
        if (logger.isDebugEnabled())
          logger.debug("Check response pattern config file modification date");
        if (responsePatternConfigFile.lastModified() != fileLastModified) {
          if (logger.isInfoEnabled())
            logger.info("Response pattern config file was modificated. Reloading response pattern config.");
          try {
            initResponsePatterns();
            fileLastModified = responsePatternConfigFile.lastModified();
          } catch (InitializationException e) {
            logger.error("Error while reloading response pattern config: " + e, e);
          }
        }

        synchronized (monitor) {
          if (!started) break;
          try {
            monitor.wait(pollingInterval);
          } catch (InterruptedException e) {
            logger.warn(getName() + " was interrupted.", e);
          }
        }
      }
      synchronized (shutmonitor) {
        shutmonitor.notifyAll();
      }

    }

  }

}
