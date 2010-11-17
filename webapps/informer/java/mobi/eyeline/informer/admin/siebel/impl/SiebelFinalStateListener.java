package mobi.eyeline.informer.admin.siebel.impl;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.siebel.SiebelDelivery;
import mobi.eyeline.informer.admin.siebel.SiebelManager;
import mobi.eyeline.informer.admin.siebel.SiebelMessage;
import mobi.eyeline.informer.admin.users.User;
import org.apache.log4j.Logger;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelFinalStateListener implements DeliveryNotificationsListener{

  private static final Logger logger = Logger.getLogger(SiebelFinalStateListener.class);

  private SiebelManager siebelManager;

  private SiebelDeliveries deliveries;

  private SiebelUserManager users;

  private ScheduledExecutorService commiter;

  private Map<String, SiebelMessage.DeliveryState> messageStatesCache = new HashMap<String, SiebelMessage.DeliveryState>(1001);      //todo?

  public SiebelFinalStateListener(SiebelManager siebelManager, SiebelDeliveries deliveries, SiebelUserManager users) {
    this.siebelManager = siebelManager;
    this.deliveries = deliveries;
    this.users = users;
    this.commiter = Executors.newSingleThreadScheduledExecutor();
    this.commiter.scheduleAtFixedRate(new Runnable() {
      public void run() {
        try{
          lock();
          commitFinalStates();
        }catch (Exception e){
          logger.error(e,e);
        }finally {
          unlock();
        }
      }
    }, 120, 120, TimeUnit.SECONDS); //todo?
  }

  private Lock lock = new ReentrantLock();

  public void lock() {
    lock.lock();
  }

  public void unlock() {
    lock.unlock();
  }

  private void commitFinalStates() throws AdminException{
    try{
      siebelManager.setMessageStates(messageStatesCache);
    }finally {
      messageStatesCache.clear();
    }
  }

  private void messageFinished(DeliveryMessageNotification notification) throws AdminException {
    if(notification.getUserData() == null) {
      return;
    }
    switch (notification.getMessageState()) {
      case New:
      case Process: return;
    }

    Map<String, String> userData = DcpConverter.convertUserData(notification.getUserData());

    String clcId = userData.get(UserDataConsts.SIEBEL_MESSAGE_ID);
    if(clcId == null){
      return;
    }
    MessageState state = notification.getMessageState();
    int code = notification.getSmppStatus();
    try{
      lock();
      messageStatesCache.put(clcId,
          new SiebelMessage.DeliveryState(stateToSiebelState(state), Integer.toString(code), null));  //todo description
      if(messageStatesCache.size() >= 1000) {
        commitFinalStates();
      }
    }finally {
      unlock();
    }
  }

  private void deliveryFinished(DeliveryNotification notification) throws AdminException{
    User u = users.getUser(notification.getUserId());
    if(u == null) {
      logger.error("Can't find user with id: "+notification.getUserId());
      return;
    }
    Delivery d = deliveries.getDelivery(u.getLogin(), u.getPassword(), notification.getDeliveryId());
    String waveId;
    if(d != null && (waveId = d.getProperty(UserDataConsts.SIEBEL_DELIVERY_ID)) != null) {
      siebelManager.setDeliveryStatus(waveId, SiebelDelivery.Status.PROCESSED);
    }
  }

  public void onDeliveryNotification(DeliveryNotification notification) {
    try{
      switch (notification.getType()) {
        case DELIVERY_FINISHED:
          deliveryFinished(notification);
          break;
        case MESSAGE_FINISHED:
          messageFinished((DeliveryMessageNotification)notification);
      }
    }catch (Exception e) {
      logger.error(e,e);
    }
  }

  public void shutdown() {
    if(commiter != null) {
      commiter.shutdown();
    }
  }

  private SiebelMessage.State stateToSiebelState(MessageState state) {
    SiebelMessage.State siebelState;
    if (state == MessageState.Delivered) {
      siebelState = SiebelMessage.State.DELIVERED;
    } else if (state == MessageState.Expired) {
      siebelState = SiebelMessage.State.EXPIRED;
    }  else if (state == MessageState.Failed) {
      siebelState = SiebelMessage.State.ERROR;
    } else {
      siebelState = SiebelMessage.State.UNKNOWN;
    }
    return siebelState;
  }


}