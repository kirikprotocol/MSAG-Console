package mobi.eyeline.informer.admin.siebel;

import com.eyeline.utils.FileUtils;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeMessageStateEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListenerStub;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.monitoring.MBean;
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
class SiebelFinalStateListener extends DeliveryChangeListenerStub {

  private static final Logger logger = Logger.getLogger("SIEBEL");

  private final SiebelManager siebelManager;

  private final SiebelContext context;

  private Thread processor;

  private final ResourceBundle smppStatuses;

  private final File dir;

  private PrintWriter writer;

  private File curentFile;

  private static final SimpleDateFormat df = new SimpleDateFormat("ddMMyyyy_HHmmss");

  private int periodSec;

  private boolean stop = true;

  private FileSystem fs;

  private static final String DELIVERIES_CHANGES_ERROR = "Delivery's changes processing";
  private static final String MESSAGES_CHANGES_ERROR = "Message's changes processing";
  private static final String FILE_PROCCESS_ERROR = "Can't process internal files";

  public SiebelFinalStateListener(SiebelManager siebelManager, SiebelContext context, File workDir, int periodSec) throws InitException{
    this.dir = new File(workDir, "siebel");
    this.fs = context.getFileSystem();
    try {
      if(!fs.exists(dir)) {
        fs.mkdirs(dir);
      }
    } catch (AdminException e) {
      throw new InitException(e);
    }
    this.siebelManager = siebelManager;
    this.context = context;
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

  private void messageFinished(ChangeMessageStateEvent stateEvent) {
    if(stop) {
      logger.warn("Listener is stopped, can't process event");
      return;
    }
    if (stateEvent.getProperties() == null) {
      if(logger.isDebugEnabled()) {
        logger.debug("Event doesn't contains userData, skip it: "+stateEvent);
      }
      return;
    }
    switch (stateEvent.getMessageState()) {
      case New:
      case Process:
        logger.warn("Message state is "+stateEvent.getMessageState()+". Skip it: "+stateEvent);
        return;
    }

    Properties userData = stateEvent.getProperties();

    String clcId = userData.getProperty(UserDataConsts.SIEBEL_MESSAGE_ID);
    if (clcId == null) {
      if(logger.isDebugEnabled()) {
        logger.debug("Message is not a siebel message. Skip it : "+stateEvent);
      }
      return;
    }
    MessageState state = stateEvent.getMessageState();
    int errCode = stateEvent.getSmppStatus();
    try {
      writeLock();
      if(!stop) {
        String line = StringEncoderDecoder.toCSVString(',',0, clcId, state, errCode);
        writer.println(line);
        writer.flush();
      }else {
        logger.warn("Listener is stopped, can't process event");
      }
    } finally {
      writeUnlock();
    }
  }

  private void deliveryFinished(ChangeDeliveryStatusEvent stateEventChange) throws AdminException {
    if(stop) {
      logger.warn("Listener is stopped, can't process event");
      return;
    }
    User u = context.getUser(stateEventChange.getUserId());
    if (u == null) {
      logger.error("Can't find user with id: " + stateEventChange.getUserId());
      return;
    }
    Delivery d = context.getDelivery(u.getLogin(), stateEventChange.getDeliveryId());
    String waveId;
    if (d != null && (waveId = d.getProperty(UserDataConsts.SIEBEL_DELIVERY_ID)) != null) {
      if(logger.isDebugEnabled()) {
        logger.debug("Siebel delivery is finalized. WaveId="+waveId);
      }
      try {
        writeLock();
        if(!stop) {
          writer.println(StringEncoderDecoder.toCSVString(',',1, waveId));
          writer.flush();
        }
      } finally {
        writeUnlock();
      }
    }
  }

  private static MBean getMBean() {
    return MBean.getInstance(MBean.Source.SIEBEL);
  }

  public void messageStateChanged(ChangeMessageStateEvent e) throws AdminException {
    if (e.getMessageState() == MessageState.New || e.getMessageState() == MessageState.Process)
      return;
    try{
    messageFinished(e);
    }catch (RuntimeException ex){
      getMBean().notifyInternalError(DELIVERIES_CHANGES_ERROR, "Can't process changes for: deliveryId="+e.getDeliveryId()+" messageId="+e.getMessageId());
      throw ex;
    }
  }

  public void deliveryStateChanged(ChangeDeliveryStatusEvent e) throws AdminException {
    if (e.getStatus() != DeliveryStatus.Finished)
      return;
    try{
      deliveryFinished(e);
    }catch (AdminException ex){
      getMBean().notifyInternalError(MESSAGES_CHANGES_ERROR, "Can't process changes for: deliveryId="+e.getDeliveryId());
      throw ex;
    }
  }

  public synchronized void start() throws AdminException{
    if(stop) {
      curentFile = new File(dir, df.format(new Date())+".csv");
      writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(curentFile, false))));
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
      reader = new BufferedReader(new InputStreamReader(fs.getInputStream(f)));
      int countDeliveries = 0;
      int countMessages = 0;
      String line;
      while((line = reader.readLine()) != null) {
        List<String> list = StringEncoderDecoder.csvDecode(',',line);
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
              curentFile = new File(dir, df.format(new Date())+".csv");
              writer = new PrintWriter(new BufferedWriter(new OutputStreamWriter(fs.getOutputStream(curentFile, false))));
            }finally {
              writeUnlock();
            }

            File[] files = fs.listFiles(dir, new FileFilter() {
              public boolean accept(File pathname) {
                return !pathname.getName().equals(curentFile.getName());
              }
            });

            for (File toProccess : files) {
              if (toProccess.length() > 0) {
                try{
                  processFile(toProccess);
                }catch (Exception e){
                  getMBean().notifyInternalError(FILE_PROCCESS_ERROR, "Can't submit changes to Siebel from file: "+toProccess.getAbsolutePath());
                  break;
                }
              }
              fs.delete(toProccess);
            }
          }catch (Exception e){
            getMBean().notifyInternalError(FILE_PROCCESS_ERROR, "Unknown error");
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