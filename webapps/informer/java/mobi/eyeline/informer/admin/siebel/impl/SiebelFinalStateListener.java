package mobi.eyeline.informer.admin.siebel.impl;

import com.eyeline.utils.FileUtils;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.siebel.SiebelDelivery;
import mobi.eyeline.informer.admin.siebel.SiebelException;
import mobi.eyeline.informer.admin.siebel.SiebelManager;
import mobi.eyeline.informer.admin.siebel.SiebelMessage;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelFinalStateListener extends DeliveryNotificationsListenerStub {

  private static final Logger logger = Logger.getLogger("SIEBEL");

  private final SiebelManager siebelManager;

  private final SiebelDeliveries deliveries;

  private final SiebelUserManager users;

  private Thread processor;

  private final ResourceBundle smppStatuses;

  private final File dir;

  private PrintWriter writer;

  private static final SimpleDateFormat df = new SimpleDateFormat("ddMMyyyy_HHmmss");

  private int periodSec;

  public static final String PERIOD_PARAM = "statsPeriod";

  private boolean stop = true;

  public SiebelFinalStateListener(SiebelManager siebelManager, SiebelDeliveries deliveries,
                                  SiebelUserManager users, File workDir, int periodSec) throws InitException{
    this.dir = new File(workDir, "siebel");
    if(!dir.exists() && !dir.mkdirs()) {
      throw new InitException("Can't create file: "+dir.getAbsolutePath());
    }
    this.siebelManager = siebelManager;
    this.deliveries = deliveries;
    this.users = users;
    this.smppStatuses = ResourceBundle.getBundle("mobi.eyeline.informer.admin.SmppStatus", Locale.ENGLISH);
    this.periodSec = periodSec;
  }

  private final Lock lockSiebelManager = new ReentrantLock();

  public void externalLock() {
    lockSiebelManager.lock();
  }

  public void externalUnlock() {
    lockSiebelManager.unlock();
  }

  private Lock writeLock = new ReentrantLock();

  private void writeLock() {
    writeLock.lock();
  }

  private void writeUnlock() {
    writeLock.unlock();
  }

  public int getPeriodSec() {
    return periodSec;
  }

  public void setPeriodSec(int periodSec) {
    if(logger.isDebugEnabled()) {
      logger.debug("Set Siebel Final state listeners period to: "+periodSec);
    }
    this.periodSec = periodSec;
  }

  private void messageFinished(DeliveryMessageNotification notification) throws AdminException {
    if(stop) {
      return;
    }
    if (notification.getUserData() == null) {
      return;
    }
    switch (notification.getMessageState()) {
      case New:
      case Process:
        return;
    }

    Map<String, String> userData = DcpConverter.convertUserData(notification.getUserData());

    String clcId = userData.get(UserDataConsts.SIEBEL_MESSAGE_ID);
    if (clcId == null) {
      return;
    }
    MessageState state = notification.getMessageState();
    int errCode = notification.getSmppStatus();
    try {
      writeLock();
      if(!stop) {
        writer.println(StringEncoderDecoder.toCSVString(0, clcId, state, errCode));
        writer.flush();
      }
    } finally {
      writeUnlock();
    }
  }

  private void deliveryFinished(DeliveryNotification notification) throws AdminException {
    if(stop) {
      return;
    }
    User u = users.getUser(notification.getUserId());
    if (u == null) {
      logger.error("Can't find user with id: " + notification.getUserId());
      return;
    }
    Delivery d = deliveries.getDelivery(u.getLogin(), u.getPassword(), notification.getDeliveryId());
    String waveId;
    if (d != null && (waveId = d.getProperty(UserDataConsts.SIEBEL_DELIVERY_ID)) != null) {
      try {
        writeLock();
        if(!stop) {
          writer.println(StringEncoderDecoder.toCSVString(1, waveId));
          writer.flush();
        }
      } finally {
        writeUnlock();
      }
    }
  }

  @Override
  public void onDeliveryFinishNotification(DeliveryNotification notification) throws AdminException {
    deliveryFinished(notification);
  }

  @Override
  public void onMessageNotification(DeliveryMessageNotification notification) throws AdminException {
    messageFinished(notification);
  }

  public synchronized void start() throws AdminException{
    if(stop) {
      try {
        writer = new PrintWriter(new FileWriter(new File(dir, df.format(new Date())+".csv")));
      } catch (IOException e) {
        logger.error(e,e);
        throw new SiebelException("internal_error");
      }
      processor =  new Thread(new Processor(), "SiebelFinalStateFileProcessor");
      processor.start();
      stop = false;
    }
  }

  public synchronized boolean isStarted() {
    return !stop;
  }


  public synchronized void shutdown() {
    if(!stop) {
      stop = true;
      processor.interrupt();
      try{
        writeLock();
        writer.close();
      }finally {
        writeUnlock();
      }
    }
  }


  private SiebelMessage.State getState(String st) {
    MessageState state = MessageState.valueOf(st);
    SiebelMessage.State siebelState;
    if (state == MessageState.Delivered) {
      siebelState = SiebelMessage.State.DELIVERED;
    } else if (state == MessageState.Expired) {
      siebelState = SiebelMessage.State.EXPIRED;
    } else if (state == MessageState.Failed) {
      siebelState = SiebelMessage.State.ERROR;
    } else {
      siebelState = SiebelMessage.State.UNKNOWN;
    }
    return siebelState;
  }

  private String getErrorDescr(String errCode) {
    String descr;
    try {
      descr = smppStatuses.getString("informer.errcode." + errCode);
    }
    catch (MissingResourceException ex) {
      descr = smppStatuses.getString("informer.errcode.unknown");
    }
    return descr;
  }

  private void processFile(File f) throws Exception {

    byte c = System.getProperty("line.separator").getBytes()[0];
    FileUtils.truncateFile(f, c, 2);

    BufferedReader reader = null;
    Map<String, SiebelMessage.DeliveryState> states = new HashMap<String, SiebelMessage.DeliveryState>(1001);
    Map<String, SiebelDelivery.Status> deliveries = new HashMap<String, SiebelDelivery.Status>(1001);
    try{
      reader = new BufferedReader(new FileReader(f));
      int countDeliveries = 0;
      int countMessages = 0;
      String line;
      while((line = reader.readLine()) != null) {
        List<String> list = StringEncoderDecoder.csvDecode(line);
        if(list.get(0).equals("0")) {
          states.put(list.get(1),
              new SiebelMessage.DeliveryState(getState(list.get(2)), list.get(3), getErrorDescr(list.get(3))));
          countMessages++;
        }else{
          deliveries.put(list.get(1), SiebelDelivery.Status.PROCESSED);
          countDeliveries++;
          if(logger.isDebugEnabled()) {
            logger.debug("Siebel delivery had been finished: waveId="+list.get(1));
          }
        }
        if(states.size() == 1000 || deliveries.size() == 1000) {
          if(!states.isEmpty()) {
            try{
              externalLock();
              siebelManager.setMessageStates(states);
            }finally {
              externalUnlock();
            }
            states.clear();
          }
          if(!deliveries.isEmpty()) {
            try{
              externalLock();
              siebelManager.setDeliveryStatuses(deliveries);
            }finally {
              externalUnlock();
            }
            deliveries.clear();
          }
        }
      }
      if(logger.isDebugEnabled()) {
        logger.debug("Deliveries finished had been processed: "+countDeliveries);
        logger.debug("Messages with final state had been processed: "+countMessages);
      }
    }finally {
      if(reader != null) {
        try{
          reader.close();
        }catch (IOException ignored){}
      }
    }

    if(!states.isEmpty()) {
      try{
        externalLock();
        siebelManager.setMessageStates(states);
      }finally {
        externalUnlock();
      }
      states.clear();
    }
    if(!deliveries.isEmpty()) {
      try{
        externalLock();
        siebelManager.setDeliveryStatuses(deliveries);
      }finally {
        externalUnlock();
      }
      deliveries.clear();
    }
  }

  private class Processor implements Runnable {

    public void run() {
      try{
        while(!stop) {
          if(logger.isDebugEnabled()) {
            logger.debug("Siebel final state processor has been started...");
          }
          try{
            try{
              writeLock();
              writer.close();
              writer = new PrintWriter(new FileWriter(new File(dir, df.format(new Date())+".csv")));
            }finally {
              writeUnlock();
            }

            File[] files = dir.listFiles();
            Arrays.sort(files, new Comparator<File>() {
              public int compare(File o1, File o2) {
                return o1.getName().compareTo(o2.getName());
              }});

            for(int i=0;i<files.length-1;i++) {
              File toProccess = files[i];
              if(toProccess.length()>0){
                processFile(toProccess);
              }
              if(!toProccess.delete()) {
                logger.error("Can't delete a file: "+toProccess.getAbsolutePath());
              }
            }
          }catch (Exception e){
            logger.error(e,e);
          }
          if(logger.isDebugEnabled()) {
            logger.debug("Siebel final state processor has been finished");
          }
          Thread.sleep(1000*periodSec);
        }
      }catch (InterruptedException ignored){
      }
    }


  }

}