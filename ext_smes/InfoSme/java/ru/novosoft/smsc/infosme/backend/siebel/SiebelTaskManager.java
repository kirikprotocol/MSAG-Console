package ru.novosoft.smsc.infosme.backend.siebel;

import org.apache.log4j.Logger;

import java.util.*;
import java.util.regex.Pattern;
import java.text.SimpleDateFormat;

import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.Message;


/**
 * author: alkhal
 */
public class SiebelTaskManager implements Runnable {

  private static final Logger logger = Logger.getLogger(SiebelTaskManager.class);

  private final static String TASK_OWNER = "siebel";

  private final SiebelDataProvider provider;

  private boolean shutdown = true;

  private final InfoSmeContext ctx;

  private long timeout = 20000;

  private Set processedTasks = new HashSet();

  private final Object processedTasksMonitor = new Object();

  public SiebelTaskManager(SiebelDataProvider dataProvider, InfoSmeContext infoSmeContext) {
    if (dataProvider == null) {
      throw new IllegalArgumentException("Some argument are null");
    }
    this.provider = dataProvider;
    this.ctx = infoSmeContext;
    if (ctx.getInfoSmeConfig().getSiebelTMPeriod() != 0) {
      timeout = 1000 * ctx.getInfoSmeConfig().getSiebelTMPeriod();
    }
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

  /** @noinspection EmptyCatchBlock*/
  public void run() {
    try {
      shutdown = false;
      while (!shutdown) {
        ResultSet rs = null;
        try {
          rs = provider.getTasksToUpdate();
          while (rs.next()) {
            final SiebelTask st = (SiebelTask) rs.get();

            if (logger.isDebugEnabled())
              logger.debug("Siebel: found task to update: " + st.getWaveId());

            boolean taskAlreadyInProcessing;
            synchronized (processedTasksMonitor) {
              taskAlreadyInProcessing = processedTasks.contains(st.getWaveId());
            }

            if (!taskAlreadyInProcessing) {
              synchronized (processedTasksMonitor) {
                processedTasks.add(st.getWaveId());
              }

              new Thread("Siebel-Task-'" + st.getWaveId() + "'-Processor") {
                public void run() {
                  try {
                    process(st);
                  } catch (Throwable e) {
                    logger.error(e, e);
                  } finally {
                    synchronized (processedTasksMonitor) {
                      processedTasks.remove(st.getWaveId());
                    }
                  }
                }
              }.start();
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

  private void _setTaskStatusInProcess(SiebelTask st) throws SiebelException {
    if (provider.getTaskStatus(st.getWaveId()) == SiebelTask.Status.ENQUEUED) {
      provider.setTaskStatus(st.getWaveId(), SiebelTask.Status.IN_PROCESS);
      if (logger.isDebugEnabled())
        logger.debug("Siebel: set tasK status to " + SiebelTask.Status.IN_PROCESS);
    }
  }

  private void beginTask(SiebelTask st) throws SiebelException {
    try {
      String taskname = buildTaskName(st.getWaveId());
      Task t;

      if (ctx.getInfoSmeConfig().containsTaskWithName(taskname, TASK_OWNER)) {
        t = ctx.getInfoSmeConfig().getTaskByName(taskname);
        if (t.isMessagesHaveLoaded()) {
          _startTask(t);
          _setTaskStatusInProcess(st);
          return;
        } else {
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: re-add tasK after crash " + st);
          }
          _removeTask(t);
        }
      }

      t = createTask(st, taskname);
      ResultSet messages = null;
      try {
        messages = provider.getMessages(st.getWaveId());
        addTask(ctx, t, messages);
      } finally {
        if (messages != null) {
          messages.close();
        }
      }
      _setTaskStatusInProcess(st);
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private void stopTask(SiebelTask st) throws SiebelException {
    try {
      String taskname = buildTaskName(st.getWaveId());
      Task t = ctx.getInfoSmeConfig().getTaskByName(taskname);
      if (t != null) {
        if (ctx.getInfoSmeConfig().isSiebelTMRemove()) {
          _removeTask(t);
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: remove task " + st);
          }
        } else {
          t.setEndDate(new Date());
          t.setEnabled(false);
          _changeTask(t);
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: pause tasK " + st);
          }
        }
      }
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private void _changeTask(Task t) throws SiebelException {
    try {
      ctx.getInfoSmeConfig().addAndApplyTask(t);
      ctx.getInfoSme().changeTask(t.getId());
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private void _addTask(Task t, boolean notifyInfoSme) throws SiebelException {
    try {
      ctx.getInfoSmeConfig().addAndApplyTask(t);
      if (notifyInfoSme) {
        ctx.getInfoSme().addTask(t.getId());
      }
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private void _removeTask(Task t) throws SiebelException {
    try {
      ctx.getInfoSmeConfig().removeAndApplyTask(TASK_OWNER, t.getId());
      ctx.getInfoSme().removeTask(t.getId());
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private void _startTask(Task t) throws SiebelException {
    try {
      if (!t.isEnabled()) {
        t.setEnabled(true);
        _changeTask(t);
      }
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private void _pauseTask(Task t) throws SiebelException {
    try {
      if (t.isEnabled()) {
        t.setEnabled(false);
        _changeTask(t);
      }
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private void pauseTask(SiebelTask st) throws SiebelException {
    try {
      String taskname = buildTaskName(st.getWaveId());
      Task t = ctx.getInfoSmeConfig().getTaskByName(taskname);
      _pauseTask(t);
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: pause task " + st);
      }
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private void process(SiebelTask st) throws SiebelException {
    try {
      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: start to process task" + st);
      }
      if (!ctx.getInfoSme().getInfo().isOnline()) {
        throw new SiebelException("Siebel: Infosme is offline");
      }
      if (st.getStatus() == SiebelTask.Status.ENQUEUED) {
        beginTask(st);
      } else if (st.getStatus() == SiebelTask.Status.STOPPED) {
        stopTask(st);
      } else if (st.getStatus() == SiebelTask.Status.PAUSED) {
        pauseTask(st);
      }
    } catch (Throwable e) {
      throw new SiebelException(e);
    }

  }

  /**
   * @noinspection EmptyCatchBlock
   */
  private void addTask(InfoSmeContext smeContext, Task task, ResultSet messages) throws SiebelException {
    try {
      task.setMessagesHaveLoaded(false);
      _addTask(task, true);

      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: starting task generation...");
      }

      final int maxMessagesPerSecond = smeContext.getInfoSmeConfig().getMaxMessagesPerSecond();

      long currentTime = task.getStartDate() == null ? System.currentTimeMillis() : task.getStartDate().getTime();

      Collection unloaded = new LinkedList();
      Collection toSend = loadMessage(maxMessagesPerSecond, new Date(currentTime), messages, unloaded);
      while (toSend != null && !toSend.isEmpty()) {
        smeContext.getInfoSme().addDeliveryMessages(task.getId(), toSend);
        currentTime += 1000;
        toSend = loadMessage(maxMessagesPerSecond, new Date(currentTime), messages, unloaded);
      }

      smeContext.getInfoSme().endDeliveryMessageGeneration(task.getId());

      task.setMessagesHaveLoaded(true);

      _addTask(task, false);

      if(!unloaded.isEmpty()) {
        updateUnloaded(unloaded);
      }

      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: task generation ok...");
      }

    } catch (Exception e) {
      logger.error(e, e);
      try {
        _removeTask(task);
      } catch (Throwable ex) {
        logger.error(ex, ex);
      }
      throw new SiebelException(e);
    }
  }

  private void updateUnloaded(Collection unloaded) {
    try{
      Iterator i = unloaded.iterator();
      Map states = new HashMap();
      while(i.hasNext()) {
        Object o = i.next();
        logger.error("Siebel: Unloaded message for '"+o+"'");
        states.put(o, new SiebelMessage.DeliveryState(SiebelMessage.State.REJECTED, "ESME_RINVDSTADR", "Invalid Dest Addr"));
      }
      provider.updateDeliveryStates(states);
    }catch(Throwable e) {
      logger.error(e,e);
    }
  }

  private static Collection loadMessage(int limit, Date sendDate, ResultSet messages, Collection unloaded) throws SiebelException {
    int i = 0;
    Collection result = new LinkedList();
    while (i < limit && messages.next()) {
      SiebelMessage sM = (SiebelMessage) messages.get();
      String msisdn = sM.getMsisdn();
      if(checkMsidn(msisdn)) {
        final Message msg = new Message();
        msg.setAbonent(msisdn);
        msg.setMessage(sM.getMessage());
        msg.setState(Message.State.NEW);
        msg.setUserData(sM.getClcId());
        msg.setSendDate(sendDate);
        result.add(msg);
        i++;
      }else {
        unloaded.add(sM.getClcId());
      }
    }

    return result;

  }

  private static Pattern msisdnPattern = Pattern.compile("^\\+[0-9]+$");

  private static boolean checkMsidn(String msisdn) {
    return msisdn != null && msisdn.length()>7 && msisdnPattern.matcher(msisdn).matches();
  }

  private Task createTask(SiebelTask siebelTask, String taskName) throws SiebelException {
    try {
      SimpleDateFormat tf = new SimpleDateFormat("HH:mm:ss");
      InfoSmeConfig cfg = ctx.getInfoSmeConfig();
      Task task = ctx.getInfoSmeConfig().createTask();
      task.setName(taskName);
      task.setOwner(TASK_OWNER);
      //todo beep, save

      task.setPriority(siebelTask.getPriority());
      task.setMessagesCacheSize(cfg.getSiebelTCacheSize());
      task.setMessagesCacheSleep(cfg.getSiebelTCacheSleep());
      task.setUncommitedInGeneration(cfg.getSiebelTUncommitGeneration());
      task.setUncommitedInProcess(cfg.getSiebelTUncommitProcess());
      task.setEnabled(true);
      task.setProvider(Task.INFOSME_EXT_PROVIDER);
      task.setKeepHistory(cfg.isSiebelTKeepHistory());
      task.setTrackIntegrity(cfg.isSiebelTTrackIntegrity());
      task.setFlash(siebelTask.isFlash());
      task.setReplaceMessage(cfg.isSiebelTReplaceMessage());
      task.setRetryOnFail(cfg.isSiebelTRetryOnFail());
      task.setSvcType(cfg.getSiebelTSvcType());
      task.setActivePeriodStart(cfg.getSiebelTPeriodStart());
      task.setActivePeriodEnd(cfg.getSiebelTPeriodEnd());
      task.setTransactionMode(cfg.isSiebelTTrMode());
      task.setDelivery(true);
      task.setSaveFinalState(true);
      if (siebelTask.getExpPeriod() != null && siebelTask.getExpPeriod().intValue() != 0) {
        if (siebelTask.getExpPeriod().intValue() > 24) {
          siebelTask.setExpPeriod(new Integer(24));
        }
        String prefix = (siebelTask.getExpPeriod().intValue() < 10) ? "0" : "";
        task.setValidityPeriod(tf.parse(prefix + siebelTask.getExpPeriod() + ":00:00"));
      } else {
        task.setValidityPeriod(tf.parse("01:00:00"));
      }

      task.setStartDate(new Date());
      return task;
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }


  private static String buildTaskName(String waveId) {
    return new StringBuffer(7 + waveId.length()).append("siebel_").append(waveId).toString();
  }
}
