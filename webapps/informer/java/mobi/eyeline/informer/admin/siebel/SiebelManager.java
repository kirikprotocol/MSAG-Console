package mobi.eyeline.informer.admin.siebel;

import com.eyeline.utils.ThreadFactoryWithCounter;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.regions.Region;
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
class SiebelManager {

  private static final Logger logger = Logger.getLogger("SIEBEL");

  private SiebelDataProvider provider;

  private boolean shutdown = true;

  private long timeout = 20;

  private final Set<String> processedDeliveries = new HashSet<String>();

  private final Lock lock = new ReentrantLock();

  protected SiebelContext context;

  private String siebelUser;

  private Thread listenerThread;

  private ExecutorService executor;

  private boolean removeOnStop = false;

  private Date lastUpdate;

  public SiebelManager(SiebelContext context) throws AdminException {
    this.provider = new SiebelDataProviderImpl();
    this.context = context;
  }

  SiebelManager(SiebelDataProvider provider, SiebelContext context) throws AdminException {
    this.provider = provider;
    this.context = context;
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
    if (executor != null) {
      executor.shutdown();
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: delivery manager has shutdowned");
      }
    }
    if (listenerThread != null) {
      listenerThread.interrupt();
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
    if (provider != null && !provider.isShutdowned()) {
      provider.setMessageStates(deliveryStates);
    } else {
      throw new SiebelException("offline");
    }
  }

  public void setDeliveryStatuses(Map<String, SiebelDelivery.Status> statuses) throws AdminException {
    if (provider != null && !provider.isShutdowned()) {
      provider.setDeliveryStatuses(statuses);
    } else {
      throw new SiebelException("offline");
    }
  }

  public void checkProperties(SiebelSettings p) throws AdminException {
    provider.check(p.getAllProperties());
  }

  private Properties siebelProperties;

