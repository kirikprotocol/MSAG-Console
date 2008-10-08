package ru.sibinco.smpp.textservice;

import ru.sibinco.smpp.*;

import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;
import javax.mail.Message;
import javax.mail.Session;
import javax.mail.Transport;
import java.util.Properties;
import java.util.Date;
import java.util.regex.Pattern;
import java.io.*;
import java.text.SimpleDateFormat;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: 03.10.2005
 * Time: 12:34:28
 * To change this template use File | Settings | File Templates.
 */
public class TextServiceProcessor implements RequestProcessor {

  private final static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(TextServiceProcessor.class);

  private String name = "TextServiceProcessor";
  private String configFileName = null;
  private Properties config = null;
  private final static String checkRequiredSuffix = ".check.enabled";
  private final static String checkSuffix = ".check.regexp";
  private final static String checkedTextSuffix = ".checked.response.text";
  private final static String uncheckedTextSuffix = ".unchecked.response.text";
  private final static String responseTextSuffix = ".response.text";
  private final static String defaultResponse = "default.response.text";
  private final static String emailSuffix=".email";
  private final static String defaultEmail = "default.email";
  private ConfigLoader configLoader = null;
  private Session session = null;
  private SimpleDateFormat sdf = new SimpleDateFormat("dd.MM.yyyy HH:mm");
  private boolean reuseConnection = true;

  public void init(String name, String configFileName) throws InitializationException {
    this.configFileName = configFileName;
    if (configFileName == null || configFileName.trim().length() == 0)
      throw new InitializationException("Could not load config from NULL.");
    init(name, load(configFileName));
  }

  public void init(String name, Properties properties) throws InitializationException {
    // set name
    if (name != null && name.trim().length() > 0)
      this.name = name.trim();
    // check default response
    if (properties.getProperty(defaultResponse, "").trim().equals(""))
      throw new InitializationException("\"" + defaultResponse + "\" preperty is mandatory but not set.");
    // set or replace config
    if (config == null || !properties.equals(config))
      config = properties;
    // start config reloader
    if (configLoader == null) {
      try {
        configLoader = new ConfigLoader(this, Long.parseLong(config.getProperty("reloading.interval", "60000L")));
        configLoader.startService();
      } catch (Exception e) {
        throw new InitializationException("Could not start config reloader.", e);
      }
    }
    if( config.getProperty("reuse.connection") != null && config.getProperty("reuse.connection").equalsIgnoreCase("false") ) {
      reuseConnection = false;
    }
    Logger.info("Reuse connection feature set to "+reuseConnection);
    if (session == null) {
      session = Session.getInstance(config, null);
    }
    Logger.info(getName()+" initialized successfully.");
  }

  private Properties load(String configFileName) throws InitializationException {
    Properties properties = null;
    BufferedReader br = null;
    try {
      br = new BufferedReader(new InputStreamReader(this.getClass().getClassLoader().getResourceAsStream(configFileName)));
      properties = new Properties();
      String line = null;
      while ((line = br.readLine()) != null) {
        line = line.trim();
        if (!line.startsWith("#") && !line.equals("")) {
          int index = -1;
          if ((index = line.indexOf("=")) > -1) {
            String key = line.substring(0, index);
            String value = line.substring(index+1);
            properties.setProperty(key, value);
            Logger.debug("Parsed property: "+key+"="+value);
          }
        }
      }
    } catch (Exception e) {
      throw new InitializationException("Could not load config.", e);
    } finally {
      if (br != null) {
        try {
          br.close();
        } catch (IOException e) {
          Logger.warn("Could not close reader.", e);
        }
      }
    }
    if (properties == null || properties.size() == 0)
      throw new InitializationException("Couldnot work with empty config.");
    return properties;
  }

  public String getName() {
    return name;
  }

