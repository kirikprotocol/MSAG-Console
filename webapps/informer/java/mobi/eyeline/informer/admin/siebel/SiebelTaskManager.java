package mobi.eyeline.informer.admin.siebel;

import com.eyeline.utils.ThreadFactoryWithCounter;
import com.sun.jmx.snmp.tasks.Task;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManager;
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
public class SiebelTaskManager implements Runnable {

  private static final Logger logger = Logger.getLogger(SiebelTaskManager.class);

  private final static String TASK_OWNER = "siebel";

  private final SiebelDataProvider provider;

  private boolean shutdown = true;

  private long timeout = 20000;   //todo

  private Set<String> processedTasks = new HashSet<String>();

  private final Lock lock = new ReentrantLock();

  private final DeliveryManager deliveryManager;

  private final UsersManager userManager;

  private Executor executor = new ThreadPoolExecutor(1,Integer.MAX_VALUE, 60, TimeUnit.SECONDS, new LinkedBlockingQueue<Runnable>(), new ThreadFactoryWithCounter("Siebel-Task-Processor",0));

  public SiebelTaskManager(SiebelDataProvider dataProvider, DeliveryManager deliveryManager, UsersManager userManager) {
    if (dataProvider == null) {
      throw new IllegalArgumentException("Some argument are null");
    }
    this.provider = dataProvider;
    this.deliveryManager = deliveryManager;
    this.userManager = userManager;
  }

  public void shutdown() {
    shutdown = true;
  }

  public boolean isOnline() {
    return !shutdown;
  }

