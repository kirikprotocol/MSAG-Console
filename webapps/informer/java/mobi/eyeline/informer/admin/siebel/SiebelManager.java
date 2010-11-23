package mobi.eyeline.informer.admin.siebel;

import com.eyeline.utils.ThreadFactoryWithCounter;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDataProviderImpl;
import mobi.eyeline.informer.admin.siebel.impl.SiebelDeliveries;
import mobi.eyeline.informer.admin.siebel.impl.SiebelRegionManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import org.apache.log4j.Logger;

import java.util.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelManager {

  private static final Logger logger = Logger.getLogger(SiebelManager.class);

  private SiebelDataProvider provider;

  private SiebelRegionManager regionManager;

  private boolean shutdown = true;

  private long timeout = 20;

  private final Set<String> processedDeliveries = new HashSet<String>();

  private final Lock lock = new ReentrantLock();

  protected SiebelDeliveries deliveries;

  private User siebelUser;

  private Thread listenerThread;

  private ExecutorService executor;

  private boolean removeOnStop = false;


  public static final String TIMEOUT = "timeout";
  public static final String USER = "siebelUser";
  public static final String REMOVE_ON_STOP_PARAM = "removeOnStop";
  public static final String JDBC_SOURCE = "jdbc.source";
  public static final String JDBC_USER = "jdbc.user";
  public static final String JDBC_PASSWORD = "jdbc.password";
  public static final String JDBC_DRIVER = "jdbc.driver";

  public SiebelManager(SiebelDeliveries deliveries, SiebelRegionManager regionManager) throws AdminException {
    this.provider = new SiebelDataProviderImpl();
    this.deliveries = deliveries;
    this.regionManager = regionManager;
    this.provider = new SiebelDataProviderImpl();
  }

  private final Lock shutdownLock = new ReentrantLock();

  private void lockShutdown() {
    shutdownLock.lock();
  }

  private void unlockShutdown() {
    shutdownLock.unlock();
  }


  public synchronized void stop() {
    shutdown = true;
    if (listenerThread != null) {
      listenerThread.interrupt();
    }
    if (executor != null) {
      executor.shutdown();
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: delivery manager has shutdowned");
      }
    }
    try {
      lockShutdown();
    } finally {
      unlockShutdown();
    }
    if (provider != null) {
      provider.shutdown();
    }
  }

  public boolean isStarted() {
    return !shutdown;
  }

  public void setMessageStates(Map<String, SiebelMessage.DeliveryState> deliveryStates) throws AdminException {
    if (isStarted()) {
      provider.setMessageStates(deliveryStates);
    } else {
      throw new SiebelException("offline");
    }
  }

  public void setDeliveryStatuses(Map<String, SiebelDelivery.Status> statuses) throws AdminException {
    provider.setDeliveryStatuses(statuses);
  }

  public synchronized void start(User siebelUser, Properties ps) throws AdminException {
    if (shutdown) {
      this.siebelUser = siebelUser;
      timeout = Integer.parseInt(ps.getProperty(TIMEOUT));
      removeOnStop = Boolean.valueOf(ps.getProperty(REMOVE_ON_STOP_PARAM));
      provider.connect(ps);
      executor = new ThreadPoolExecutor(5, Integer.MAX_VALUE, 60, TimeUnit.SECONDS,   //todo?
          new LinkedBlockingQueue<Runnable>(), new ThreadFactoryWithCounter("Siebel-Delivery-Processor", 0));
      listenerThread = new Thread(new ProviderListener(), "SiebelProviderListener");
      listenerThread.start();
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: delivery manager has started");
      }
    }
  }

  private boolean isInProcessing(SiebelDelivery st) {
    synchronized (lock) {
      return processedDeliveries.contains(st.getWaveId());
    }
  }

  private void setInProcessing(SiebelDelivery st) {
    try {
      lock.lock();
      processedDeliveries.add(st.getWaveId());
    } finally {
      lock.unlock();
    }
  }

  private void setProcessed(SiebelDelivery st) {
    try {
      lock.lock();
      processedDeliveries.remove(st.getWaveId());
    } finally {
      lock.unlock();
    }
  }

  private class ProviderListener implements Runnable {

    public void run() {
      try {
        lockShutdown();
        shutdown = false;
        while (!shutdown) {
          ResultSet<SiebelDelivery> rs = null;
          try {
            rs = provider.getDeliveriesToUpdate();
            while (rs.next()) {
              final SiebelDelivery st = rs.get();

              if (logger.isDebugEnabled())
                logger.debug("Siebel: found delivery to update: " + st.getWaveId());

              if (!isInProcessing(st)) {
                process(st);
              } else {
                if (logger.isDebugEnabled())
                  logger.debug("Siebel: delivery: " + st.getWaveId() + " already in processing...");
              }
            }
          } catch (Exception e) {
            logger.error(e, e);
          } finally {
            if (rs != null) {
              rs.close();
            }
          }
          try {
            Thread.sleep(timeout * 1000);
          } catch (InterruptedException ignored) {
            return;
          }
        }

        if (logger.isDebugEnabled()) {
          logger.debug("Siebel: delivery manager stopped");
        }

      } catch (Exception e) {
        logger.error(e, e);
      } finally {
        unlockShutdown();
      }
    }
  }

  private void _setDeliveryStatusInProcess(SiebelDelivery st) throws AdminException {
    if (provider.getDeliveryStatus(st.getWaveId()) == SiebelDelivery.Status.ENQUEUED) {
      provider.setDeliveryStatus(st.getWaveId(), SiebelDelivery.Status.IN_PROCESS);
      if (logger.isDebugEnabled())
        logger.debug("Siebel: set delivery status to " + SiebelDelivery.Status.IN_PROCESS);
    }
  }

  private void _setDeliveryStatusInProcessed(SiebelDelivery st) throws AdminException {
    if (provider.getDeliveryStatus(st.getWaveId()) == SiebelDelivery.Status.ENQUEUED) {
      provider.setDeliveryStatus(st.getWaveId(), SiebelDelivery.Status.PROCESSED);
      if (logger.isDebugEnabled())
        logger.debug("Siebel: set tasK status to " + SiebelDelivery.Status.PROCESSED);
    }

  }

  private void beginDelivery(SiebelDelivery st, DeliveryInfo info) throws AdminException {

    String generationFlag = "message_generation_in_process";

    Delivery delivery = info == null ? null : deliveries.getDelivery(siebelUser.getLogin(), siebelUser.getPassword(), info.getDeliveryId());

    if (delivery != null) {
      if (info.getProperty("message_generation_in_process") == null) {
        if (logger.isDebugEnabled()) {
          logger.debug("Siebel: delivery already exists and has generated, waveId='" + st.getWaveId() + "'. Enable it");
        }
        deliveries.activateDelivery(siebelUser.getLogin(), siebelUser.getPassword(), delivery.getId());
        _setDeliveryStatusInProcess(st);
        return;
      } else {
        if (logger.isDebugEnabled()) {
          logger.debug("Siebel: delivery is damaged. Recreate it. WaveId=" + st.getWaveId());
        }
        deliveries.dropDelivery(siebelUser.getLogin(), siebelUser.getPassword(), delivery.getId());
      }
    }

    delivery = createDelivery(st, buildDeliveryName(st.getWaveId()));

    deliveries.createDelivery(siebelUser.getLogin(), siebelUser.getPassword(), delivery, null);
    delivery.setProperty(generationFlag, "true");
    deliveries.modifyDelivery(siebelUser.getLogin(), siebelUser.getPassword(), delivery);

    ResultSet<SiebelMessage> messages = null;
    boolean hasMessages;
    try {
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: delivery is new or doesn't contain some parts. Generate it. WaveId=" + st.getWaveId());
      }
      messages = provider.getMessages(st.getWaveId());
      hasMessages = addMessages(delivery, messages) != 0;

      delivery.removeProperty("message_generation_in_process");
      deliveries.modifyDelivery(siebelUser.getLogin(), siebelUser.getPassword(), delivery);

      deliveries.activateDelivery(siebelUser.getLogin(), siebelUser.getPassword(), delivery.getId());
    } finally {
      if (messages != null) {
        messages.close();
      }
    }
    if (hasMessages) {
      _setDeliveryStatusInProcess(st);
    } else {
      _setDeliveryStatusInProcessed(st);
    }
  }

  private void stopDelivery(DeliveryInfo delivery, boolean remove) throws AdminException {
    try {
      if (delivery != null) {
        if (remove) {
          deliveries.dropDelivery(siebelUser.getLogin(), siebelUser.getPassword(), delivery.getDeliveryId());
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: remove delivery " + delivery.getName());
          }
        } else {
          deliveries.cancelDelivery(siebelUser.getLogin(), siebelUser.getPassword(), delivery.getDeliveryId());
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: stop tasK " + delivery.getName());
          }
        }
      }
    } catch (AdminException e) {
      throw e;
    } catch (Exception e) {
      logger.error(e, e);
      throw new SiebelException("internal_error");
    }
  }


  private void pauseDelivery(DeliveryInfo delivery) throws AdminException {
    try {
      deliveries.pauseDelivery(siebelUser.getLogin(), siebelUser.getPassword(), delivery.getDeliveryId());
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: pause delivery " + delivery.getName());
      }
    } catch (AdminException e) {
      throw e;
    } catch (Exception e) {
      throw new SiebelException("internal_error");
    }
  }

  private void process(final SiebelDelivery st) throws AdminException {
    try {

      final DeliveryInfo deliveryInfo = getExistingDelivery(st.getWaveId());

      final Runnable thread;

      if (st.getStatus() == SiebelDelivery.Status.ENQUEUED) {
        thread = new Runnable() {
          public void run() {
            try {
              if (logger.isDebugEnabled()) {
                logger.debug("Siebel: Found delivery in state=" + SiebelDelivery.Status.ENQUEUED + " WaveId=" + st.getWaveId());
              }
              beginDelivery(st, deliveryInfo);
            } catch (Exception e) {
              logger.error(e, e);
            } finally {
              setProcessed(st);
            }
          }
        };
      } else if (st.getStatus() == SiebelDelivery.Status.STOPPED) {
        if (deliveryInfo == null) {
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: Found delivery in state=" + SiebelDelivery.Status.STOPPED +
                ". It doesn't exist.  WaveId=" + st.getWaveId());
          }
          return;
        }
        thread = new Runnable() {
          public void run() {
            try {
              if (logger.isDebugEnabled()) {
                logger.debug("Siebel: Found delivery in state=" + SiebelDelivery.Status.STOPPED + " WaveId=" + st.getWaveId());
              }
              stopDelivery(deliveryInfo, removeOnStop);
            } catch (Exception e) {
              logger.error(e, e);
            } finally {
              setProcessed(st);
            }
          }
        };
      } else if (st.getStatus() == SiebelDelivery.Status.PAUSED) {
        if (deliveryInfo == null) {
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: Found delivery in state=" + SiebelDelivery.Status.PAUSED +
                ". It doesn't exist. WaveId=" + st.getWaveId());
          }
          return;
        }
        thread = new Runnable() {
          public void run() {
            try {
              pauseDelivery(deliveryInfo);
            } catch (Exception e) {
              logger.error(e, e);
            } finally {
              setProcessed(st);
            }
          }
        };
      } else {
        throw new SiebelException("Siebel: Illegal delivery's status: " + st.getStatus());
      }

      setInProcessing(st);
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: start to process delivery " + st);
      }
      executor.execute(thread);
    } catch (Exception e) {
      logger.error(e, e);
      throw new SiebelException("internal_error");
    }
  }


  private static String convertMsisdn(String msisdn) {
    msisdn = msisdn.trim();
    if (msisdn.charAt(0) != '+') {
      if (msisdn.charAt(0) == '8') {
        msisdn = "+7" + msisdn.substring(1);
      } else {
        msisdn = '+' + msisdn;
      }
    }
    return msisdn;
  }

  private int addMessages(Delivery delivery, final ResultSet<SiebelMessage> messages) throws AdminException {
    try {

      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: starting delivery generation...");
      }
      final int[] countMessages = new int[]{0};

      final Collection<String> unloaded = new ArrayList<String>(10001);

      deliveries.addMessages(siebelUser.getLogin(), siebelUser.getPassword(), new DataSource<Message>() {
        public Message next() throws AdminException {
          Message msg;
          while (messages.next()) {
            SiebelMessage sM = messages.get();
            String msisdn = sM.getMsisdn();
            if (msisdn != null && Address.validate(msisdn = convertMsisdn(msisdn))) {
              Address abonent = new Address(msisdn);
              Region r;
              if (siebelUser.isAllRegionsAllowed() || ((r = regionManager.getRegion(abonent)) != null
                  && siebelUser.getRegions() != null && siebelUser.getRegions().contains(r.getRegionId()))) {
                msg = Message.newMessage(abonent, sM.getMessage());
                msg.setProperty(UserDataConsts.SIEBEL_MESSAGE_ID, sM.getClcId());
                countMessages[0]++;
                return msg;
              } else {
                unloaded.add(sM.getClcId());
              }
            } else {
              unloaded.add(sM.getClcId());
            }
            if (unloaded.size() == 10000) {
              updateUnloaded(unloaded);
              unloaded.clear();
            }
          }
          return null;
        }
      }, delivery.getId());

      if (!unloaded.isEmpty()) {
        updateUnloaded(unloaded);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: delivery generation ok: " + countMessages[0] + " messages");
      }
      return countMessages[0];
    } catch (AdminException e) {
      logger.error(e, e);
      if (logger.isDebugEnabled()) {
        logger.debug("Error during adding messages. Drop delivery: " + delivery.getName());
      }
      try {
        deliveries.dropDelivery(siebelUser.getLogin(), siebelUser.getPassword(), delivery.getId());
      } catch (Exception ex) {
        logger.error(ex, ex);
      }
      throw e;
    }
  }

  private void updateUnloaded(Collection<String> unloaded) {
    try {
      Map<String, SiebelMessage.DeliveryState> states = new HashMap<String, SiebelMessage.DeliveryState>(unloaded.size());
      for (String anUnloaded : unloaded) {
        logger.error(new StringBuilder(60).append("Siebel: Unloaded message for '").append(anUnloaded).append('\'').toString());
        states.put(anUnloaded, new SiebelMessage.DeliveryState(SiebelMessage.State.REJECTED, "11", "Invalid Dest Addr"));
      }
      provider.setMessageStates(states);
    } catch (Exception e) {
      logger.error(e, e);
    }
  }

  private Delivery createDelivery(SiebelDelivery siebelDelivery, String deliveryName) throws AdminException {
    Delivery delivery = Delivery.newCommonDelivery();
    deliveries.getDefaultDelivery(siebelUser.getLogin(), delivery);
    delivery.setName(deliveryName);
    //todo beep, save

    delivery.setPriority(siebelDelivery.getPriority());
    delivery.setFlash(siebelDelivery.isFlash());
//      delivery.setReplaceMessage(cfg.isSiebelTReplaceMessage()); todo
    if (siebelDelivery.getExpPeriod() != null && siebelDelivery.getExpPeriod() != 0) {
      delivery.setValidityPeriod(new Time(siebelDelivery.getExpPeriod(), 0 ,0));
    } else {
      delivery.setValidityPeriod(new Time(1,0,0));
    }

    delivery.setStartDate(new Date());
    delivery.setProperty(UserDataConsts.SIEBEL_DELIVERY_ID, siebelDelivery.getWaveId());
    return delivery;
  }

  private static String buildDeliveryName(String waveId) {
    return new StringBuffer(7 + waveId.length()).append("siebel_").append(waveId).toString();
  }

  private DeliveryInfo getExistingDelivery(final String waveId) throws AdminException {
    DeliveryFilter filter = new DeliveryFilter();
    final DeliveryInfo[] infos = new DeliveryInfo[1];
    deliveries.getDeliveries(siebelUser.getLogin(), siebelUser.getPassword(), filter, 1000,
        new Visitor<mobi.eyeline.informer.admin.delivery.DeliveryInfo>() {
          public boolean visit(DeliveryInfo value) throws AdminException {
            if (waveId.equals(value.getProperty(UserDataConsts.SIEBEL_DELIVERY_ID))) {
              infos[0] = value;
              return false;
            }
            return true;
          }
        });
    return infos[0];
  }


}