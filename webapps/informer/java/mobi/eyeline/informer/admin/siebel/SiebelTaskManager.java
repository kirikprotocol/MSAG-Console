package mobi.eyeline.informer.admin.siebel;

import com.eyeline.utils.ThreadFactoryWithCounter;
import com.sun.jmx.snmp.tasks.Task;
import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManager;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.util.*;
import java.util.concurrent.Executor;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.regex.Pattern;

/**
 * @author Aleksandr Khalitov
 */
public class SiebelTaskManager {

  private static final Logger logger = Logger.getLogger(SiebelTaskManager.class);

  private final static String TASK_OWNER = "siebel";

  private final SiebelDataProvider provider;

  private boolean shutdown = true;

  private long timeout = 20000;   //todo

  private Set<String> processedTasks = new HashSet<String>();

  private final Lock lock = new ReentrantLock();

  private AdminContext adminContext;

  private final UsersManager userManager;

  private Thread listenerThread;

  private Executor executor = new ThreadPoolExecutor(1,Integer.MAX_VALUE, 60, TimeUnit.SECONDS, new LinkedBlockingQueue<Runnable>(), new ThreadFactoryWithCounter("Siebel-Task-Processor",0));

  private boolean removeOnStop = false; //todo

  public SiebelTaskManager(SiebelDataProvider dataProvider, AdminContext context, UsersManager userManager) {
    this.provider = dataProvider;
    this.adminContext = context;
    this.userManager = userManager;
  }

  public void shutdown() {
    shutdown = true;
    if(listenerThread != null) {
      listenerThread.interrupt();
    }
  }

  public boolean isOnline() {
    return !shutdown;
  }

  public void start() {
    if (shutdown) {
      listenerThread = new Thread(new ProviderListener(), "SiebelProviderListener");
      listenerThread.start();
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: task manager started");
      }
    }
  }

  private boolean isInProcessing(SiebelDelivery st) {
    synchronized (lock) {
      return processedTasks.contains(st.getWaveId());
    }
  }

  private void setInProcessing(SiebelDelivery st) {
    try {
      lock.lock();
      processedTasks.add(st.getWaveId());
    }finally {
      lock.unlock();
    }
  }

  private void setProcessed(SiebelDelivery st) {
    try {
      lock.lock();
      processedTasks.remove(st.getWaveId());
    }finally {
      lock.unlock();
    }
  }

  private class ProviderListener implements Runnable{

    public void run() {
      try {
        shutdown = false;
        while (!shutdown) {
          ResultSet<SiebelDelivery> rs = null;
          try {
            rs = provider.getDeliveriesToUpdate();
            while (rs.next()) {
              final SiebelDelivery st = rs.get();

              if (logger.isDebugEnabled())
                logger.debug("Siebel: found task to update: " + st.getWaveId());

              if (!isInProcessing(st)) {
                process(st);
              } else {
                if (logger.isDebugEnabled())
                  logger.debug("Siebel: task: " + st.getWaveId() + " already in processing...");
              }
            }
          } catch (Throwable e) {
            logger.error(e, e);
          } finally {
            if (rs != null) {
              rs.close();
            }
          }
          try {
            Thread.sleep(timeout);
          } catch (InterruptedException ignored) {
          }
        }

        if (logger.isDebugEnabled()) {
          logger.debug("Siebel: task manager stopped");
        }

      } catch (Throwable e) {
        logger.error(e, e);
      }
    }
  }


  public void setTimeout(long timeout) throws AdminException{
    if(timeout <= 0) {
      throw new IllegalArgumentException("Timeout can't be less then or equal 0");
    }
    this.timeout = timeout;
  }

  public long getTimeout() {
    return timeout;
  }

  public boolean isRemoveOnStop() {
    return removeOnStop;
  }

  public void setRemoveOnStop(boolean removeOnStop) {
    this.removeOnStop = removeOnStop;
  }

  private void _setTaskStatusInProcess(SiebelDelivery st) throws SiebelException {
    if (provider.getDeliveryStatus(st.getWaveId()) == SiebelDelivery.Status.ENQUEUED) {
      provider.setDeliveryStatus(st.getWaveId(), SiebelDelivery.Status.IN_PROCESS);
      if (logger.isDebugEnabled())
        logger.debug("Siebel: set task status to " + SiebelDelivery.Status.IN_PROCESS);
    }
  }
  private void _setTaskStatusInProcessed(SiebelDelivery st) throws SiebelException {
    if (provider.getDeliveryStatus(st.getWaveId()) == SiebelDelivery.Status.ENQUEUED) {
      provider.setDeliveryStatus(st.getWaveId(), SiebelDelivery.Status.PROCESSED);
      if (logger.isDebugEnabled())
        logger.debug("Siebel: set tasK status to " + SiebelDelivery.Status.PROCESSED);
    }

  }

  private void beginTask(User u , SiebelDelivery st, DeliveryInfo info) throws SiebelException {
//    try {
//
//      Delivery delivery = info == null ? null : adminContext.getDelivery(u.getLogin(), u.getPassword(), info.getDeliveryId());
//
//      if (delivery != null) {
////        if (delivery.isMessagesHaveLoaded()) {
////          if (logger.isDebugEnabled()) {
////            logger.debug("Siebel: task already exists and has generated, waveId='" + st.getWaveId() + "'. Enable it");
////          }
////          _startTask(delivery);
////          _setTaskStatusInProcess(st);
////          return;
////        } else {
////          if (logger.isDebugEnabled()) {
////            logger.debug("Siebel: task is damaged. Remove crached parts. WaveId=" + st.getWaveId());
////          }
////          adminContext.dropDelivery(u.getLogin(), u.getPassword(), delivery.getId());
////        }
//      }
//
//      delivery = createDelivery(u, st, buildTaskName(st.getWaveId()));
//
////      InfoSmeConfig.validate(delivery.originTask);   todo?
//
//      ResultSet<SiebelMessage> messages = null;
//      boolean hasMessages;
//      try {
//        if (logger.isDebugEnabled()) {
//          logger.debug("Siebel: task is new or doesn't contain some parts. Generate it. WaveId=" + st.getWaveId());
//        }
//        messages = provider.getMessages(st.getWaveId());
////        hasMessages = addMessages(delivery, messages) != 0;
////        delivery.done();
//      } finally {
//        if (messages != null) {
//          messages.close();
//        }
//      }
//      if(hasMessages) {
//        _setTaskStatusInProcess(st);
//      }else {
//        _setTaskStatusInProcessed(st);
//      }
//    } catch (AdminException e) {
//      throw new SiebelException(e);
//    }
  }

  private void stopTask(User u, DeliveryInfo task, boolean remove) throws AdminException {
    try {
      if (task != null) {
        if (remove) {
          adminContext.dropDelivery(u.getLogin(), u.getPassword(), task.getDeliveryId());
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: remove task " + task.getName());
          }
        } else {
          adminContext.cancelDelivery(u.getLogin(), u.getPassword(), task.getDeliveryId());
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: stop tasK " + task.getName());
          }
        }
      }
    } catch (AdminException e){
      throw e;
    } catch (Throwable e) {
      logger.error(e,e);
      throw new SiebelException("internal_error");
    }
  }

  private void _removeTask(Task t) throws SiebelException {
//todo    try {
//      ctx.getInfoSmeConfig().removeAndApplyTask(TASK_OWNER, t.getId());
//      ctx.getInfoSme().removeTask(t.getId());
//    } catch (Throwable e) {
//      throw new SiebelException(e);
//    }
  }

