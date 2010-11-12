package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.CSVTokenizer;
import org.apache.log4j.Logger;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 16:52:38
 */
public class DeliveryNotificationsProducer implements Runnable {
  
  Logger log = Logger.getLogger(this.getClass());
  private File baseDir, backupDir;
  private FileSystem fileSys;
  ScheduledExecutorService scheduler;
  final List<DeliveryNotificationsListener> listeners = Collections.synchronizedList(new LinkedList<DeliveryNotificationsListener>());


  public DeliveryNotificationsProducer(File directory,FileSystem fileSys) {
    this.baseDir = directory;
    this.fileSys = fileSys;
  }

  public void addListener(DeliveryNotificationsListener listener) {
      listeners.add(listener);
  }

  public void removeListener(DeliveryNotificationsListener listener) {
      listeners.remove(listener);
  }

  public void removeAllListeners() {
      listeners.clear();
  }


  public synchronized void start() {
    if(scheduler!=null) return;
    scheduler = Executors.newSingleThreadScheduledExecutor();
    scheduler.scheduleAtFixedRate(this,0,1000, TimeUnit.MILLISECONDS);
  }

  public synchronized void shutdown() {
    scheduler.shutdown();
    try {
      scheduler.awaitTermination(15,TimeUnit.SECONDS);
    }
    catch (InterruptedException e) {
      scheduler.shutdownNow();
    }
    scheduler=null;
  }


  public void run() {
    try {
      String[] files = fileSys.list(baseDir);
      Arrays.sort(files);
      for(String fileName : files) {
        if(fileName.endsWith(".csv")) {
          processFile(fileName);
        }
      }
    }
    catch (Exception e) {
      log.error("Fatal error, Notification producer EXITING! ", e);
      synchronized (this) {
        scheduler.shutdown();
        scheduler=null;
      }
    }
  }

  private void processFile(String fileName) throws Exception {
    File f = new File(baseDir,fileName);
    BufferedReader reader=null;
    try {
      Calendar c = Calendar.getInstance();
      c.setTime(new SimpleDateFormat("yyyyMMddHHmm'.csv'").parse(fileName));
      reader = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f)));
      String line;
      while((line = reader.readLine())!=null) {
        processLine(fileName, c,line);
      }
    }
    catch (Exception e) {
      log.error("Error parsing delivery states log : "+fileName, e);
    }
    finally {
      if(reader!=null) try {reader.close();} catch (Exception e){log.error("",e);}
      try {
        fileSys.rename(f,new File(backupDir,fileName));
      }
      catch (Exception e) {
        log.error("Error moving file to backup dir, EXITING! ", e);
        throw e;
      }
    }

  }

  private void processLine(String fileName, Calendar c, String line) {
    try {
      CSVTokenizer t = new CSVTokenizer(line);
      if(t.hasMoreTokens()) {
        DeliveryNotification notification;
        int ss = Integer.valueOf(t.nextToken());
        c.set(Calendar.SECOND,ss);
        int deliveryId = Integer.valueOf(t.nextToken());
        String userId = t.nextToken();
        DeliveryNotificationType type = getTypeByInt(Integer.valueOf(t.nextToken()));
        if(type== DeliveryNotificationType.MESSAGE_FINISHED) {
          //MSG_ID, STATUS, SMPP_STATUS, ADDRESS, USER_DATA
          long msgId = Long.valueOf(t.nextToken());
          MessageState messageState = getMessageState(t.nextToken());
          int smpp_status = Integer.valueOf(t.nextToken());
          Address addr = new Address(t.nextToken());
          String userData = t.nextToken();
          notification = new DeliveryMessageNotification(type,c.getTime(),deliveryId,userId,
                 msgId,messageState,smpp_status,addr,userData
              );
        }
        else {
          notification = new DeliveryNotification(type,c.getTime(),deliveryId,userId);
        }
        notifyListeners(notification);
      }
    }
    catch (Exception e) {
      log.error("Error processing log "+fileName+" line : "+line,e);
    }
  }

  private void notifyListeners(DeliveryNotification notification) {    
    for(DeliveryNotificationsListener listener : listeners) {
      listener.onDeliveryNotification(notification);
    }
  }

  private MessageState getMessageState(String s) {
    s = s.trim();
    if(s.length()==1) {
      switch (s.charAt(0)) {
        case 'E' : return MessageState.Expired;
        case 'D' : return MessageState.Delivered;
        case 'F' : return MessageState.Failed;
        default  :
      }
    }
    throw new IllegalArgumentException("invalid message event state ='"+s+"'");
  }

  private DeliveryNotificationType getTypeByInt(int value) {
    switch (value) {
      case 0 : return DeliveryNotificationType.MESSAGE_FINISHED;
      case 1 : return DeliveryNotificationType.DELIVERY_START;
      case 2 : return DeliveryNotificationType.DELIVERY_FINISHED;
      default: throw new IllegalArgumentException("Invalid event type = "+value);
    }

  }


}
