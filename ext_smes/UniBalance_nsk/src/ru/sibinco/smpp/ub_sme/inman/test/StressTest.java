package ru.sibinco.smpp.ub_sme.inman.test;

import java.util.Random;
import java.util.Properties;
import java.util.Map;
import java.util.HashMap;
import java.io.*;

import org.apache.log4j.Category;
import ru.sibinco.smpp.ub_sme.inman.*;
import ru.sibinco.smpp.ub_sme.ProductivityControlObject;
import ru.sibinco.smpp.ub_sme.ProductivityController;
import ru.sibinco.smpp.ub_sme.InitializationException;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 19.12.2006
 * Time: 14:42:48
 * To change this template use File | Settings | File Templates.
 */
public class StressTest implements InManPDUHandler {
  private final static Category logger = Category.getInstance(StressTest.class);

  private String phone;
  private String phonePrefix;
  private long phoneNumber;

  private String host;
  private String port;
  private String startAbonent = "+79130000000";
  private long sleepInterval = 0L;
  private int maxRequestsPerSecond = 1000;
  private int maxConcurrentRequests = 200;
  private int maxRequestsCount = 0;
  private int abonentsCount = 1000000;
  private long shutdownTimeout = 5000L;

  private InManClient client;

  private Map requests = new HashMap();
  private int totalSent = 0;
  private int totalReceived = 0;
  private int totalErrors = 0;
  private long serverTime = 0;
  private PrintWriter responseStatsWriter = null;

  private ProductivityController pcontroller;
  private ProductivityControlObject senderSpeed = new ProductivityControlObject("Sender");
  private ProductivityControlObject receiverSpeed = new ProductivityControlObject("Receiver");

  private final static long NEXT_CONCURRENT_REQUEST_WAIT_INTERVAL = 1L;
  private final static long SHUTDOWN_WAIT_INTERVAL = 1000L;

  private boolean shutdown = false;

  private void init(String configFileName) throws InitializationException {
    Properties config = new Properties();
    try {
      config.load(new FileInputStream(configFileName));
    } catch (IOException e) {
      throw new InitializationException(e);
    }
    init(config);
  }

  private void init(Properties config) throws InitializationException {
    host = config.getProperty("inman.host", "localhost");
    port = config.getProperty("inman.port", "5678");
    startAbonent = config.getProperty("start.abonent", startAbonent);
    try {

      abonentsCount = Integer.parseInt(config.getProperty("abonents.count", Integer.toString(abonentsCount)));

      maxRequestsCount = Integer.parseInt(config.getProperty("max.requests.count", Integer.toString(maxRequestsCount)));
      maxRequestsPerSecond = Integer.parseInt(config.getProperty("max.requests.per.second", Integer.toString(maxRequestsPerSecond)));
      maxConcurrentRequests = Integer.parseInt(config.getProperty("max.concurrent.requests", Integer.toString(maxConcurrentRequests)));

      sleepInterval = Long.parseLong(config.getProperty("sleep.interval", Long.toString(sleepInterval)));

      shutdownTimeout = Long.parseLong(config.getProperty("shutdown.timeout", Long.toString(shutdownTimeout)));

    } catch (NumberFormatException e) {
      throw new InitializationException("Number property format error: " + e, e);
    }

    String responseStatsFileName = config.getProperty("response.stats.file.name", "");

    setPhone(startAbonent);

    client = new InManClient(host, port, this);

    if (!client.connect()) {
      throw new InitializationException("Couldn't connect to inman");
    }

    if(responseStatsFileName.length()>0){
      try {
        responseStatsWriter = new PrintWriter(new FileOutputStream(responseStatsFileName));
      } catch (FileNotFoundException e) {
        throw new InitializationException("Response stats file error: " + e, e);
      }
    }

    pcontroller = new ProductivityController();
    pcontroller.init(config);

    pcontroller.addControlObject(senderSpeed);
    pcontroller.addControlObject(receiverSpeed);
    pcontroller.startService();

    Runtime.getRuntime().addShutdownHook(new ShutdownThread());
  }

