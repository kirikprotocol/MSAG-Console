package mobi.eyeline.informer.admin.delivery.changelog;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.notifications.DateAndFile;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.CSVTokenizer;
import mobi.eyeline.informer.util.Functions;
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
 * Объект, отслеживающий изменения в рассылках и оповещающий об этом своих подписчиков
 *
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 12.11.2010
 * Time: 16:52:38
 */
public class DeliveryChangesDetector implements Runnable {

  Logger log = Logger.getLogger(this.getClass());

  private static final TimeZone STAT_TIMEZONE=TimeZone.getTimeZone("UTC");

  private static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

  private File baseDir;
  private File backupDir;
  private FileSystem fileSys;
  ScheduledExecutorService scheduler;
  final List<DeliveryChangeListener> listeners = Collections.synchronizedList(new LinkedList<DeliveryChangeListener>());
  private static final String FILE_NAME_DATE_PATTERN = "yyyyMMddHHmm'.csv'";


  public DeliveryChangesDetector(File directory, FileSystem fileSys) throws InitException {
    this.baseDir = directory;
    this.fileSys = fileSys;
    backupDir = new File(baseDir, "processedFiles");
    try {
      if (!fileSys.exists(backupDir)) {
        fileSys.mkdirs(backupDir);
      }
    }
    catch (AdminException e) {
      throw new InitException("Can't create dir for processed files :" + backupDir, e);
    }

  }

  public void addListener(DeliveryChangeListener listener) {
    listeners.add(listener);
  }

  public void removeListener(DeliveryChangeListener listener) {
    listeners.remove(listener);
  }

  public void removeAllListeners() {
    listeners.clear();
  }


  public synchronized void start() {
    if (scheduler != null) return;
    scheduler = Executors.newSingleThreadScheduledExecutor();
    scheduler.scheduleAtFixedRate(this, 0, 1000, TimeUnit.MILLISECONDS);
  }

  public synchronized void shutdown() {
    if (scheduler == null) return;
    scheduler.shutdown();
    try {
      scheduler.awaitTermination(15, TimeUnit.SECONDS);
    }
    catch (InterruptedException e) {
      scheduler.shutdownNow();
    }
    scheduler = null;
  }


  public void run() {
    String[] files=null;
    try {
       files = fileSys.list(baseDir);        
    }
    catch (Exception e) {
      log.error("Fatal error,EXITING! can't access baseDir: "+baseDir.getAbsolutePath(), e);
      shutdown();
    }

    if (files == null) {
      log.error("Unable to get list of files.");
      return;
    }

    try{
      Arrays.sort(files);
      for (String fileName : files) {
        if (fileName.endsWith(".csv")) {
          processFile(fileName);
        }
      }
    }
    catch (Exception e) {
      log.error("Fatal error,EXITING! ", e);
      shutdown();
    }
  }

