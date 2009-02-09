package ru.sibinco.smpp.ub_sme;

import com.logica.smpp.Data;
import com.lorissoft.advertising.client.AdvClientConst;
import com.lorissoft.advertising.client.AdvertisingClientException;
import com.lorissoft.advertising.client.IAdvertisingClient;
import com.lorissoft.advertising.client.RequestContext;
import com.lorissoft.advertising.protocol.BannerReq;
import com.lorissoft.advertising.protocol.BannerResp;
import com.lorissoft.advertising.protocol.RollBackReq;
import com.lorissoft.advertising.protocol.ProtocolConsts;
import com.lorissoft.advertising.syncclient.SyncClientImpl;
import com.lorissoft.advertising.util.Encode;
import ru.aurorisoft.smpp.*;
import ru.sibinco.smpp.ub_sme.snmp.Agent;
import ru.sibinco.smpp.ub_sme.util.DBConnectionManager;
import ru.sibinco.smpp.ub_sme.util.Utils;
import ru.sibinco.util.threads.ThreadsPool;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.sql.CallableStatement;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.text.MessageFormat;
import java.util.*;

public class SmeEngine implements MessageListener, ResponseListener {

  private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(SmeEngine.class);

  protected final static byte BILLING_SYSTEM_CBOSS = 0;

  public final static int BILLING_SYSTEMS_COUNT = 1;

  public final static String[] BILLING_SYSTEMS = {"CBOSS"};

  private String cbossPoolName = null;
  private String cbossQuery = null;

  private String cbossConnectionErrorPattern = "Connection,NullPointerException";

  private String bannerEngineServiceName = "UniBalance";
  private int bannerEngineTransportType = 1;
  private int bannerEngineCharSet = 1;
  private int bannerEngineBannerLength = 140;

  private boolean bannerEngineClientEnabled = false;
  private IAdvertisingClient bannerEngineClient;
  private String bannerEngineClientHost = "localhost";
  private String bannerEngineClientPort = "";
  private String bannerEngineClientTimeout = "1000";
  private String bannerEngineClientReconnectTimeout = "1000";
  private String maxRequestInMemory = "1000";

  private boolean smsResponseMode = false;

  private File responsePatternConfigFile;

  private String balanceResponsePattern = "{0} {1}";
  private String waitForSmsResponsePattern = "{0}";
  private String bannerAddPattern = "{0}\n{1}";
  private String errorPattern = "Error occurred";

  private int ussdMaxLength = 67;
  private long ussdSessionTimeout = 5000L;

  private int maxProcessingRequests = 10000;

  // SNMP counters
  private AvgCounter balanceCounter;
  private AvgCounter bannerCounter;

  private long balanceWarnIntervalStart = 250;
  private long balanceMinorIntervalStart = 500;
  private long balanceMajorIntervalStart = 1000;
  private long balanceCriticalIntervalStart = 1000;

  private long bannerWarnIntervalStart = 250;
  private long bannerMinorIntervalStart = 500;
  private long bannerMajorIntervalStart = 750;
  private long bannerCriticalIntervalStart = 1000;

  private long bannerDisableOnErrorTimeout = 60000;

  private byte currentBalanceSeverity = Agent.SNMP_SEVERITY_NORMAL;
  private byte currentBannerSeverity = Agent.SNMP_SEVERITY_NORMAL;

  private Agent snmpAgent;

  //

  private ThreadsPool threadsPool = null;
  private ThreadsPool bannerThreadsPool = null;
  private Multiplexor multiplexor = null;
  private OutgoingQueue outgoingQueue;

  private DBConnectionManager connectionManager;
  private final Map states = new HashMap();
  private final Map bannerResponses = new HashMap();
  private final List statesExpire = new LinkedList();

  private Map cbossStatements = new HashMap();

  private ProductivityControlObject requests;
  private ProductivityControlObject responses;
  private ProductivityControlObject waitForSmsResponses;

  private boolean testMode = false;