  public void main() {

    Random rand = new Random();

    int count = 0;
    long startTm = System.currentTimeMillis();
    while (!shutdown && (totalSent < maxRequestsCount || maxRequestsCount==0)) {
      if (maxRequestsPerSecond > 0 && count > maxRequestsPerSecond) {
        count = 0;
        long now = System.currentTimeMillis();
        if (now - startTm < 1000) {
          try {
            Thread.sleep(1000 - (now - startTm));
          } catch (InterruptedException e) {
            logger.warn(e.getMessage(), e);
          }
        }
        startTm = System.currentTimeMillis();
      }
      while (maxConcurrentRequests > 0 && requests.size() >= maxConcurrentRequests) {
        try {
          Thread.sleep(NEXT_CONCURRENT_REQUEST_WAIT_INTERVAL);
        } catch (InterruptedException e) {
          logger.warn(e.getMessage(), e);
        }
      }
      try {
        int n = rand.nextInt(abonentsCount);
        String phone = getPhoneIndexed(n);
        int dialogID = client.assignDialogID();
        if (logger.isDebugEnabled())
          logger.debug("Send contract type request for abonent: " + phone + ", dialogID="+dialogID);
        synchronized (requests) {
          requests.put(new Integer(dialogID), new Long(System.currentTimeMillis()));
        }
        client.sendContractRequest(phone, dialogID, true);
        senderSpeed.count();
        count++;
        totalSent++;
        if (sleepInterval > 0) {
          try {
            Thread.sleep(sleepInterval);
          } catch (InterruptedException e) {
            logger.error(e);
          }
        }
      } catch (InManClientException e) {
        logger.error("InManClientException: " + e, e);
        break;
      }
    }

    shutdown();
  }

  public void handleInManPDU(InManPDU pdu) {
    try {
      totalReceived++;
      AbonentContractResult acr = (AbonentContractResult) pdu;
      Long request;
      synchronized (requests) {
        request = (Long) requests.remove(new Integer(acr.getDialogID()));
      }
      if (request != null) {
        long responseTime = System.currentTimeMillis() - request.longValue();
        serverTime=serverTime+responseTime;
        if(responseStatsWriter!=null){
          responseStatsWriter.println(responseTime);
        }
        if(logger.isDebugEnabled()){
          logger.debug("Got response dialogID=" + acr.getDialogID() + ", contractType: " + acr.getContractType() + ", nmPolicy: " + acr.getNmPolicy() + ", gsmSCFAddress: " + acr.getGsmSCFAddress() + ", serviceKey: " + acr.getServiceKey() + ", error: " + acr.getError()+", response time: "+responseTime+" ms");
        }
        if(acr.getError()!=-1){
          totalErrors++;
        }
      } else {
        logger.warn("Got response for unknown request dialogID=" + acr.getDialogID() + ", contractType: " + acr.getContractType() + ", nmPolicy: " + acr.getNmPolicy() + ", gsmSCFAddress: " + acr.getGsmSCFAddress() + ", serviceKey: " + acr.getServiceKey() + ", error: " + acr.getError());
      }
    } catch (InManPDUException e) {
      logger.error(e.getMessage(), e);
    }
    receiverSpeed.count();
  }

  public void setPhone(String phone) {
    this.phone = phone;
    if (phone != null) {
      int c = 0;
      while (!Character.isDigit(phone.charAt(c))) {
        c++;
      }
      if (c > 0) {
        phonePrefix = phone.substring(0, c);
        phone = phone.substring(c);
      }
      phoneNumber = Long.parseLong(phone);
    }
  }

  public String getPhoneIndexed(int index) {
    if (index == 0)
      return phone;
    long newPhone = phoneNumber + index;
    return (phonePrefix != null) ? (phonePrefix + String.valueOf(newPhone)) : (String.valueOf(newPhone));
  }

  public static void main(String[] args) {

    if (args.length < 1) {
      System.out.println("Usage: java ru.sibinco.smpp.ub_sme.inman.test.StressTest [config_file]");
      return;
    }

    String configFileName = args[0];
    StressTest inst = new StressTest();

    try {
      inst.init(configFileName);
    } catch (InitializationException e) {
      logger.error("Initialization error: " + e, e);
      return;
    }
    inst.main();
  }

  public void shutdown(){
    if(shutdown){
      return;
    }
    shutdown = true;
    long shutdownStarted = System.currentTimeMillis();
    logger.info("Shutdown started...");
    while(requests.size()>0 && (System.currentTimeMillis()-shutdownStarted < shutdownTimeout)){
      try {
        Thread.sleep(SHUTDOWN_WAIT_INTERVAL);
      } catch (InterruptedException e) {
        logger.warn(e.getMessage(), e);
      }
    }
    client.close();
    pcontroller.stopService();
    if(responseStatsWriter!=null){
      responseStatsWriter.flush();
      responseStatsWriter.close();
    }
    logger.info("Shutdown finished");
    logger.info("Total requests sent: "+totalSent);
    logger.info("Total responses received: "+totalReceived);
    logger.info("Total errors: "+totalErrors);
    logger.info("Avg response time: "+serverTime/totalReceived);
  }

  class ShutdownThread extends Thread {
    public void run() {
      shutdown();
    }
  }
}
