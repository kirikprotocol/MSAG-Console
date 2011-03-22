package ru.novosoft.smsc.infosme.backend.siebel;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.infosme.backend.InfoSmeContext;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.backend.radixtree.TemplatesRadixTree;
import ru.novosoft.smsc.infosme.beans.InfoSmeBean;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.*;
import java.util.regex.Pattern;


/**
 * author: alkhal
 */
public class SiebelTaskManager implements Runnable {

  private static final Logger logger = Logger.getLogger(SiebelTaskManager.class);

  private final static String TASK_OWNER = "siebel";

  private final SiebelDataProvider provider;

  private boolean shutdown = true;

  private final InfoSmeContext ctx;
  private final SMSCAppContext appContext;

  private long timeout = 20000;

  private Set processedTasks = new HashSet();

  private final Object processedTasksMonitor = new Object();

  public SiebelTaskManager(SiebelDataProvider dataProvider, SMSCAppContext appContext, InfoSmeContext infoSmeContext) {
    if (dataProvider == null) {
      throw new IllegalArgumentException("Some argument are null");
    }
    this.provider = dataProvider;
    this.ctx = infoSmeContext;
    this.appContext = appContext;
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

  private boolean isInProcessing(SiebelTask st) {
    synchronized (processedTasksMonitor) {
      return processedTasks.contains(st.getWaveId());
    }
  }

  private void setInProcessing(SiebelTask st) {
    synchronized (processedTasksMonitor) {
      processedTasks.add(st.getWaveId());
    }
  }

  private void setProcessed(SiebelTask st) {
    synchronized (processedTasksMonitor) {
      processedTasks.remove(st.getWaveId());
    }
  }

  /**
   * @noinspection EmptyCatchBlock
   */
  public void run() {
    try {
      shutdown = false;
      while (!shutdown) {
        ResultSet rs = null;
        try {
          rs = provider.getTasksToUpdate();
          while (rs.next()) {
            try{
              final SiebelTask st = (SiebelTask) rs.get();

              if (logger.isDebugEnabled())
                logger.debug("Siebel: found task to update: " + st.getWaveId());

              if (!isInProcessing(st)) {
                process(st);
              } else {
                if (logger.isDebugEnabled())
                  logger.debug("Siebel: task: " + st.getWaveId() + " already in processing...");
              }
            }catch (Exception e){
              logger.error(e, e);
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
  private void _setTaskStatusInProcessed(SiebelTask st) throws SiebelException {
    if (provider.getTaskStatus(st.getWaveId()) == SiebelTask.Status.ENQUEUED) {
      provider.setTaskStatus(st.getWaveId(), SiebelTask.Status.PROCESSED);
      if (logger.isDebugEnabled())
        logger.debug("Siebel: set tasK status to " + SiebelTask.Status.PROCESSED);
    }

  }

  private void beginTask(SiebelTask st, MultiTask t) throws SiebelException {
    try {

      if (t != null) {
        if (t.isMessagesHaveLoaded()) {
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: task already exists and has generated, waveId='" + st.getWaveId() + "'. Enable it");
          }
          _startTask(t);
          _setTaskStatusInProcess(st);
          return;
        } else {
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: task is damaged. Remove crached parts. WaveId=" + st.getWaveId());
          }
          t.removeUnloaded();
        }
      } else {
        t = new MultiTask();
      }

      t.originTask = createTask(st, buildTaskName(st.getWaveId()));

      InfoSmeConfig.validate(t.originTask);

      ResultSet messages = null;
      boolean hasMessages;
      try {
        if (logger.isDebugEnabled()) {
          logger.debug("Siebel: task is new or doesn't contain some parts. Generate it. WaveId=" + st.getWaveId());
        }
        messages = provider.getMessages(st.getWaveId());
        hasMessages = addMessages(ctx, t, messages) != 0;
        t.done();
      } finally {
        if (messages != null) {
          messages.close();
        }
      }
      if(hasMessages) {
        _setTaskStatusInProcess(st);
      }else {
        _setTaskStatusInProcessed(st);
      }
    } catch (AdminException e) {
      throw new SiebelException(e);
    }
  }

  private void stopTask(MultiTask task, boolean remove) throws SiebelException {
    try {
      if (task != null) {
        Iterator i = task.tasks.values().iterator();
        while (i.hasNext()) {
          TaskPart t = (TaskPart) i.next();
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: Stop task " + t.task.getName());
          }
          if (remove) {
            t.remove();
            if (logger.isDebugEnabled()) {
              logger.debug("Siebel: remove task " + t.task.getName());
            }
          } else {
            t.setEndDate(new Date());
            t.setEnabled(false);
            t.changeTask();
            if (logger.isDebugEnabled()) {
              logger.debug("Siebel: stop tasK " + t.task.getName());
            }
          }
        }
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

  private void _startTask(MultiTask task) throws SiebelException {
    try {
      Iterator i = task.tasks.values().iterator();
      while (i.hasNext()) {
        TaskPart t = (TaskPart) i.next();
        if (!t.task.isEnabled()) {
          t.setEnabled(true);
          t.changeTask();
        }
      }
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private void pauseTask(MultiTask task) throws SiebelException {
    try {
      Iterator i = task.tasks.values().iterator();
      while (i.hasNext()) {
        TaskPart t = (TaskPart) i.next();
        if (t.isEnabled()) {
          t.setEnabled(false);
          t.changeTask();
        }
        if (logger.isDebugEnabled()) {
          logger.debug("Siebel: pause task " + t.task.getName());
        }
      }
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private void process(final SiebelTask st) throws SiebelException {
    try {
      if (!ctx.getInfoSme().getInfo().isOnline()) {
        throw new SiebelException("Siebel: Infosme is offline");
      }

      final String threadName = new StringBuffer().append("Siebel-Task-'").append(st.getWaveId()).append("'-Processor").toString();


      final MultiTask t;

      t = getMultiTask(ctx, st.getWaveId());

      final Thread thread;

      if (st.getStatus() == SiebelTask.Status.ENQUEUED) {
        thread = new Thread(threadName) {
          public void run() {
            try {
              if (logger.isDebugEnabled()) {
                logger.debug("Siebel: Found task in state=" + SiebelTask.Status.ENQUEUED + " WaveId=" + st.getWaveId());
              }
              beginTask(st, t);
            } catch (Throwable e) {
              logger.error(e, e);
            } finally {
              setProcessed(st);
            }
          }
        };
      } else if (st.getStatus() == SiebelTask.Status.STOPPED) {
        final boolean remove;
        if (t == null || (!(remove = ctx.getInfoSmeConfig().isSiebelTMRemove()) && !t.isEnabled())) {
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: Found task in state=" + SiebelTask.Status.STOPPED +
                ". It doesn't exist or disabled or doesn't need to remove.  WaveId=" + st.getWaveId());
          }
          return;
        }
        thread = new Thread(threadName) {
          public void run() {
            try {
              if (logger.isDebugEnabled()) {
                logger.debug("Siebel: Found task in state=" + SiebelTask.Status.STOPPED + " WaveId=" + st.getWaveId());
              }
              stopTask(t, remove);
            } catch (Throwable e) {
              logger.error(e, e);
            } finally {
              setProcessed(st);
            }
          }
        };
      } else if (st.getStatus() == SiebelTask.Status.PAUSED) {
        if (t == null || !t.isEnabled()) {
          if (logger.isDebugEnabled()) {
            logger.debug("Siebel: Found task in state=" + SiebelTask.Status.PAUSED +
                ". It doesn't exist or disabled.  WaveId=" + st.getWaveId());
          }
          return;
        }
        thread = new Thread(threadName) {
          public void run() {
            try {
              pauseTask(t);
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
      thread.start();
    } catch (Throwable e) {
      throw new SiebelException(e);
    }
  }

  private void initiateRadixTree(TemplatesRadixTree tree) {
    Collection regions = appContext.getRegionsManager().getRegions();
    User user = appContext.getUserManager().getUser(TASK_OWNER);
    boolean admin = user == null || user.getRoles().contains(InfoSmeBean.INFOSME_ADMIN_ROLE);

    Region r;
    for (Iterator regionsIter = regions.iterator(); regionsIter.hasNext();) {
      r = (Region) regionsIter.next();
      if (admin || user.getPrefs().isInfoSmeRegionAllowed(String.valueOf(r.getId()))) {
        for (Iterator subjectsIter = r.getSubjects().iterator(); subjectsIter.hasNext();) {
          String subjectName = (String) subjectsIter.next();
          Subject s = appContext.getRouteSubjectManager().getSubjects().get(subjectName);
          tree.add(s.getMasks().getNames(), r);
        }
      }
    }
  }

  private int addMessages(InfoSmeContext smeContext, MultiTask task, ResultSet messages) throws SiebelException {
    try {

      if (logger.isDebugEnabled()) {
        logger.debug("Siebel: starting task generation...");
      }
      int countMessages = 0;

      TemplatesRadixTree rtree = new TemplatesRadixTree();
      initiateRadixTree(rtree);

      Collection unloaded = new ArrayList(10001);

      while (messages.next()) {
        Region r;
        SiebelMessage sM = (SiebelMessage) messages.get();
        String msisdn = sM.getMsisdn();
        if (msisdn != null) {
          msisdn = msisdn.trim();
          if (msisdn.charAt(0) != '+')
            msisdn = '+' + msisdn;
          r = (Region) rtree.getValue(msisdn);
          if (checkMsidn(msisdn) && r != null) {
            final Message msg = new Message();
            msg.setAbonent(msisdn);
            msg.setMessage(sM.getMessage());
            msg.setState(Message.State.NEW);
            msg.setUserData(sM.getClcId());
            Integer regionId = new Integer(r.getId());
            TaskPart taskPart = (TaskPart) task.tasks.get(regionId);
            if (taskPart == null) {
              taskPart = new TaskPart(task.originTask, r, smeContext);
              task.tasks.put(regionId, taskPart);
            }
            taskPart.addMessage(msg);
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
      try {
        Iterator i = task.tasks.values().iterator();
        while (i.hasNext()) {
          ((TaskPart) i.next()).remove();
        }
      } catch (Throwable ex) {
        logger.error(ex, ex);
      }
      throw new SiebelException(e);
    }
  }

  private void updateUnloaded(Collection unloaded) {
    try {
      Iterator i = unloaded.iterator();
      Map states = new HashMap();
      while (i.hasNext()) {
        Object o = i.next();
        logger.error("Siebel: Unloaded message for '" + o + '\'');
        states.put(o, new SiebelMessage.DeliveryState(SiebelMessage.State.REJECTED, "11", "Invalid Dest Addr"));
      }
      provider.updateDeliveryStates(states);
    } catch (Throwable e) {
      logger.error(e, e);
    }
  }

  private static Pattern msisdnPattern = Pattern.compile("^(\\+|)[0-9]+$");

  private static boolean checkMsidn(String msisdn) {
    return msisdn != null && msisdn.length() > 7 && msisdnPattern.matcher(msisdn).matches();
  }

  private Task createTask(SiebelTask siebelTask, String taskName) throws SiebelException {
    try {
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
      if (cfg.isSiebelTRetryOnFail()) {
        task.setRetryOnFail(true);
        task.setRetryPolicy(cfg.getSiebelTRetryPolicy());
      }
      task.setSvcType(cfg.getSiebelTSvcType());
      task.setActivePeriodStart(cfg.getSiebelTPeriodStart());
      task.setActivePeriodEnd(cfg.getSiebelTPeriodEnd());
      task.setActiveWeekDays(cfg.getSiebelTWeekDaysSet());
      task.setTransactionMode(cfg.isSiebelTTrMode());
      task.setDelivery(true);
      task.setSaveFinalState(true);
      if (siebelTask.getExpPeriod() != null && siebelTask.getExpPeriod().intValue() != 0) {
        task.setValidityPeriod(siebelTask.getExpPeriod());
      } else {
        task.setValidityPeriod(new Integer(1));
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

  private MultiTask getMultiTask(InfoSmeContext smeContext, String waveId) throws SiebelException {
    String taskPrefix = buildTaskName(waveId);
    Map tasks = new HashMap();
    List allTasks = smeContext.getInfoSmeConfig().getTasks(TASK_OWNER);
    if (allTasks != null && !allTasks.isEmpty()) {
      Collection regions = appContext.getRegionsManager().getRegions();
      Iterator i = allTasks.iterator();
      while (i.hasNext()) {
        Task t = (Task) i.next();
        if (t.getName().startsWith(taskPrefix)) {
          Region r = null;
          if (t.getName().endsWith(")")) {
            String regionName = t.getName().substring(0, t.getName().length() - 1);
            regionName = regionName.substring(regionName.lastIndexOf("(") + 1);
            Iterator iter = regions.iterator();
            while (iter.hasNext()) {
              Region next = (Region) iter.next();
              if (next.getName().equals(regionName)) {
                r = next;
                break;
              }
            }
          }
          try {
            TaskPart tp = new TaskPart(t, smeContext);
            tasks.put(r != null ? new Integer(r.getId()) : new Integer(Integer.MAX_VALUE), tp);
          } catch (AdminException e) {
            throw new SiebelException(e);
          }
        }
      }
    }
    if (tasks.isEmpty()) {
      return null;
    }
    MultiTask t = new MultiTask();
    t.tasks = tasks;
    return t;
  }


  private class MultiTask {

    private Task originTask;

    private Map tasks = new HashMap();

    private boolean isMessagesHaveLoaded() {
      Iterator i = tasks.values().iterator();
      while (i.hasNext()) {
        TaskPart t = (TaskPart) i.next();
        if (!t.task.isMessagesHaveLoaded()) {
          return false;
        }
      }
      return true;
    }

    private void removeUnloaded() throws SiebelException {
      Iterator i = tasks.values().iterator();
      while (i.hasNext()) {
        TaskPart t = (TaskPart) i.next();
        if (!t.task.isMessagesHaveLoaded()) {
          _removeTask(t.task);
          i.remove();
        }
      }
    }

    public boolean isEnabled() {
      Iterator i = tasks.values().iterator();
      while (i.hasNext()) {
        if (((TaskPart) i.next()).task.isEnabled()) {
          return true;
        }
      }
      return false;
    }

    public void done() throws AdminException {
      Iterator i = tasks.values().iterator();
      while (i.hasNext()) {
        TaskPart tp = (TaskPart) i.next();
        tp.done();
      }
    }

  }

  private static class TaskPart {

    private Task task;
    private final List messageBuffer;
    private final int maxMessagesPerSecond;
    private Date startDate;
    private final InfoSmeContext smeContext;

    private TaskPart(Task task, InfoSmeContext smeContext) throws AdminException {
      this.maxMessagesPerSecond = smeContext.getLicense().getMaxSmsThroughput();
      this.messageBuffer = new ArrayList(maxMessagesPerSecond + 1);
      this.smeContext = smeContext;
      this.startDate = new Date();
      this.task = task;
    }

    private TaskPart(Task originalTask, Region r, InfoSmeContext smeContext) throws AdminException {
      this.maxMessagesPerSecond = smeContext.getLicense().getMaxSmsThroughput();
      this.messageBuffer = new ArrayList(maxMessagesPerSecond + 1);
      this.smeContext = smeContext;
      this.startDate = new Date();

      task = originalTask.cloneTask();
      if (r != null) {
        task.setName(task.getName() + '(' + r.getName() + ')');
      }
      task.setMessagesHaveLoaded(false);

      // Convert date to TZ
      if (r != null) {
        task.setActivePeriodStart(r.getLocalTime(task.getActivePeriodStart()));
        task.setActivePeriodEnd(r.getLocalTime(task.getActivePeriodEnd()));
      }
      // Create new task in InfoSme or use existed task

      smeContext.getInfoSmeConfig().addAndApplyTask(task);
      smeContext.getInfoSme().addTask(task.getId());
    }

    public void addMessage(Message m) throws AdminException {
      if (task.isMessagesHaveLoaded()) {
        return;
      }
      m.setSendDate(startDate);
      messageBuffer.add(m);

      if (messageBuffer.size() == maxMessagesPerSecond)
        flushBuffer();
    }

    private void flushBuffer() throws AdminException {
      if (!messageBuffer.isEmpty()) {
        smeContext.getInfoSme().addDeliveryMessages(task.getId(), messageBuffer);
        messageBuffer.clear();
      }
      startDate.setTime(startDate.getTime() + 1000);
    }

    public void done() throws AdminException {
      flushBuffer();

      smeContext.getInfoSme().endDeliveryMessageGeneration(task.getId());
      task.setMessagesHaveLoaded(true);
      smeContext.getInfoSmeConfig().addAndApplyTask(task);
    }

    public void remove() throws AdminException {
      if (task.getId() != null && smeContext.getInfoSmeConfig().containsTaskWithId(task.getId())) {
        smeContext.getInfoSmeConfig().removeAndApplyTask(task.getOwner(), task.getId());
        smeContext.getInfoSme().removeTask(task.getId());
      }
    }

    private void setEnabled(boolean enabled) {
      task.setEnabled(enabled);
    }

    private boolean isEnabled() {
      return task.isEnabled();
    }

    private void setEndDate(Date date) {
      task.setEndDate(date);
    }

    private void changeTask() throws SiebelException {
      if (task.getId() != null) {
        try {
          smeContext.getInfoSmeConfig().addAndApplyTask(task);
          smeContext.getInfoSme().changeTask(task.getId());
        } catch (Throwable e) {
          throw new SiebelException(e);
        }
      }
    }
  }


}