// todo private void _startTask(MultiTask task) throws SiebelException {
//    try {
//      Iterator i = task.tasks.values().iterator();
//      while (i.hasNext()) {
//        TaskPart t = (TaskPart) i.next();
//        if (!t.task.isEnabled()) {
//          t.setEnabled(true);
//          t.changeTask();
//        }
//      }
//    } catch (Throwable e) {
//      throw new SiebelException(e);
//    }
//  }

  private void pauseTask(User u, DeliveryInfo task) throws AdminException {
    try {
      adminContext.pauseDelivery(u.getLogin(), u.getPassword(), task.getDeliveryId());
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: pause task " + task.getName());
      }
    } catch (AdminException e){
      throw e;
    } catch (Throwable e) {
      throw new SiebelException("internal_error");
    }
  }

  private void process(final SiebelDelivery st) throws SiebelException {
    try {

      final User u = userManager.getUser(TASK_OWNER);
      if(u == null) {
        logger.error("Siebel user is not found");
        throw new SiebelException("internal_error");
      }

      final DeliveryInfo deliveryInfo = getExistingDelivery(u, st.getWaveId());

      final Runnable thread;

      if (st.getStatus() == SiebelDelivery.Status.ENQUEUED) {
        thread = new Runnable() {
          public void run() {
            try {
              if (logger.isDebugEnabled()) {
                logger.debug("Siebel: Found task in state=" + SiebelDelivery.Status.ENQUEUED + " WaveId=" + st.getWaveId());
              }
              beginTask(u, st, deliveryInfo);
            } catch (Throwable e) {
              logger.error(e, e);
            } finally {
              setProcessed(st);
            }
          }
        };
      } else if (st.getStatus() == SiebelDelivery.Status.STOPPED) {
        if (deliveryInfo == null || (!removeOnStop && (deliveryInfo.getStatus() == DeliveryStatus.Cancelled
            || deliveryInfo.getStatus() == DeliveryStatus.Finished))) {
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: Found task in state=" + SiebelDelivery.Status.STOPPED +
                ". It doesn't exist or disabled or doesn't need to remove.  WaveId=" + st.getWaveId());
          }
          return;
        }
        thread = new Runnable() {
          public void run() {
            try {
              if (logger.isDebugEnabled()) {
                logger.debug("Siebel: Found task in state=" + SiebelDelivery.Status.STOPPED + " WaveId=" + st.getWaveId());
              }
              stopTask(u, deliveryInfo, removeOnStop);
            } catch (Throwable e) {
              logger.error(e, e);
            } finally {
              setProcessed(st);
            }
          }
        };
      } else if (st.getStatus() == SiebelDelivery.Status.PAUSED) {
        if (deliveryInfo == null || deliveryInfo.getStatus() == DeliveryStatus.Paused || deliveryInfo.getStatus() == DeliveryStatus.Finished || deliveryInfo.getStatus() == DeliveryStatus.Cancelled) {
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: Found task in state=" + SiebelDelivery.Status.PAUSED +
                ". It doesn't exist or disabled.  WaveId=" + st.getWaveId());
          }
          return;
        }
        thread = new Runnable() {
          public void run() {
            try {
              pauseTask(u, deliveryInfo);
            } catch (Throwable e) {
              logger.error(e, e);
            } finally {
              setProcessed(st);
            }
          }
        };
      } else {
        throw new SiebelException("Illegal task's status: " + st.getStatus());
      }

      setInProcessing(st);
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: start to process task " + st);
      }
      executor.execute(thread);
    } catch (Throwable e) {
      logger.error(e,e);
      throw new SiebelException("internal_error");
    }
  }


  private static String convertMsisdn(String msisdn) {
    msisdn = msisdn.trim();
    if (msisdn.charAt(0) != '+') {
      if(msisdn.charAt(0) == '8') {
        msisdn = "+7" + msisdn.substring(1);
      }else {
        msisdn = '+' + msisdn;
      }
    }
    return msisdn;
  }

  private int addMessages(User u, Delivery delivery, ResultSet<SiebelMessage> messages) throws SiebelException {
    try {

      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: starting task generation...");
      }
      int countMessages = 0;

      Collection<String> unloaded = new ArrayList<String>(10001);

      Region r;
      while (messages.next()) {
        SiebelMessage sM = messages.get();
        String msisdn = sM.getMsisdn();
        if (msisdn != null && Address.validate(msisdn = convertMsisdn(msisdn))) {
          Address abonent = new Address(msisdn);
          if (u.isAllRegionsAllowed() || ((r = adminContext.getRegion(abonent)) != null
                  && u.getRegions().contains(r.getRegionId()))) {
            final Message msg = Message.newMessage(abonent, sM.getMessage());
            msg.setProperty(UserDataConsts.SIEBEL_MESSAGE_ID, sM.getClcId()); 
            countMessages++;
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

      if (!unloaded.isEmpty()) {
        updateUnloaded(unloaded);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: task generation ok: "+countMessages+" messages");
      }
      return countMessages;
    } catch (AdminException e) {
      logger.error(e, e);
//      try {
//        Iterator i = task.tasks.values().iterator();
//        while (i.hasNext()) {
//          ((TaskPart) i.next()).remove();
//        }
//      } catch (Throwable ex) {
//        logger.error(ex, ex);
//      }
//      throw new SiebelException(e);
      return 0;
    }
  }

  private void updateUnloaded(Collection<String> unloaded) {
    try {
      Map<String, SiebelMessage.DeliveryState> states = new HashMap<String, SiebelMessage.DeliveryState>();
      for (String anUnloaded : unloaded) {
        logger.error("Siebel: Unloaded message for '" + anUnloaded + '\'');
        states.put(anUnloaded, new SiebelMessage.DeliveryState(SiebelMessage.State.REJECTED, "11", "Invalid Dest Addr"));
      }
      provider.setMessageStates(states);
    } catch (Throwable e) {
      logger.error(e, e);
    }
  }

  private static Pattern msisdnPattern = Pattern.compile("^(\\+|)[0-9]+$");

  private static boolean checkMsidn(String msisdn) {
    return msisdn != null && msisdn.length() > 7 && msisdnPattern.matcher(msisdn).matches();
  }

  private Delivery createDelivery(User u, SiebelDelivery siebelTask, String taskName) throws AdminException {
    Delivery delivery = Delivery.newCommonDelivery();
    adminContext.getDefaultDelivery(u.getLogin(), delivery);
    delivery.setName(taskName);
    //todo beep, save

    delivery.setPriority(siebelTask.getPriority());
    delivery.setFlash(siebelTask.isFlash());
//      delivery.setReplaceMessage(cfg.isSiebelTReplaceMessage()); todo
    if (siebelTask.getExpPeriod() != null && siebelTask.getExpPeriod() != 0) {
      delivery.setValidityPeriod(siebelTask.getExpPeriod().toString());
    } else {
      delivery.setValidityPeriod("1");
    }

    delivery.setStartDate(new Date());
    return delivery;
  }

  private static String buildTaskName(String waveId) {
    return new StringBuffer(7 + waveId.length()).append("siebel_").append(waveId).toString();
  }

  private DeliveryInfo getExistingDelivery(User u, String waveId) throws AdminException {
    String taskName = buildTaskName(waveId);
    DeliveryFilter filter = new DeliveryFilter();
    filter.setNameFilter(new String[]{taskName});
    final DeliveryInfo[] infos = new DeliveryInfo[1];
    adminContext.getDeliveries(u.getLogin(), u.getPassword(), filter, 1,
        new Visitor<mobi.eyeline.informer.admin.delivery.DeliveryInfo>() {
          public boolean visit(DeliveryInfo value) throws AdminException {
            infos[1] = value;
            return false;
          }
        });
    return infos[0];
  }


}