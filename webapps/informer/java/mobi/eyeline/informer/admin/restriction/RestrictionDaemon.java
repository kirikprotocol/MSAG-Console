package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.users.User;
import org.apache.log4j.Logger;

import java.util.Date;
import java.util.List;
import java.util.concurrent.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 08.11.2010
 * Time: 16:35:50
 */
class RestrictionDaemon {

  Logger log = Logger.getLogger(NAME);


  public static final String NAME = "RestrictionDaemon";

  private RestrictionContext context;
  private final RestrictionsManager manager;
  ScheduledExecutorService scheduler = null;
  ScheduledFuture task;
  long startDate;
  int taskNum;


  public RestrictionDaemon(RestrictionContext context, RestrictionsManager manager) {
    this.context = context;
    this.manager = manager;
  }


  public String getName() {
    return NAME;
  }

  public synchronized void start() throws AdminException {

    if (!isStarted()) {
      scheduler = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
        public Thread newThread(Runnable runnable) {
          return new Thread(runnable, NAME);
        }
      });
      taskNum = 0;
      rebuildSchedule();
    }
  }

  public synchronized void rebuildSchedule() {
    startDate = 0;
    cancelCurrentTask();
    nextSchedule();
  }


  private synchronized void nextSchedule() {
    log.debug("next schedule<-");
    if (isStarted()) {
      RestrictionsFilter rFilter = new RestrictionsFilter();
      List<Restriction> restrictions = manager.getRestrictions(rFilter);
      RestrictionTask rTask;
      long delay;

      long minTime = startDate == 0 ? 0 : Long.MAX_VALUE;
      for (Restriction r : restrictions) {
        long eventTime = r.getStartDate().getTime();
        if (eventTime > startDate) {
          minTime = Math.min(eventTime, minTime);
        }
        eventTime = r.getEndDate().getTime();
        if (eventTime > startDate) {
          minTime = Math.min(eventTime, minTime);
        }
      }
      startDate = System.currentTimeMillis();
      delay = minTime - startDate;
      if (delay < 0) delay = 0L;
      startDate = startDate + delay;
      rTask = new RestrictionTask(taskNum, startDate);
      taskNum++;

      task = scheduler.schedule(rTask, delay, TimeUnit.MILLISECONDS);
      log.debug("task "+taskNum+" delay="+delay);
    }

    log.debug("next schedule->");
  }

  private void cancelCurrentTask() {
    if (task != null) {
      if (!task.isDone()) task.cancel(false);
      task = null;
    }
  }


  private void applyRestrictions(long forDate) throws AdminException {
    log.debug("apply restrictions<-");
    List<User> users = context.getUsers();
    for (final User u : users) {

      //do not get restrictions if user disabled
      final List<Restriction> restrictions =
          (u.getStatus() == User.Status.ENABLED) ?
              getActiveRestrictions(forDate, u)
              :
              null;

      log.debug("user restrictions = "+restrictions);
      
      DeliveryFilter dFilter = new DeliveryFilter();
      dFilter.setUserIdFilter(u.getLogin());
      dFilter.setStatusFilter(DeliveryStatus.Planned, DeliveryStatus.Active, DeliveryStatus.Paused);
      log.debug("get deliveries for user = "+u.getLogin());
      context.getDeliveries(u.getLogin(), dFilter,
          new Visitor<Delivery>() {
            public boolean visit(Delivery di) throws AdminException {
              log.debug("check state for delivery "+di);
              boolean shouldBeRestricted;
              if (restrictions != null) {
                shouldBeRestricted = false;
                for (Restriction r : restrictions) {
                  if (r.isAllUsers() || (r.getUserIds()!=null && r.getUserIds().contains(u.getLogin()))) {
                    shouldBeRestricted = true;
                    log.debug("should be restricted r.allUsers="+r.isAllUsers()+" userIds="+r.getUserIds());
                    break;
                  }
                }
              } else {
                log.debug("should be restricted (user not enabled)");
                shouldBeRestricted = true;
              }
              adjustDeliveryState(u, di, shouldBeRestricted);
              return true;
            }
          });
    }
    log.debug("applyRestictions->");
  }

  private List<Restriction> getActiveRestrictions(long startDate, User u) {
    RestrictionsFilter rFilter = new RestrictionsFilter();
    rFilter.setStartDate(new Date(startDate));
    rFilter.setEndDate(new Date(startDate + 1));
    rFilter.setUserId(u.getLogin());
    return manager.getRestrictions(rFilter);
  }

  private void adjustDeliveryState(User u, Delivery di, boolean shouldBeRestricted) throws AdminException {
    log.debug("Adjust delivery state "+di.getId()+" "+di.getOwner()+" to restricted="+shouldBeRestricted);
    try {
      if (shouldBeRestricted) {
        if (di.getStatus() != DeliveryStatus.Paused) {
          log.debug("pause delivery "+di);
          context.setDeliveryRestriction(u.getLogin(), di.getId(), true);
          context.pauseDelivery(u.getLogin(), di.getId());
        }
      }
      else {
        if ( (di.getStatus() == DeliveryStatus.Paused) && Boolean.valueOf(di.getProperty(UserDataConsts.RESTRICTION))) {
          log.debug("activate delivery "+di);
          context.setDeliveryRestriction(u.getLogin(), di.getId(), false);
          context.activateDelivery(u.getLogin(), di.getId());
        }
      }
    }
    catch (Exception e) {
      log.error("Error adjusting delivery state ",e);
    }
  }

  public void stop() throws AdminException {
    ExecutorService executor;
    if (isStarted()) {
      synchronized (this) {
        executor = scheduler;
        scheduler.shutdownNow(); // Cancel currently executing tasks
        scheduler = null;
        task = null;
      }

      //wait termination
      try {
        if (!executor.awaitTermination(60, TimeUnit.SECONDS)) {
          log.error("Unable to shutdown  " + NAME + " in 60 sec");
        }
      }
      catch (InterruptedException e) {
        log.error("Unable to shutdown " + NAME, e);
      }
    }
  }


  public boolean isStarted() {
    return scheduler != null;
  }

  public long getTaskDate() {
    return startDate;
  }

  public int getTaskNum() {
    return taskNum;
  }

  class RestrictionTask implements Callable<Object> {
    Long forDate;
    int num;

    public RestrictionTask(int num, long forDate) {
      this.forDate = forDate;
      this.num = num;
    }

    public Object call() throws Exception {
      applyRestrictions(forDate);
      nextSchedule();
      return null;
    }
  }
}
