package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.Daemon;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManager;
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
public class RestrictionDaemon implements Daemon {

  Logger log = Logger.getLogger(this.getClass());


  public static final String NAME = "RestrictionDaemon";
  DeliveryManager deliveryManager;
  RestrictionsManager restrictionManager;
  UsersManager userManager;
  ScheduledExecutorService scheduler = null;
  ScheduledFuture task;
  long startDate;
  int taskNum;


  public RestrictionDaemon(DeliveryManager deliveryManager, RestrictionsManager restrictionManager, UsersManager userManager) {
    this.deliveryManager = deliveryManager;
    this.restrictionManager = restrictionManager;
    this.userManager = userManager;
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
    //System.out.println("next schedule<-");
    if (isStarted()) {
      RestrictionsFilter rFilter = new RestrictionsFilter();
      List<Restriction> restrictions = restrictionManager.getRestrictions(rFilter);
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
      //System.out.println("task "+taskNum+" delay="+delay);
    }

    //System.out.println("next schedule->");
  }

  private void cancelCurrentTask() {
    if (task != null) {
      if (!task.isDone()) task.cancel(false);
      task = null;
    }
  }


  private void applyRestrictions(long forDate) throws AdminException {
    //System.out.println("apply restrictions<-");
    List<User> users = userManager.getUsers();
    for (final User u : users) {

      //do not get restrictions if user disabled
      final List<Restriction> restrictions =
          (u.getStatus() == User.Status.ENABLED) ?
              getActiveRestrictions(forDate, u)
              :
              null;

      DeliveryFilter dFilter = new DeliveryFilter();
      dFilter.setUserIdFilter(u.getLogin());
      dFilter.setStatusFilter(DeliveryStatus.Planned, DeliveryStatus.Active, DeliveryStatus.Paused);
      dFilter.setResultFields(DeliveryFields.Status);
      //System.out.println("apply restrictions get deliveries");
      deliveryManager.getDeliveries(u.getLogin(), u.getPassword(), dFilter, 1000,
          new Visitor<DeliveryInfo>() {
            public boolean visit(DeliveryInfo di) throws AdminException {
              //System.out.println("visit");
              boolean shouldBeRestricted;
              if (restrictions != null) {
                shouldBeRestricted = false;
                for (Restriction r : restrictions) {
                  if (r.isAllUsers() || r.getUserIds().contains(u.getLogin())) {
                    shouldBeRestricted = true;
                    break;
                  }
                }
              } else {
                shouldBeRestricted = true;
              }
              adjustDeliveryState(u, di, shouldBeRestricted);
              return true;
            }
          });
    }
    //System.out.println("applyRestictions->");
  }

  private List<Restriction> getActiveRestrictions(long startDate, User u) {
    RestrictionsFilter rFilter = new RestrictionsFilter();
    rFilter.setStartDate(new Date(startDate));
    rFilter.setEndDate(new Date(startDate + 1));
    rFilter.setUserId(u.getLogin());
    return restrictionManager.getRestrictions(rFilter);
  }

  private void adjustDeliveryState(User u, DeliveryInfo di, boolean shouldBeRestricted) throws AdminException {
    //System.out.println("adjust delivery "+di.getDeliveryId()+" "+di.getUserId()+" to restricted="+shouldBeRestricted);
    if (shouldBeRestricted) {
      if (di.getStatus() != DeliveryStatus.Paused) {
        //System.out.println("changed");
        deliveryManager.setDeliveryRestriction(u.getLogin(), u.getPassword(), di.getDeliveryId(), true);
        deliveryManager.pauseDelivery(u.getLogin(), u.getPassword(), di.getDeliveryId());
      }
    } else {
      if (di.getStatus() == DeliveryStatus.Paused && Boolean.valueOf(di.getProperty(UserDataConsts.RESTRICTION))) {
        //System.out.println("changed");
        deliveryManager.setDeliveryRestriction(u.getLogin(), u.getPassword(), di.getDeliveryId(), false);
        deliveryManager.activateDelivery(u.getLogin(), u.getPassword(), di.getDeliveryId());
      }
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