  public void start() {
    if (shutdown) {
      new Thread(this, "SiebelTaskManager").start();
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

  /**
   * @noinspection EmptyCatchBlock
   */
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
        } catch (InterruptedException e) {
        }
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: task manager stopped");
      }

    } catch (Throwable e) {
      logger.error(e, e);
    }
  }

  public void setTimeout(long timeout) throws AdminException{
    //todo
    this.timeout = timeout;
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

  private void beginTask(User u , SiebelDelivery st, DeliveryInfo t) throws SiebelException {
// todo   try {
//
//      if (t != null) {
//        if (t.isMessagesHaveLoaded()) {
//          if (logger.isDebugEnabled()) {
//            logger.debug("Siebel: task already exists and has generated, waveId='" + st.getWaveId() + "'. Enable it");
//          }
//          _startTask(t);
//          _setTaskStatusInProcess(st);
//          return;
//        } else {
//          if (logger.isDebugEnabled()) {
//            logger.debug("Siebel: task is damaged. Remove crached parts. WaveId=" + st.getWaveId());
//          }
//          t.removeUnloaded();
//        }
//      } else {
//        t = new MultiTask();
//      }
//
//      t.originTask = createTask(st, buildTaskName(st.getWaveId()));
//
//      InfoSmeConfig.validate(t.originTask);
//
//      ResultSet messages = null;
//      boolean hasMessages;
//      try {
//        if (logger.isDebugEnabled()) {
//          logger.debug("Siebel: task is new or doesn't contain some parts. Generate it. WaveId=" + st.getWaveId());
//        }
//        messages = provider.getMessages(st.getWaveId());
//        hasMessages = addMessages(ctx, t, messages) != 0;
//        t.done();
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
          deliveryManager.dropDelivery(u.getLogin(), u.getPassword(), task.getDeliveryId());
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: remove task " + task.getName());
          }
        } else {
          deliveryManager.cancelDelivery(u.getLogin(), u.getPassword(), task.getDeliveryId());
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
      deliveryManager.pauseDelivery(u.getLogin(), u.getPassword(), task.getDeliveryId());
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

      final DeliveryInfo  t = getExistingDelivery(u, st.getWaveId());

      final Runnable thread;

      if (st.getStatus() == SiebelDelivery.Status.ENQUEUED) {
        thread = new Runnable() {
          public void run() {
            try {
              if (logger.isDebugEnabled()) {
                logger.debug("Siebel: Found task in state=" + SiebelDelivery.Status.ENQUEUED + " WaveId=" + st.getWaveId());
              }
              beginTask(u, st, t);
            } catch (Throwable e) {
              logger.error(e, e);
            } finally {
              setProcessed(st);
            }
          }
        };
      } else if (st.getStatus() == SiebelDelivery.Status.STOPPED) {
//todo        final boolean remove;
//        if (t == null || (!(remove = ctx.getInfoSmeConfig().isSiebelTMRemove()) && t.getStatus() == DeliveryStatus.Cancelled)) {
//          if (logger.isDebugEnabled()) {
//            logger.debug("Siebel: Found task in state=" + SiebelDelivery.Status.STOPPED +
//                ". It doesn't exist or disabled or doesn't need to remove.  WaveId=" + st.getWaveId());
//          }
//          return;
//        }
//        thread = new Runnable() {
//          public void run() {
//            try {
//              if (logger.isDebugEnabled()) {
//                logger.debug("Siebel: Found task in state=" + SiebelDelivery.Status.STOPPED + " WaveId=" + st.getWaveId());
//              }
//              stopTask(u, t, remove);
//            } catch (Throwable e) {
//              logger.error(e, e);
//            } finally {
//              setProcessed(st);
//            }
//          }
//        };
      } else if (st.getStatus() == SiebelDelivery.Status.PAUSED) {
        if (t == null || t.getStatus() == DeliveryStatus.Paused) {
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: Found task in state=" + SiebelDelivery.Status.PAUSED +
                ". It doesn't exist or disabled.  WaveId=" + st.getWaveId());
          }
          return;
        }
        thread = new Runnable() {
          public void run() {
            try {
              pauseTask(u, t);
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
//todo      executor.execute(thread);
    } catch (Throwable e) {
      logger.error(e,e);
      throw new SiebelException("internal_error");
    }
  }


//todo  private int addMessages(InfoSmeContext smeContext, MultiTask task, ResultSet<DeliveryMessage> messages) throws SiebelException {
//    try {
//
//      if (logger.isDebugEnabled()) {
//        logger.debug("Siebel: starting task generation...");
//      }
//      int countMessages = 0;
//
//      TemplatesRadixTree rtree = new TemplatesRadixTree();
//      initiateRadixTree(rtree);
//
//      Collection unloaded = new ArrayList(10001);
//
//      while (messages.next()) {
//        Region r;
//        SiebelMessage sM = (SiebelMessage) messages.get();
//        String msisdn = sM.getMsisdn();
//        if (msisdn != null) {
//          msisdn = msisdn.trim();
//          if (msisdn.charAt(0) != '+')
//            msisdn = '+' + msisdn;
//          r = (Region) rtree.getValue(msisdn);
//          if (checkMsidn(msisdn) && r != null) {
//            final Message msg = new Message();
//            msg.setAbonent(msisdn);
//            msg.setMessage(sM.getMessage());
//            msg.setState(Message.State.NEW);
//            msg.setUserData(sM.getClcId());
//            Integer regionId = new Integer(r.getId());
//            TaskPart taskPart = (TaskPart) task.tasks.get(regionId);
//            if (taskPart == null) {
//              taskPart = new TaskPart(task.originTask, r, smeContext);
//              task.tasks.put(regionId, taskPart);
//            }
//            taskPart.addMessage(msg);
//            countMessages++;
//          } else {
//            unloaded.add(sM.getClcId());
//          }
//        } else {
//          unloaded.add(sM.getClcId());
//        }
//
//        if (unloaded.size() == 10000) {
//          updateUnloaded(unloaded);
//          unloaded.clear();
//        }
//      }
//
//      if (!unloaded.isEmpty()) {
//        updateUnloaded(unloaded);
//      }
//
//      if (logger.isDebugEnabled()) {
//        logger.debug("Siebel: task generation ok: "+countMessages+" messages");
//      }
//      return countMessages;
//    } catch (AdminException e) {
//      logger.error(e, e);
//      try {
//        Iterator i = task.tasks.values().iterator();
//        while (i.hasNext()) {
//          ((TaskPart) i.next()).remove();
//        }
//      } catch (Throwable ex) {
//        logger.error(ex, ex);
//      }
//      throw new SiebelException(e);
//    }
//  }

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

  private Task createTask(SiebelDelivery siebelTask, String taskName) throws SiebelException {
// todo   try {
//      InfoSmeConfig cfg = ctx.getInfoSmeConfig();
//      Task task = ctx.getInfoSmeConfig().createTask();
//      task.setName(taskName);
//      task.setOwner(TASK_OWNER);
//      //todo beep, save
//
//      task.setPriority(siebelTask.getPriority());
//      task.setMessagesCacheSize(cfg.getSiebelTCacheSize());
//      task.setMessagesCacheSleep(cfg.getSiebelTCacheSleep());
//      task.setUncommitedInGeneration(cfg.getSiebelTUncommitGeneration());
//      task.setUncommitedInProcess(cfg.getSiebelTUncommitProcess());
//      task.setEnabled(true);
//      task.setProvider(Task.INFOSME_EXT_PROVIDER);
//      task.setKeepHistory(cfg.isSiebelTKeepHistory());
//      task.setTrackIntegrity(cfg.isSiebelTTrackIntegrity());
//      task.setFlash(siebelTask.isFlash());
//      task.setReplaceMessage(cfg.isSiebelTReplaceMessage());
//      if (cfg.isSiebelTRetryOnFail()) {
//        task.setRetryOnFail(true);
//        task.setRetryPolicy(cfg.getSiebelTRetryPolicy());
//      }
//      task.setSvcType(cfg.getSiebelTSvcType());
//      task.setActivePeriodStart(cfg.getSiebelTPeriodStart());
//      task.setActivePeriodEnd(cfg.getSiebelTPeriodEnd());
//      task.setActiveWeekDays(cfg.getSiebelTWeekDaysSet());
//      task.setTransactionMode(cfg.isSiebelTTrMode());
//      task.setDelivery(true);
//      task.setSaveFinalState(true);
//      if (siebelTask.getExpPeriod() != null && siebelTask.getExpPeriod().intValue() != 0) {
//        task.setValidityPeriod(siebelTask.getExpPeriod());
//      } else {
//        task.setValidityPeriod(new Integer(1));
//      }
//
//      task.setStartDate(new Date());
//      return task;
//    } catch (Throwable e) {
//      throw new SiebelException(e);
//    }
    return null;
  }

  private static String buildTaskName(String waveId) {
    return new StringBuffer(7 + waveId.length()).append("siebel_").append(waveId).toString();
  }

  private DeliveryInfo getExistingDelivery(User u, String waveId) throws AdminException {
    String taskName = buildTaskName(waveId);
    DeliveryFilter filter = new DeliveryFilter();
    filter.setNameFilter(new String[]{taskName});
    final DeliveryInfo[] infos = new DeliveryInfo[1];
    deliveryManager.getDeliveries(u.getLogin(), u.getPassword(), filter, 1,
        new Visitor<mobi.eyeline.informer.admin.delivery.DeliveryInfo>() {
      public boolean visit(DeliveryInfo value) throws AdminException {
        infos[1] = value;
        return false;
      }
    });
    return infos[0];
  }


}