  private synchronized void processFile(String fileName) throws Exception {
    File f = new File(baseDir, fileName);
    BufferedReader reader = null;
    try {
      Calendar c = Calendar.getInstance();
      c.setTimeZone(STAT_TIMEZONE);
      c.setTime(Functions.convertTime(new SimpleDateFormat(FILE_NAME_DATE_PATTERN).parse(fileName), STAT_TIMEZONE, LOCAL_TIMEZONE));
      reader = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f)));
      String line = reader.readLine(); // Skip header
      if (line == null)
        return;
      while ((line = reader.readLine()) != null) {
        processLine(fileName, c, line);
      }
    }
    catch (Exception e) {
      log.error("Error parsing delivery states log : " + fileName, e);
    }
    finally {
      if (reader != null) try {
        reader.close();
      } catch (Exception e) {
        log.error("", e);
      }
      try {
        fileSys.rename(f, new File(backupDir, fileName));
      }
      catch (Exception e) {
        log.error("Error moving file to backup dir, EXITING! ", e);
        throw e;
      }
    }

  }

  private static Map<String, String> convertUserData(String s) {
    if (s == null || (s = s.trim()).length() == 0) {
      return null;
    }
    String[] entries = s.split(";");
    Map<String, String> r = new HashMap<String, String>(entries.length);
    for (String e : entries) {
      String[] kv = e.split("=");
      r.put(kv[0], kv[1]);
    }
    return r;
  }

  private void processLine(String fileName, Calendar c, String line) {
    try {
      CSVTokenizer t = new CSVTokenizer(line);
      if (t.hasMoreTokens()) {
        ChangeDeliveryStatusEvent stateEventChange;
        int ss = Integer.valueOf(t.nextToken());
        c.set(Calendar.SECOND, ss);
        int deliveryId = Integer.valueOf(t.nextToken());
        String userId = t.nextToken();
        DeliveryNotificationType type = getTypeByInt(Integer.valueOf(t.nextToken()));
        if (type == DeliveryNotificationType.MESSAGE_FINISHED) {
          //MSG_ID, STATUS, SMPP_STATUS, ADDRESS, USER_DATA
          long msgId = Long.valueOf(t.nextToken());
          MessageState messageState = getMessageState(t.nextToken());
          int smpp_status = Integer.valueOf(t.nextToken());
          Address addr = new Address(t.nextToken());
          String userData = null;
          if (t.hasMoreTokens()) userData = t.nextToken();
          Properties props = new Properties();
          if (userData != null)
            props.putAll(convertUserData(userData));

          notifyListenersOnMessage(new ChangeMessageStateEvent(c.getTime(), deliveryId, userId,
              msgId, messageState, smpp_status, addr, props));
        }
        else {
          DeliveryStatus state = type == DeliveryNotificationType.DELIVERY_START ? DeliveryStatus.Active : DeliveryStatus.Finished;
          stateEventChange = new ChangeDeliveryStatusEvent(state, c.getTime(), deliveryId, userId);
          notifyListeners(stateEventChange);
        }
      }
    }
    catch (Exception e) {
      log.error("Error processing log " + fileName + " line : " + line, e);
    }
  }

  private void notifyListenersOnMessage(ChangeMessageStateEvent stateEvent) {
    for (DeliveryChangeListener listener : listeners) {
      try {
        listener.messageStateChanged(stateEvent);
      }
      catch (Throwable e) {
        log.error("error in listener", e);
      }
    }
  }

  private void notifyListeners(ChangeDeliveryStatusEvent stateEventChange) {
    for (DeliveryChangeListener listener : listeners) {
      try {
        listener.deliveryStateChanged(stateEventChange);
      }
      catch (Throwable e) {
        log.error("error in listener", e);
      }
    }
  }

  private MessageState getMessageState(String s) {
    s = s.trim();
    if (s.length() == 1) {
      switch (s.charAt(0)) {
        case 'E':
          return MessageState.Expired;
        case 'D':
          return MessageState.Delivered;
        case 'F':
          return MessageState.Failed;
        default:
          break;
      }
    }
    throw new IllegalArgumentException("invalid message event state ='" + s + "'");
  }

  private DeliveryNotificationType getTypeByInt(int value) {
    switch (value) {
      case 0:
        return DeliveryNotificationType.MESSAGE_FINISHED;
      case 1:
        return DeliveryNotificationType.DELIVERY_START;
      case 2:
        return DeliveryNotificationType.DELIVERY_FINISHED;
      default:
        break;
    }
    throw new IllegalArgumentException("Invalid event type = " + value);
  }


  public List<DateAndFile> getProcessedNotificationsFiles(Date startDate,Date endDate) throws AdminException {
    List<DateAndFile> ret = new ArrayList<DateAndFile>();
    File[] files = fileSys.listFiles(backupDir);
    if(files==null) return ret;
    for (File f : files ) {
      try {
        Calendar c = Calendar.getInstance();
        c.setTime(new SimpleDateFormat(FILE_NAME_DATE_PATTERN).parse(f.getName()));
        if(startDate!=null && c.getTime().before(startDate)) continue;
        if(endDate!=null && endDate.before(c.getTime()))     continue;
        ret.add(new DateAndFile(c,f));
      }
      catch (Exception e) {
        log.error("Error parsing delivery processed file: " + f.getAbsolutePath(), e);
      }      
    }
    return ret;
  }

}