  public Response process(MessageData messageData) throws RequestProcessingException {
    String response = config.getProperty(defaultResponse);
    if ((config.getProperty(messageData.getDestinationAddress() + checkRequiredSuffix, "").equalsIgnoreCase("1") ||
        config.getProperty(messageData.getDestinationAddress() + checkRequiredSuffix, "").equalsIgnoreCase("yes") ||
        config.getProperty(messageData.getDestinationAddress() + checkRequiredSuffix, "").equalsIgnoreCase("true")) &&
        !config.getProperty(messageData.getDestinationAddress() + checkSuffix, "").equals("") &&
        !config.getProperty(messageData.getDestinationAddress() + checkedTextSuffix, "").equals("") &&
        !config.getProperty(messageData.getDestinationAddress() + uncheckedTextSuffix, "").equals("")) {
      // check request
      if (Pattern.matches(config.getProperty(messageData.getDestinationAddress() + checkSuffix), messageData.getMessageString())) {
        response = config.getProperty(messageData.getDestinationAddress() + checkedTextSuffix);
      } else {
        response = config.getProperty(messageData.getDestinationAddress() + uncheckedTextSuffix);
      }
    } else if (!config.getProperty((messageData.getDestinationAddress() + responseTextSuffix), "").equals("")) {
      response = config.getProperty((messageData.getDestinationAddress() + responseTextSuffix));
    }
    sendEmail(messageData);
    return getSimpleResponse(response, messageData);
  }

  protected Response getSimpleResponse(String data, MessageData request) {
    MessageData respMessageData = new MessageData();
    respMessageData.setDestinationAddress(request.getSourceAddress());
    respMessageData.setSourceAddress(request.getDestinationAddress());
    respMessageData.setMessageString(data);
    if( reuseConnection ) respMessageData.setConnectionName(request.getConnectionName());
    respMessageData.setTransactionId(request.getTransactionId());
    respMessageData.setAddressPrefix(request.getAddressPrefix());
    if (request.hasUssdServiceOp()) {
      if (request.getUssdServiceOp() == MessageData.USSD_OP_PROC_SS_REQ_IND) {
        respMessageData.setUssdServiceOp(MessageData.USSD_OP_PROC_SS_REQ_RESP);
      } else {
        Logger.warn("Unexpacted ussd_service_op="+request.getUssdServiceOp()+"! Use non USSD response.");
      }
    }
    return new Response(Response.TYPE_ARRAY, new MessageData[]{respMessageData});
  }

  private void sendEmail(MessageData data) {
    String to = config.getProperty(defaultEmail);
    if (config.getProperty(data.getDestinationAddress()+emailSuffix) != null)
      to = config.getProperty(data.getDestinationAddress()+emailSuffix);
    if (to == null || to.length() == 0) {
      Logger.warn("Could not find email for "+data.getDestinationAddress());
      return;
    }
    try {
      Message msg = new MimeMessage(session);
      msg.setFrom(new InternetAddress(config.getProperty("sender.address"), false));
      msg.setRecipient(Message.RecipientType.TO, new InternetAddress(to, false));
      msg.setSubject(data.getDestinationAddress());
      StringBuffer sb = new StringBuffer("");
      sb.append(data.getSourceAddress());
      sb.append(",");
      sb.append(sdf.format(new Date()));
      sb.append(",");
      sb.append(data.getMessageString());
      msg.setText(sb.toString());
      msg.setHeader("X-Mailer", "msgsend");
      msg.setSentDate(new Date(System.currentTimeMillis()));
      Transport.send(msg);
    } catch (Exception e) {
      Logger.warn("Could not email notification to: "+to, e);
    }

  }

  class ConfigLoader extends Thread {

    private TextServiceProcessor processor = null;
    private boolean started = false;
    private long reloadingInterval = 60000L;
    private Object monitor = new Object();
    private Object shutmonitor = new Object();

    public ConfigLoader(TextServiceProcessor processor, long pollingInterval) {
      this.processor= processor;
      if (pollingInterval < 60000)
        Logger.info(getName() + " config reloader use default reloading interval: 60000 millis.");
      else {
        Logger.info(getName() + " config reloader use " + pollingInterval + " reloading interval.");
        this.reloadingInterval = pollingInterval;
      }
      Logger.info(getName() + " config reloader created.");
    }

    public void startService() {
      started = true;
      Logger.info(getName() + " config reloader started.");
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
          Logger.warn(getName() + " config reloader shutting down exception:", e);
        }
      }
    }

    public boolean isStarted() {
      return started;
    }

    public void run() {
      while (true) {
        try {
          processor.init(processor.getName(), processor.configFileName);
        } catch (InitializationException e) {
          Logger.warn(getName()+" config reloader could not reload config from file: "+processor.configFileName, e);
        }
        synchronized (monitor) {
          if (!started) break;
          try {
            monitor.wait(reloadingInterval);
          } catch (InterruptedException e) {
            Logger.debug(getName() + " config reloader was interrupted.", e);
          }
        }
      }
      synchronized (shutmonitor) {
        shutmonitor.notifyAll();
      }
    }
  }
}