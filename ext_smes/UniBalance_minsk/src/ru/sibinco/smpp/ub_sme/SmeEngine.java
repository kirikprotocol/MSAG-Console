package ru.sibinco.smpp.ub_sme;

import com.logica.smpp.Data;
import com.lorissoft.advertising.syncclient.AdvertisingClientImpl;
import com.lorissoft.advertising.syncclient.IAdvertisingClient;
import ru.aurorisoft.smpp.*;
import ru.sibinco.smpp.ub_sme.util.DBConnectionManager;
import ru.sibinco.smpp.ub_sme.util.Utils;
import ru.sibinco.util.threads.ThreadsPool;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.sql.CallableStatement;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.text.MessageFormat;
import java.text.NumberFormat;
import java.util.*;

public class SmeEngine implements MessageListener, ResponseListener {

  private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(SmeEngine.class);

  protected final static byte BILLING_SYSTEM_CBOSS = 0;

  public final static int BILLING_SYSTEMS_COUNT = 1;

  public final static String[] BILLING_SYSTEMS = {"CBOSS"};

  private String cbossPoolName = null;
  private String cbossQuery = null;

  private String cbossConnectionErrorPattern = "Connection,NullPointerException";

  private byte[] billingSystemsOrder = {1};

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
  private String numberFormatPattern = "#.##";

  private File responsePatternConfigFile;

  private String balanceResponsePattern = "{0} {1}";
  private String waitForSmsResponsePattern = "{0}";
  private String bannerAddPattern = "{0}\n{1}";
  private String errorPattern = "Error occurred";

  private int ussdMaxLength = 67;
  private long ussdSessionTimeout = 5000L;

  private int maxProcessingRequests = 10000;

  private ThreadsPool threadsPool = null;
  private ThreadsPool bannerThreadsPool = null;
  private Multiplexor multiplexor = null;
  private OutgoingQueue outgoingQueue;

  private DBConnectionManager connectionManager = DBConnectionManager.getInstance();
  private Map states = new HashMap();
  private List statesExpire = new LinkedList();

  private Map cbossStatements = new HashMap();

  private ProductivityControlObject requests;
  private ProductivityControlObject responses;
  private ProductivityControlObject waitForSmsResponses;

  private int bannerEngineTransactionId;
  private final Object bannerEngineTransactionIdSyncMonitor = new Object();

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

    for (int i = 0; i < billingSystemsOrder.length; i++) {
      String bso = config.getProperty("billing.system." + i + ".order", "");
      if (bso.length() > 0) {
        try {
          billingSystemsOrder[i] = Byte.parseByte(bso);
        } catch (NumberFormatException e) {
          throw new InitializationException("Invalid value for config parameter \"billing.system." + i + ".order\": " + bso);
        }
      }
    }

    numberFormatPattern = config.getProperty("balance.number.format.pattern", numberFormatPattern);
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
    waitForSmsResponsePattern = patternConfig.getProperty("balance.wait.for.sms.response.pattern", waitForSmsResponsePattern);
    errorPattern = patternConfig.getProperty("balance.error.pattern", errorPattern);

    bannerAddPattern = patternConfig.getProperty("balance.banner.add.pattern", bannerAddPattern);

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
        outgoingQueue.removeOutgoingObject(pdu.getConnectionId(), pdu.getSequenceNumber(), pdu.getStatusClass());
      }
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

  /*protected RequestState extractRequestState(String abonent) {
    synchronized (states) {
      return (RequestState)states.remove(abonent);
    }
  } */

  /*protected void closeRequestState(String abonent) {
    if (abonent == null) {
      return;
    }
    RequestState state = extractRequestState(abonent);
    if (state == null)
      return;
    closeRequestState(state);
  }*/

  protected void closeRequestState(RequestState state) {
    String abonent = state.getAbonentRequest().getSourceAddress();
    synchronized (state) {
      if( state.isClosed() ) return;
      if( !state.isError() && (!state.isBalanceReady() || (bannerEngineClientEnabled && !state.isBannerReady())) ) return;
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
      msg.setDestAddrSubunit(1); // for Flash SMS
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
      logger.error("Could not send message.", e);
    }
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
      threadsPool.execute(new BalanceProcessor(this, state));
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

  private String getBanner(String abonent) {
    if (abonent.startsWith("+")) {
      abonent = ".1.1." + abonent.substring(1);
    } else if (!abonent.startsWith(".")) {
      abonent = ".1.1." + abonent;
    }
    String encoding = "UTF-16BE";
    int transactionId;
    synchronized (bannerEngineTransactionIdSyncMonitor) {
      transactionId = bannerEngineTransactionId++;
    }
    byte[] banner = null;
    if (bannerEngineClientEnabled) {
      banner = bannerEngineClient.getLikelyBanner(abonent.getBytes(), abonent.getBytes().length, bannerEngineServiceName.getBytes(), bannerEngineTransportType, 140, bannerEngineCharSet, bannerEngineClientID, transactionId);
    }
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

  protected NumberFormat getNumberFormat() {
    return new DecimalFormat(numberFormatPattern, decimalFormatSymbols);
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

  public boolean isCbossConnectionError(Exception e) {
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
        state.setBannerRequestTime(System.currentTimeMillis());
        String banner = getBanner(state.getAbonentRequest().getSourceAddress());
        state.setBanner(banner);
        closeRequestState(state);
      } catch (Throwable t) {
        state.setError(true);
        if (logger.isInfoEnabled())
          logger.info("Can not get banner for " + state.getAbonentRequest().getSourceAddress());
        logger.error("Unexpected exception occured during processing request.", t);
      }
    }
  }

  class ResponcePatternConfigController extends Thread {
    private boolean started = false;
    private Object monitor = new Object();
    private Object shutmonitor = new Object();

    private long pollingInterval = 60000L;
    private long fileLastModified;

    public ResponcePatternConfigController(long pollingInterval) {
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