  public synchronized void start(String siebelUser, SiebelSettings ps) throws AdminException {
    if (shutdown) {
      lastUpdate = new Date(0);
      this.siebelUser = siebelUser;
      timeout = ps.getTimeout();
      removeOnStop = ps.isRemoveOnStop();
      siebelProperties = new Properties();
      siebelProperties.putAll(ps.getAllProperties());
      executor = new ThreadPoolExecutor(5, Integer.MAX_VALUE, 60, TimeUnit.SECONDS,
          new LinkedBlockingQueue<Runnable>(), new ThreadFactoryWithCounter("Siebel-Delivery-Processor", 0));
      listenerThread = new ProviderListener("SiebelProviderListener");
      listenerThread.start();
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: delivery manager has started");
      }
    }
  }

  private boolean isInProcessing(SiebelDelivery st) {
    try {
      lock.lock();
      return processedDeliveries.contains(st.getWaveId());
    } finally {
      lock.unlock();
    }
  }
  private boolean isInProcessing(String waveId) {
    try {
      lock.lock();
      return processedDeliveries.contains(waveId);
    } finally {
      lock.unlock();
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

  private Collection<String> errors = new LinkedList<String>();

  private final Lock errorLock = new ReentrantLock();


  private List<String> getErrors() {
    try{
      errorLock.lock();
      List<String> res = new ArrayList<String>(errors);
      errors.clear();
      return res;
    }finally {
      errorLock.unlock();
    }
  }

  private void addError(String waveId) {
    try{
      errorLock.lock();
      errors.add(waveId);
    }finally {
      errorLock.unlock();
    }
  }

  private class ProviderListener extends Thread{

    private ProviderListener(String name) {
      super(name);
    }

    public void run() {
      try {
        lockShutdown();
        shutdown = false;
        while (!shutdown) {
          boolean connectionError = false;
          if(provider.isShutdowned()) {
            try{
              provider.connect(siebelProperties);
            }catch (Exception e){
              logger.error(e, e);
              connectionError = true;
            }
          }

          if(!connectionError) {

            ResultSet<SiebelDelivery> rs = null;
            try {
              rs = provider.getDeliveriesToUpdate(lastUpdate);
              while (rs.next()) {
                try{
                  final SiebelDelivery st = rs.get();

                  if (logger.isDebugEnabled())
                    logger.debug("Siebel: found delivery to update: waveId=" + st.getWaveId());

                  if (!isInProcessing(st)) {
                    process(st);
                    if(st.getLastUpdate().after(lastUpdate)) {
                      lastUpdate = new Date(st.getLastUpdate().getTime());
                    }
                  }else{
                    if (logger.isDebugEnabled())
                      logger.debug("Siebel: delivery (waveId=" + st.getWaveId() + ") is already in processing...");
                  }
                }catch (Exception e){
                  logger.error(e,e);
                }
              }

            } catch (Exception e) {
              logger.error(e, e);
            } finally {
              if (rs != null) {
                rs.close();
              }
            }

            for(String e : getErrors()) {
              if(!isInProcessing(e)) {
                if(logger.isDebugEnabled()) {
                  logger.debug("Try to repair operation with wave: waveId="+e);
                }
                try{
                  final SiebelDelivery st = provider.getDelivery(e);
                  if(st != null) {
                    process(st);
                  }else {
                    if(logger.isDebugEnabled()) {
                      logger.debug("Wave hasn't been found: waveId="+e);
                    }
                  }
                }catch (Exception ex){
                  logger.error(ex,ex);
                  addError(e);
                }
              }else {
                addError(e);
              }
            }

          }
          try {
            Thread.sleep(timeout * 1000);
          } catch (InterruptedException ignored) {
            return;
          }
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
        logger.debug("Siebel: set wave status to " + SiebelDelivery.Status.IN_PROCESS);
    }
  }

  private void _setDeliveryStatusInProcessed(SiebelDelivery st) throws AdminException {
    if (provider.getDeliveryStatus(st.getWaveId()) == SiebelDelivery.Status.ENQUEUED) {
      provider.setDeliveryStatus(st.getWaveId(), SiebelDelivery.Status.PROCESSED);
      if (logger.isDebugEnabled())
        logger.debug("Siebel: set wave status to " + SiebelDelivery.Status.PROCESSED);
    }

  }

  private void beginDelivery(SiebelDelivery st) throws AdminException {

    if (logger.isDebugEnabled()) {
      logger.debug("Siebel: Found wave in state=" + SiebelDelivery.Status.ENQUEUED +
          "; waveId=" + st.getWaveId());
    }

    final Delivery info = getExistingDelivery(st.getWaveId());

    String generationFlag = "message_generation_in_process";

    Delivery delivery = info == null ? null : context.getDelivery(siebelUser, info.getId());

    if (delivery != null) {
      if (info.getProperty(generationFlag) == null) {
        if (logger.isDebugEnabled()) {
          logger.debug("Siebel: delivery already exists and has generated, waveId='" + st.getWaveId() + "'. Activate it");
        }
        context.activateDelivery(siebelUser, delivery.getId());
        _setDeliveryStatusInProcess(st);
        return;
      } else {
        if (logger.isDebugEnabled()) {
          logger.debug("Siebel: delivery is damaged. Recreate it. WaveId=" + st.getWaveId());
        }
        context.dropDelivery(siebelUser, delivery.getId());
      }
    }

    DeliveryPrototype proto = createDelivery(st, buildDeliveryName(st.getWaveId()));

    proto.setProperty(generationFlag, "true");

    delivery = context.createDeliveryWithIndividualTexts(siebelUser, proto, null);

    ResultSet<SiebelMessage> messages = null;
    boolean hasMessages;
    try {
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: Create delivery: waveId=" + st.getWaveId());
      }
      messages = provider.getMessages(st.getWaveId());
      hasMessages = addMessages(delivery, messages) != 0;

      delivery.removeProperty(generationFlag);
      context.modifyDelivery(siebelUser, delivery);

      context.activateDelivery(siebelUser, delivery.getId());
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: Delivery has been created: waveId=" + st.getWaveId()+" informerId="+delivery.getId()+" name="+delivery.getName());
      }
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

  private void stopDelivery(SiebelDelivery st, boolean remove) throws AdminException {
    if (logger.isDebugEnabled()) {
      logger.debug("Siebel: Found wave in state=" + SiebelDelivery.Status.STOPPED +
          "; waveId=" + st.getWaveId());
    }

    final Delivery delivery = getExistingDelivery(st.getWaveId());

    if (delivery == null) {
      if (logger.isDebugEnabled()) {
        logger.warn("Siebel: delivery found isn't exist: waveId=" + st.getWaveId());
      }
      return;
    }

    try {
      if (remove) {
        context.dropDelivery(siebelUser, delivery.getId());
        if (logger.isDebugEnabled()) {
          logger.debug("Siebel: delivery has been removed name=" + delivery.getName()+" waveId="+st.getWaveId()+" informerId="+delivery.getId());
        }
      } else {
        context.cancelDelivery(siebelUser, delivery.getId());
        if (logger.isDebugEnabled()) {
          logger.debug("Siebel: delivery has been canceled name=" + delivery.getName()+" waveId="+st.getWaveId()+" informerId="+delivery.getId());
        }
      }
    } catch (AdminException e) {
      throw e;
    } catch (Exception e) {
      logger.error(e, e);
      throw new SiebelException("internal_error");
    }
  }


  private void pauseDelivery(SiebelDelivery st) throws AdminException {

    if (logger.isDebugEnabled()) {
      logger.debug("Siebel: Found wave in state=" + SiebelDelivery.Status.PAUSED +
          "; waveId=" + st.getWaveId());
    }

    final Delivery delivery = getExistingDelivery(st.getWaveId());

    if (delivery == null) {
      if (logger.isDebugEnabled()) {
        logger.warn("Siebel: delivery found isn't exist: waveId=" + st.getWaveId());
      }
      return;
    }

    try {
      context.pauseDelivery(siebelUser, delivery.getId());
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: delivery has been paused=" + delivery.getName()+" waveId="+st.getWaveId()+" informerId="+delivery.getId());
      }
    } catch (AdminException e) {
      throw e;
    } catch (Exception e) {
      throw new SiebelException("internal_error");
    }
  }

  private boolean isCreationAvailable() throws AdminException{
    User u = context.getUser(siebelUser);
    if(u == null || u.getStatus() == User.Status.DISABLED) {
      if(logger.isDebugEnabled()) {
        logger.debug("Creation is not available. Siebel user is blocked!");
      }
      return false;
    }
    try {
      context.checkNoRestrictions(siebelUser);
    } catch (AdminException e) {
      if(logger.isDebugEnabled()) {
        logger.debug(e.getLocalizedMessage());
      }
      return false;
    }
    return true;
  }

  private void process(final SiebelDelivery st) throws AdminException {
    try {

      final Runnable thread;

      if (st.getStatus() == SiebelDelivery.Status.ENQUEUED) {
        if(!isCreationAvailable()) {
              if(st.getWaveId() != null) {
                addError(st.getWaveId());
              }
          return;
        }
        thread = new Runnable() {
          public void run() {
            try {
              beginDelivery(st);
            } catch (Exception e) {
              logger.error(e, e);
              if(st.getWaveId() != null) {
                addError(st.getWaveId());
              }
            } finally {
              setProcessed(st);
            }
          }
        };
      } else if (st.getStatus() == SiebelDelivery.Status.STOPPED) {
        thread = new Runnable() {
          public void run() {
            try {
              stopDelivery(st, removeOnStop);
            } catch (Exception e) {
              logger.error(e, e);
              if(st.getWaveId() != null) {
                addError(st.getWaveId());
              }
            } finally {
              setProcessed(st);
            }
          }
        };
      } else if (st.getStatus() == SiebelDelivery.Status.PAUSED) {
        thread = new Runnable() {
          public void run() {
            try {
              pauseDelivery(st);
            } catch (Exception e) {
              logger.error(e, e);
              if(st.getWaveId() != null) {
                addError(st.getWaveId());
              }
            } finally {
              setProcessed(st);
            }
          }
        };
      } else {
        throw new SiebelException("proc_error", "Siebel: Illegal delivery's status: " + st.getStatus());
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

      final User u = context.getUser(siebelUser);

      context.addMessages(siebelUser, new DataSource<Message>() {
        public Message next() throws AdminException {
          Message msg;
          while (messages.next()) {
            SiebelMessage sM = messages.get();
            String msisdn = sM.getMsisdn();
            if (msisdn != null && Address.validate(msisdn = convertMsisdn(msisdn))) {
              Address abonent = new Address(msisdn);
              Region r;
              if (u.isAllRegionsAllowed() || ((r = context.getRegion(abonent)) != null
                  && u.getRegions() != null && u.getRegions().contains(r.getRegionId()))) {
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
        context.dropDelivery(siebelUser, delivery.getId());
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

  private DeliveryPrototype createDelivery(SiebelDelivery siebelDelivery, String deliveryName) throws AdminException {
    DeliveryPrototype delivery = new DeliveryPrototype();
    context.copyUserSettingsToDeliveryPrototype(siebelUser, delivery);
    delivery.setEnableMsgFinalizationLogging(true);
    delivery.setEnableStateChangeLogging(true);
    delivery.setName(deliveryName);
    delivery.setArchiveTime(null);

    delivery.setPriority(siebelDelivery.getPriority());
    delivery.setFlash(siebelDelivery.isFlash());
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

  private Delivery getExistingDelivery(final String waveId) throws AdminException {
    DeliveryFilter filter = new DeliveryFilter();
    final Delivery[] infos = new Delivery[1];
    context.getDeliveries(siebelUser, filter,
        new Visitor<mobi.eyeline.informer.admin.delivery.Delivery>() {
          public boolean visit(Delivery value) throws AdminException {
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