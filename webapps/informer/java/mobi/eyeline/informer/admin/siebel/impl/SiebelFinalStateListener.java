package mobi.eyeline.informer.admin.siebel.impl;

import com.eyeline.utils.FileUtils;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.siebel.SiebelDelivery;
import mobi.eyeline.informer.admin.siebel.SiebelManager;
import mobi.eyeline.informer.admin.siebel.SiebelMessage;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.StringEncoderDecoder;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelFinalStateListener implements DeliveryNotificationsListener {

  private static final Logger logger = Logger.getLogger(SiebelFinalStateListener.class);

  private final SiebelManager siebelManager;

  private final SiebelDeliveries deliveries;

  private final SiebelUserManager users;

  private final ScheduledExecutorService commiter;

  private final ResourceBundle smppStatuses;

  private final File cache;

  private final File cacheTmp;

  private final File toProccess;

  private PrintWriter writer;

  public SiebelFinalStateListener(SiebelManager siebelManager, SiebelDeliveries deliveries,
                                  SiebelUserManager users, File workDir, int periodSec) throws InitException{
    this.cache = new File(workDir, "siebelFinalStates.csv");
    this.cacheTmp = new File(workDir, "siebelFinalStates.tmp.csv");
    this.toProccess = new File(workDir, "siebelFinalStates.process.csv");
    this.siebelManager = siebelManager;
    this.deliveries = deliveries;
    this.users = users;
    this.smppStatuses = ResourceBundle.getBundle("mobi.eyeline.informer.admin.SmppStatus", Locale.ENGLISH);
    repair();
    try {
      writer = new PrintWriter(new FileWriter(cache));
    } catch (IOException e) {
      logger.error(e,e);
      throw new InitException(e);
    }
    this.commiter = Executors.newSingleThreadScheduledExecutor();
    this.commiter.scheduleAtFixedRate(new Processor(), periodSec, periodSec, TimeUnit.SECONDS); //todo?
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

  private void messageFinished(DeliveryMessageNotification notification) throws AdminException {
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
      writer.println(StringEncoderDecoder.toCSVString(0, clcId, state, errCode));
      writer.flush();
    } finally {
      writeUnlock();
    }
  }

  private void deliveryFinished(DeliveryNotification notification) throws AdminException {
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
        writer.println(StringEncoderDecoder.toCSVString(1, waveId));
        writer.flush();
      } finally {
        writeUnlock();
      }
    }
  }

  public void onDeliveryNotification(DeliveryNotification notification) {
    try {
      switch (notification.getType()) {
        case DELIVERY_FINISHED:
          deliveryFinished(notification);
          break;
        case MESSAGE_FINISHED:
          messageFinished((DeliveryMessageNotification) notification);
      }
    } catch (Exception e) {
      logger.error(e, e);
    }
  }

  public void shutdown() {
    if (commiter != null) {
      commiter.shutdown();
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
    BufferedReader reader = null;
    Map<String, SiebelMessage.DeliveryState> states = new HashMap<String, SiebelMessage.DeliveryState>(1001);
    Map<String, SiebelDelivery.Status> deliveries = new HashMap<String, SiebelDelivery.Status>(1001);
    try{
      reader = new BufferedReader(new FileReader(f));
      String line;
      while((line = reader.readLine()) != null) {
        List<String> list = StringEncoderDecoder.csvDecode(line);
        if(list.get(0).equals("0")) {
          states.put(list.get(1),
              new SiebelMessage.DeliveryState(getState(list.get(2)), list.get(3), getErrorDescr(list.get(3))));
        }else{
          deliveries.put(list.get(1), SiebelDelivery.Status.PROCESSED);
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

  private void copy(File f, PrintWriter to) throws IOException{
    BufferedReader r = null;
    try{
      r = new BufferedReader(new FileReader(f));
      String l;
      while((l = r.readLine()) != null) {
        to.println(l);
        to.flush();
      }
    }finally {
      if(r != null) {
        try{
          r.close();
        }catch (IOException ignored){}
      }
    }
  }



  private void repair() throws InitException{
    try{

      byte c = System.getProperty("line.separator").getBytes()[0];

      if(toProccess.exists()) {
        FileUtils.truncateFile(toProccess, c, 2);
      }

      if(cache.exists()) {
        FileUtils.truncateFile(cache, c, 2);
        PrintWriter w = null;
        try{
          w = new PrintWriter(new BufferedWriter(new FileWriter(toProccess, true)));
          copy(cache, w);
        }finally {
          if(w != null) {
            w.close();
          }
        }
        if(!cache.delete()) {
          logger.error("Can't delete a file: "+cache.getAbsolutePath());
        }
      }

      if(cacheTmp.exists()) {
        FileUtils.truncateFile(cacheTmp, c, 2);
        PrintWriter w = null;
        try{
          w = new PrintWriter(new BufferedWriter(new FileWriter(toProccess, true)));
          copy(cacheTmp, w);
        }finally {
          if(w != null) {
            w.close();
          }
        }
        if(!cacheTmp.delete()) {
          logger.error("Can't delete a file: "+cacheTmp.getAbsolutePath());
        }
      }
      if(toProccess.exists()) {
        processFile(toProccess);
      }
      if(!toProccess.delete()) {
        logger.error("Can't delete a file: "+toProccess.getAbsolutePath());
      }
    }catch (Exception e){
      logger.error(e,e);
      throw new InitException(e);
    }
  }

  private class Processor implements Runnable {

    public void run() {
      try{
        try{
          writeLock();
          writer.close();
          writer = new PrintWriter(cacheTmp);
        }finally {
          writeUnlock();
        }
        PrintWriter w = null;
        try{
          w = new PrintWriter(new BufferedWriter(new FileWriter(toProccess, true)));

          copy(cache, w);

          if(!cache.delete()) {
            logger.error("Can't delete a file: "+cache.getAbsolutePath());
          }

          try{
            writeLock();
            writer.close();
            writer = new PrintWriter(cache);
          }finally {
            writeUnlock();
          }

          copy(cacheTmp, w);

          if(!cacheTmp.delete()) {
            logger.error("Can't delete a file: "+cacheTmp.getAbsolutePath());
          }

        }finally {
          if(w != null) {
            w.close();
          }
        }
        if(toProccess.length()>0){
          processFile(toProccess);
        }
        if(!toProccess.delete()) {
          logger.error("Can't delete a file: "+toProccess.getAbsolutePath());
        }
      }catch (Exception e){
        logger.error(e,e);
      }
    }

  }


}