  public void init(Properties config) throws InitializationException {
    if (logger.isDebugEnabled()) logger.debug("UniBalance SME init started");

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
    cbossConnectionErrorPattern = Utils.aggregateRegexp(cbossConnectionErrorPattern);

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
    (new ResponcePatternConfigController(responsePatternConfigFileCheckInterval)).startService();

    smsResponseMode = Boolean.valueOf(config.getProperty("sms.response.mode", "false")).booleanValue();

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
    logger.info("banner.engine.client.enabled on init: " + bannerEngineClientEnabled);
    if (bannerEngineClientEnabled) {

      bannerEngineServiceName = config.getProperty("banner.engine.service.name", bannerEngineServiceName);
      if (bannerEngineServiceName.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"banner.engine.service.name\" is missed");
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
      try {
        bannerEngineBannerLength = Integer.parseInt(config.getProperty("banner.engine.banner.length", Integer.toString(bannerEngineBannerLength)));
      } catch (NumberFormatException e) {
        throw new InitializationException("Invalid value for config parameter \"banner.engine.banner.length\": " + config.getProperty("banner.engine.banner.length"));
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
      maxRequestInMemory = config.getProperty("banner.engine.max.request.in.memory", maxRequestInMemory);
      if (maxRequestInMemory.length() == 0) {
        throw new InitializationException("Mandatory config parameter \"banner.engine.max.request.in.memory\" is missed");
      }
      Properties bannerEngineClientConfig = new Properties();
      bannerEngineClientConfig.put("advclient.ip", bannerEngineClientHost);
      bannerEngineClientConfig.put("advclient.port", bannerEngineClientPort);
      bannerEngineClientConfig.put("advclient.clientTimeOut", bannerEngineClientTimeout);
      bannerEngineClientConfig.put("advclient.watchDogSleep", bannerEngineClientReconnectTimeout);
      bannerEngineClientConfig.put("advclient.maxRequestInMemory", maxRequestInMemory);

      bannerEngineClient = new SyncClientImpl();
      try {
        bannerEngineClient.init(bannerEngineClientConfig);
        bannerEngineClient.connect();
      } catch (AdvertisingClientException e) {
        throw new InitializationException("BE error on init: "+e, e);
      }
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

    boolean responseTimeControllerEnabled = Boolean.valueOf(config.getProperty("response.time.controller.enabled", "true")).booleanValue();
    if (responseTimeControllerEnabled) {

      long responseTimeControllerPoolingInterval = 60000L;
      try {
        responseTimeControllerPoolingInterval = Long.parseLong(config.getProperty("response.time.controller.pooling.interval", Long.toString(responseTimeControllerPoolingInterval)));
      } catch (NumberFormatException e) {
        throw new InitializationException("Invalid value for config parameter \"response.time.controller.pooling.interval\": " + config.getProperty("response.time.controller.pooling.interval"));
      }

      try {
        balanceWarnIntervalStart = Long.parseLong(config.getProperty("balance.warning.interval.start", Long.toString(balanceWarnIntervalStart)));
      } catch (NumberFormatException e) {
        logger.error("Illegal long value: " + config.getProperty("balance.warning.interval.start"), e);
      }
      try {
        balanceMinorIntervalStart = Long.parseLong(config.getProperty("balance.minor.interval.start", Long.toString(balanceMinorIntervalStart)));
      } catch (NumberFormatException e) {
        logger.error("Illegal long value: " + config.getProperty("balance.minor.interval.start"), e);
      }
      try {
        balanceMajorIntervalStart = Long.parseLong(config.getProperty("balance.major.interval.start", Long.toString(balanceMajorIntervalStart)));
      } catch (NumberFormatException e) {
        logger.error("Illegal long value: " + config.getProperty("balance.major.interval.start"), e);
      }
      try {
        balanceCriticalIntervalStart = Long.parseLong(config.getProperty("balance.critical.interval.start", Long.toString(balanceCriticalIntervalStart)));
      } catch (NumberFormatException e) {
        logger.error("Illegal long value: " + config.getProperty("balance.critical.interval.start"), e);
      }

      balanceCounter = new AvgCounter(responseTimeControllerPoolingInterval);

      if (bannerEngineClientEnabled) {
        try {
          bannerWarnIntervalStart = Long.parseLong(config.getProperty("banner.warning.interval.start", Long.toString(bannerWarnIntervalStart)));
        } catch (NumberFormatException e) {
          logger.error("Illegal long value: " + config.getProperty("banner.warning.interval.start"), e);
        }
        try {
          bannerMinorIntervalStart = Long.parseLong(config.getProperty("banner.minor.interval.start", Long.toString(bannerMinorIntervalStart)));
        } catch (NumberFormatException e) {
          logger.error("Illegal long value: " + config.getProperty("banner.minor.interval.start"), e);
        }
        try {
          bannerMajorIntervalStart = Long.parseLong(config.getProperty("banner.major.interval.start", Long.toString(bannerMajorIntervalStart)));
        } catch (NumberFormatException e) {
          logger.error("Illegal long value: " + config.getProperty("banner.major.interval.start"), e);
        }
        try {
          bannerCriticalIntervalStart = Long.parseLong(config.getProperty("banner.critical.interval.start", Long.toString(bannerCriticalIntervalStart)));
        } catch (NumberFormatException e) {
          logger.error("Illegal long value: " + config.getProperty("banner.critical.interval.start"), e);
        }

        try {
          bannerDisableOnErrorTimeout = Long.parseLong(config.getProperty("banner.disable.on.error.timeout", Long.toString(bannerDisableOnErrorTimeout)));
        } catch (NumberFormatException e) {
          logger.error("Illegal long value: " + config.getProperty("banner.disable.on.error.timeout"), e);
        }
        bannerCounter = new AvgCounter(responseTimeControllerPoolingInterval);

        boolean snmpAgentEnabled = Boolean.valueOf(config.getProperty("snmp.agent.enabled", "false")).booleanValue();
        if (snmpAgentEnabled) {
          snmpAgent = new Agent();
          try {
            snmpAgent.init(config);
          } catch (InitializationException e) {
            logger.error("Error while init SNMP Agent: " + e, e);
            snmpAgent = null;
          }
        }
      }

      (new ResponseTimeControllerThread(responseTimeControllerPoolingInterval)).startService();

    }

    new RequestStatesController(requestStatesControllerPollingInterval).startService();

    testMode = (new Boolean(config.getProperty("test.mode", Boolean.toString(testMode)))).booleanValue();

    if (!testMode) {
      try {
        connectionManager = DBConnectionManager.getInstance();
      } catch (InitializationException e) {
        logger.error("Could not get DB connection manager.", e);
        throw new InitializationException("Could not get DB connection manager.", e);
      }
    }

    if (logger.isDebugEnabled()) logger.debug("UniBalance SME init fineshed");
  }

  private void initResponsePatterns() throws InitializationException {
    Properties config;
    try {
      config = new Properties();
      config.load(new FileInputStream(responsePatternConfigFile));
    } catch (IOException e) {
      logger.error("Exception occured during loading response pattern configuration from " + responsePatternConfigFile.getName(), e);
      throw new InitializationException("Exception occured during loading response pattern configuration.", e);
    }

    balanceResponsePattern = config.getProperty("balance.response.pattern", balanceResponsePattern);
    waitForSmsResponsePattern = config.getProperty("balance.wait.for.sms.response.pattern", waitForSmsResponsePattern);
    errorPattern = config.getProperty("balance.error.pattern", errorPattern);

    bannerAddPattern = config.getProperty("balance.banner.add.pattern", bannerAddPattern);
  }

  public SmeEngine(Multiplexor multiplexor, OutgoingQueue messagesQueue, ThreadsPool threadsPool, ThreadsPool banneThreadsPool) {
    this.multiplexor = multiplexor;
    this.outgoingQueue = messagesQueue;
    this.threadsPool = threadsPool;
    this.bannerThreadsPool = banneThreadsPool;
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
      if (msg.getEncoding() == Message.ENCODING_BINARY) {
        if (logger.isDebugEnabled())
          logger.debug("SKIP MSG FROM #" + msg.getSourceAddress() + ". REASON: Unsupported encoding: BINARY");
        sendDeliverSmResponse(msg, Data.ESME_ROK);
        return true;
      }
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
    String responseType;
    switch (pdu.getType()) {
      case Data.SUBMIT_SM_RESP:
        responseType = "SUBMIT_SM_RESP";
        break;
      case Data.DATA_SM_RESP:
        responseType = "DATA_SM_RESP";
        break;
      case Data.ENQUIRE_LINK_RESP:
        responseType = "ENQUIRE_LINK_RESP";
        break;
      default:
        responseType = "UNKNOWN";
    }

    if (pdu.getType() == Data.SUBMIT_SM_RESP || pdu.getType() == Data.DATA_SM_RESP) {
      if (logger.isDebugEnabled()) {
        logger.debug(responseType + " handled. ConnID #" + pdu.getConnectionId() + "; SeqN #" + pdu.getSequenceNumber() + "; Status #" + pdu.getStatus());
      }
      if (outgoingQueue != null) {
        switch (pdu.getStatusClass()) {
          case PDU.STATUS_CLASS_TEMP_ERROR:
            int updateResult = outgoingQueue.updateOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatus());
            switch (updateResult) {
              case OutgoingQueueProxy.OUTGOING_OBJECT_UPDATED:
                break;
              case OutgoingQueueProxy.OUTGOING_OBJECT_REMOVED_BY_ERROR:
                rollbackSentBannerByResponse(pdu);
                break;
              case OutgoingQueueProxy.OUTGOING_OBJECT_REMOVED_BY_MAX_ATTEMPS_REACHED:
                rollbackSentBannerByResponse(pdu);
                break;
              case OutgoingQueueProxy.OUTGOING_OBJECT_NOT_FOUND:
                break;
            }

            break;
          case PDU.STATUS_CLASS_PERM_ERROR:
            outgoingQueue.removeOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatusClass());
            rollbackSentBannerByResponse(pdu);
            break;
          case PDU.STATUS_CLASS_NO_ERROR:
            outgoingQueue.removeOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatusClass());
            synchronized (bannerResponses) {
              bannerResponses.remove(new Long((((long) pdu.getConnectionId()) << 8) | (long) pdu.getSequenceNumber()));
            }
            break;
        }
      }
    }

  }

  private void rollbackSentBannerByResponse(PDU response) {
    BannerResp resp;
    synchronized (bannerResponses) {
      resp = (BannerResp) bannerResponses.get(new Long((((long) response.getConnectionId()) << 8) | (long) response.getSequenceNumber()));
    }
    if (resp != null) {
      if (logger.isDebugEnabled())
        logger.debug("Rollback banner by resp sn=" + response.getSequenceNumber() + "; status=" + response.getStatus());
      bannerThreadsPool.execute(new BannerRollbackThread(resp));
    }
  }

  protected void addRequestState(RequestState state) {
    synchronized (states) {
      states.put(state.getAbonentRequest().getSourceAddress(), state);
    }
    synchronized (statesExpire) {
      statesExpire.add(state);
    }
  }

  protected RequestState getRequestState(String abonent) {
    synchronized (states) {
      return (RequestState) states.get(abonent);
    }
  }

  protected RequestState removeRequestState(RequestState state) {
    synchronized (states) {
      return (RequestState) states.remove(state.getAbonentRequest().getSourceAddress());
    }
  }

  protected void closeRequestState(RequestState state) {
    String abonent = state.getAbonentRequest().getSourceAddress();
    synchronized (state) {
      if (state.isClosed()) return;
      if (!state.isError() && (!state.isBalanceReady() || (bannerEngineClientEnabled && !state.isBannerReady())))
        return;
      state.setClosed(true);
    }
    state.setStartCloseRequestTime();
    if (state.isError() || state.getAbonentResponse() == null) {
      Message message = new Message();
      message.setSourceAddress(state.getAbonentRequest().getDestinationAddress());
      message.setDestinationAddress(state.getAbonentRequest().getSourceAddress());
      if (state.getAbonentRequest().hasUssdServiceOp() && !state.isUssdSessionClosed()) {
        message.setUssdServiceOp(Message.USSD_OP_PROC_SS_REQ_RESP);
      } else {
        message.setDestAddrSubunit(1); // for Flash SMS
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

    if (bannerCounter != null && state.isBannerQueried()) {
      //bannerCounter.add(250 + (new Random()).nextInt(750) + state.getBannerResponseTime() - state.getBannerRequestTime());
      bannerCounter.add(state.getBannerResponseTime() - state.getBannerRequestTime());
    }
    balanceCounter.add(state.getBillingResponseDelay());

    state.setSendingResponseTime();
    sendResponse(state);
    state.setResponseSentTime();
    removeRequestState(state);
    if (logger.isInfoEnabled())
      logger.info(state.toString());
  }

  private void processRequestStates() {
    RequestState state;
    if (logger.isDebugEnabled()) {
      logger.debug("Processing requests count: " + states.size());
    }
    while (!statesExpire.isEmpty()) {
      long tm = System.currentTimeMillis();
      synchronized (statesExpire) {
        state = (RequestState) statesExpire.get(0);
        if (state.getAbonentRequestTime() + ussdSessionTimeout > tm) return;
        statesExpire.remove(0);
      }
      boolean sendWaitMessage = false;
      synchronized (state) {
        if (!state.isClosed() && !state.isUssdSessionClosed()) {
          // if state is in process state send ussd resp to release session and then it will be sms on closeRequestState
          logger.warn("Request expired for " + state.getAbonentRequest().getSourceAddress());
          sendWaitMessage = true;
          state.setUssdSessionClosed(true);
        } // else state already processed, skip it here
      }
      if (sendWaitMessage) {
        sendWaitForSmsMessage(state);
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
    if (bannerEngineClientEnabled && state.isBannerReady()) {
      if (!state.isError()) {
        String banner = state.getBanner();
        if (banner != null && banner.length() > 0) {
          MessageFormat bannerFormat = new MessageFormat(bannerAddPattern);
          String messageWithBanner = bannerFormat.format(new String[]{msg.getMessageString(), banner});
          if (messageWithBanner.length() <= ussdMaxLength) {
            msg.setMessageString(messageWithBanner);
          } else {
            logger.info("USSD response with banner too long: " + messageWithBanner);
          }
        } else {
          if (logger.isDebugEnabled())
            logger.debug("Banner is empty");
        }
      } else {
        if (logger.isDebugEnabled())
          logger.debug("Rollback banner by balance error for abonent " + state.getAbonentRequest().getSourceAddress());
        bannerThreadsPool.execute(new BannerRollbackThread(state.getBannerResponce()));
        state.setBannerResponse(null);
      }
    }
    if (((msg.getMessageString().length() > ussdMaxLength) || smsResponseMode) && !state.isUssdSessionClosed()) {
      sendWaitForSmsMessage(state);
      state.setUssdSessionClosed(true);
    }
    if (state.isUssdSessionClosed()) {
      msg.setUssdServiceOp(-1);
      msg.setDestAddrSubunit(1); // for Flash SMS
    }
    state.setAbonentResponseTime(System.currentTimeMillis());
    int sn = sendMessage(msg, state.getAbonentRequest().getConnectionName());
    if (sn != -1 && state.getBannerResponce() != null && bannerEngineClientEnabled) {
      long snKey = (((long) msg.getConnectionId()) << 8) | ((long) sn);
      synchronized (bannerResponses) {
        bannerResponses.put(new Long(snKey), state.getBannerResponce());
      }
    }
    if (responses != null) {
      responses.count();
    }
  }

  private int sendMessage(Message msg, String connectionName) {
    try {
      multiplexor.assingSequenceNumber(msg, connectionName);
      if (outgoingQueue != null) {
        outgoingQueue.addOutgoingObject(new OutgoingObject(msg));
      } else {
        multiplexor.sendMessage(msg, false);
      }

      if (logger.isDebugEnabled())
        logger.debug("MSG sent. ConnID #" + msg.getConnectionId() + "; SeqN #" + msg.getSequenceNumber() + "; USSD #" + msg.getUssdServiceOp() + "; destination #" + msg.getDestinationAddress() + "; source #" + msg.getSourceAddress() + "; msg: " + msg.getMessageString());

      return msg.getSequenceNumber();
    } catch (SMPPException e) {
      logger.error("Could not send message.", e);
    }
    return -1;
  }

  private void processIncomingMessage(Message message, long abonentRequestTime) {
    RequestState state;

    logger.info("Got request from " + message.getSourceAddress());
    state = getRequestState(message.getSourceAddress());

    if (state != null) {
      logger.warn("Request rejected because another request from this abonent is already processing");
      sendDeliverSmResponse(message, Data.ESME_RMSGQFUL);
      return;
    }

    if (states.size() >= maxProcessingRequests) {
      logger.warn("Request rejected because max processing requests count is reached");
      sendDeliverSmResponse(message, Data.ESME_RMSGQFUL);
      return;
    }

    if (requests != null) {
      requests.count();
    }

    state = new RequestState(message, abonentRequestTime);
    addRequestState(state);
    try {
      if (testMode) {
        threadsPool.execute(new DummyBalanceProcessor(this, state));
      } else {
        threadsPool.execute(new BalanceProcessor(this, state));
      }
    } catch (RuntimeException e) {
      logger.error("Exception occured during creating balance processor: " + e, e);
      state.setError(true);
      removeRequestState(state);
      sendDeliverSmResponse(message, Data.ESME_RTHROTTLED);
      return;
    }
    if (bannerEngineClientEnabled) {
      try {
        bannerThreadsPool.execute(new BannerRequestThread(state));
      } catch (RuntimeException e) {
        logger.error("Exception occured during creating banner processor: " + e, e);
      }
    }
    sendDeliverSmResponse(message, Data.ESME_ROK);
  }

  private String getBanner(RequestState state) {
    String abonent = state.getAbonentRequest().getSourceAddress();

    if (abonent.startsWith("+")) {
      abonent = ".1.1." + abonent.substring(1);
    } else if (!abonent.startsWith(".")) {
      abonent = ".1.1." + abonent;
    }

    // Send bannerRequest
    BannerReq req = new BannerReq(0, abonent, bannerEngineServiceName, AdvClientConst.SMS_TRANSPORT, bannerEngineBannerLength, bannerEngineCharSet);
    RequestContext rc = RequestContext.buildContext(req);

    try {
      bannerEngineClient.sendRequest(rc);
    } catch (AdvertisingClientException e) {
      logger.error("Error while sending BE request: "+e, e);
      return null;
    }

    if (logger.isDebugEnabled())
      logger.debug("BE rc.getState(): " + rc.getState());
    if (logger.isDebugEnabled())
      logger.debug("BE rc.getException(): " + rc.getException());

    BannerResp res = (BannerResp) rc.getResponse();
    if (res != null && res.getBannerBody() != null) {
      if (logger.isDebugEnabled())
        logger.debug(
            "BE got banlength=" + res.getBannerBody().length +
                " tranzact=" + res.getTransactionID() +
                " banner=" + Encode.decodeUTF16(res.getBannerBody()));

      state.setBannerResponse(res);

      switch (bannerEngineCharSet) {
        case(AdvClientConst.UTF16BE):
          return Encode.decodeUTF16(res.getBannerBody());
        case(AdvClientConst.GSMSMS):
          return Encode.decodeGSM(res.getBannerBody(), false);
        case(AdvClientConst.GSMUSSD):
          return Encode.decodeGSM(res.getBannerBody(), true);
        case(AdvClientConst.ASCII_TRANSLIT):
          return Encode.decodeASCII(res.getBannerBody());
        default:
          return Encode.decodeUTF16(res.getBannerBody());
      }
    } else {
      if (res != null) {
        if (logger.isDebugEnabled())
          logger.debug("BE COME NULL! tranzact= " + res.getTransactionID());
      } else {
        if (logger.isDebugEnabled())
          logger.debug("BE COME NULL!");
      }
      return null;
    }
  }

  private void rollbackBanner(BannerResp res) {
    RollBackReq rbr = new RollBackReq(res,
        ProtocolConsts.ERROR_CODE_ROLL_BACK);
    RequestContext rc = RequestContext.buildContext(rbr);

    try {
      bannerEngineClient.sendRequest(rc);
    } catch (AdvertisingClientException e) {
      logger.error("Error while sending BE rollback: "+e, e);
    }
  }

  protected MessageFormat getMessageFormat() {
    return new MessageFormat(balanceResponsePattern);
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
      try {
        Connection connection = stmt.getConnection();
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

  protected Connection getCbossConnection() throws SQLException {
    return connectionManager.getConnection(cbossPoolName);
  }

  class RequestStatesController extends Thread {

    private boolean started = false;
    private long pollingInterval = 5000L;
    private final Object monitor = new Object();
    private final Object shutmonitor = new Object();

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
        state.setBannerRequestTime(System.currentTimeMillis());
        String banner = getBanner(state);
        state.setBanner(banner, System.currentTimeMillis());
        closeRequestState(state);
      } catch (Throwable t) {
        state.setError(true);
        if (logger.isInfoEnabled())
          logger.info("Can not get banner for " + state.getAbonentRequest().getSourceAddress());
        logger.error("Unexpected exception occured during processing request.", t);
      }
    }
  }

  class BannerRollbackThread implements Runnable {
    private BannerResp resp;

    public BannerRollbackThread(BannerResp resp) {
      this.resp = resp;
    }

    public void run() {
      try {
        rollbackBanner(resp);
      } catch (Throwable t) {
        if (logger.isInfoEnabled())
          logger.info("Can not rollback banner " + resp);
        logger.error("Unexpected exception occured during processing request.", t);
      }
    }
  }

  class ResponcePatternConfigController extends Thread {
    private boolean started = false;
    private final Object monitor = new Object();
    private final Object shutmonitor = new Object();

    private long pollingInterval = 60000L;
    private long fileLastModified;

    public ResponcePatternConfigController(long pollingInterval) {
      super("ResponcePatternConfigController");
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

  class ResponseTimeControllerThread extends Thread {
    private boolean started = false;
    private final Object monitor = new Object();
    private final Object shutmonitor = new Object();

    private long pollingInterval = 601000L;

    public ResponseTimeControllerThread(long pollingInterval) {
      super("ResponseTimeControllerThread");
      this.pollingInterval = pollingInterval;
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

        long balanceAvgTime = balanceCounter.getAvg();
        if (logger.isDebugEnabled())
          logger.debug("Avg balance response time: " + balanceAvgTime);

        byte newBalanceSeverity = Agent.SNMP_SEVERITY_NORMAL;
        if (balanceAvgTime > balanceCriticalIntervalStart) {
          newBalanceSeverity = Agent.SNMP_SEVERITY_CRITICAL;
        } else if (balanceAvgTime > balanceMajorIntervalStart) {
          newBalanceSeverity = Agent.SNMP_SEVERITY_MAJOR;
        } else if (balanceAvgTime > balanceMinorIntervalStart) {
          newBalanceSeverity = Agent.SNMP_SEVERITY_MINOR;
        } else if (balanceAvgTime > balanceWarnIntervalStart) {
          newBalanceSeverity = Agent.SNMP_SEVERITY_WARNING;
        }

        if (newBalanceSeverity != currentBalanceSeverity) {
          if (logger.isDebugEnabled())
            logger.debug("Balance counter threshold crossed " + currentBalanceSeverity + "->" + newBalanceSeverity);

          byte status;
          if (newBalanceSeverity > currentBalanceSeverity) {
            status = Agent.SNMP_STATUS_NEW;
          } else {
            status = Agent.SNMP_STATUS_CLEAR;
          }
          currentBalanceSeverity = newBalanceSeverity;
          if (logger.isDebugEnabled())
            logger.debug("Generated trap: " + Agent.SNMP_STATUS[status] + " UNIBALANCE BillingSystem Threshold crossed (AlarmID=BillingSystem; severity=" + currentBalanceSeverity + ")");

          if (snmpAgent != null) {
            snmpAgent.sendTrap(status, "BillingSystem", currentBalanceSeverity);
          }
        } else {
          logger.debug("Balance counter severity unchanged");
        }

        if (bannerEngineClientEnabled) {
          long bannerAvgTime = bannerCounter.getAvg();
          if (bannerAvgTime != -1) {
            if (logger.isDebugEnabled())
              logger.debug("Avg banner response time: " + bannerAvgTime);

            byte newBannerSeverity = Agent.SNMP_SEVERITY_NORMAL;
            if (bannerAvgTime > bannerCriticalIntervalStart) {
              newBannerSeverity = Agent.SNMP_SEVERITY_CRITICAL;
            } else if (bannerAvgTime > bannerMajorIntervalStart) {
              newBannerSeverity = Agent.SNMP_SEVERITY_MAJOR;
            } else if (bannerAvgTime > bannerMinorIntervalStart) {
              newBannerSeverity = Agent.SNMP_SEVERITY_MINOR;
            } else if (bannerAvgTime > bannerWarnIntervalStart) {
              newBannerSeverity = Agent.SNMP_SEVERITY_WARNING;
            }

            if (newBannerSeverity != currentBannerSeverity) {
              if (logger.isDebugEnabled())
                logger.debug("Banner counter threshold crossed " + currentBannerSeverity + "->" + newBannerSeverity);

              byte status;
              if (newBannerSeverity > currentBannerSeverity) {
                status = Agent.SNMP_STATUS_NEW;
              } else {
                status = Agent.SNMP_STATUS_CLEAR;
              }
              currentBannerSeverity = newBannerSeverity;

              if (logger.isDebugEnabled())
                logger.debug("Generate trap: " + Agent.SNMP_STATUS[status] + " UNIBALANCE BannerRotator Threshold crossed (AlarmID=BannerRotator; severity=" + currentBannerSeverity + ")");

              if (snmpAgent != null) {
                snmpAgent.sendTrap(status, "BannerRotator", currentBannerSeverity);
              }

              if (status == Agent.SNMP_STATUS_NEW && currentBannerSeverity > Agent.SNMP_SEVERITY_WARNING && bannerDisableOnErrorTimeout > 0) {
                logger.info("BannerEngine client disabled for " + bannerDisableOnErrorTimeout + " ms");
                bannerEngineClientEnabled = false;
                (new BannerDisableTimeoutThread(bannerDisableOnErrorTimeout)).start();
              }
            } else {
              logger.debug("Banner counter severity unchanged");
            }
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

  class BannerDisableTimeoutThread extends Thread {
    private long timeout;

    public BannerDisableTimeoutThread(long timeout) {
      this.timeout = timeout;
    }

    public void run() {
      try {
        Thread.sleep(timeout);
      } catch (InterruptedException e) {
        logger.error("BannerDisableTimeoutThread was interrupted: " + e, e);
      }
      bannerCounter.reset();
      bannerEngineClientEnabled = true;
      logger.info("BannerEngine client enabled");
    }
  }

}
