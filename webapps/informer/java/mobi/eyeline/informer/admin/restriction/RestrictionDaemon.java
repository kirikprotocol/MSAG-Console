package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.Daemon;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManager;

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

  public final static DeliveryStatus[] STATUSES_FOR_BOTH_EVENTS = {DeliveryStatus.Planned,DeliveryStatus.Active,DeliveryStatus.Paused};
  public final static DeliveryStatus[] STATUSES_FOR_START_EVENT = {DeliveryStatus.Planned,DeliveryStatus.Active};
  public final static DeliveryStatus[] STATUSES_FOR_STOP_EVENT  = {DeliveryStatus.Paused};

  public static final String NAME = "RestrictionDaemon";
  DeliveryManager deliveryManager;
  RestrictionsManager restrictionManager;
  UsersManager userManager;
  ScheduledExecutorService scheduler=null;
  ScheduledFuture task;
  long startDate;
  int taskNum;


  public RestrictionDaemon(DeliveryManager deliveryManager, RestrictionsManager restrictionManager, UsersManager userManager ) {
    this.deliveryManager=deliveryManager;
    this.restrictionManager = restrictionManager;
    this.userManager = userManager;
  }


  public String getName() {
    return NAME;
  }

  public synchronized void start() throws AdminException {
    scheduler = Executors.newSingleThreadScheduledExecutor();
    taskNum=0;
    rebuildSchedule();

  }

  public synchronized void rebuildSchedule() {
    startDate = 0;
    cancelCurrentTask();
    nextSchedule();
  }


  private synchronized void nextSchedule() {
    if(isStarted()) {
      long minTime = startDate==0 ? 0 : Long.MAX_VALUE;
      RestrictionsFilter rFilter = new RestrictionsFilter();
      List<Restriction> restrictions = restrictionManager.getRestrictions(rFilter);
      for(Restriction r : restrictions) {
        long eventTime = r.getStartDate().getTime();
        if( eventTime > startDate ) {
          minTime = Math.min(eventTime,minTime);
        }
        eventTime = r.getEndDate().getTime();
        if( eventTime > startDate ) {
          minTime = Math.min(eventTime,minTime);
        }
      }
      startDate = System.currentTimeMillis();
      long delay = minTime - startDate;
      if(delay < 0) delay = 0L ;
      startDate = startDate+delay;
      task = scheduler.schedule(new RestrictionTask(taskNum,startDate),delay,TimeUnit.MILLISECONDS);
      taskNum++;
      System.out.println("task "+taskNum+" delay="+delay);
    }
  }

  private void cancelCurrentTask() {
    if(task!=null) {
      if(!task.isDone()) task.cancel(true);
      task=null;
    }
  }


  private void applyRestrictions(long forDate) throws AdminException {
    System.out.println("applyRestictions");
    List<User> users  = userManager.getUsers();
    for(final User u : users) {
      final List<Restriction> restrictions = getActiveRestrictions(forDate,  u);
      DeliveryFilter dFilter = new DeliveryFilter();
      dFilter.setUserIdFilter(new String[]{u.getLogin()});
      dFilter.setStatusFilter(new DeliveryStatus[]{DeliveryStatus.Planned,DeliveryStatus.Active,DeliveryStatus.Paused});
      dFilter.setResultFields(new DeliveryFields[]{DeliveryFields.Status});
      deliveryManager.getDeliveries(u.getLogin(),u.getPassword(),dFilter,1000,
          new Visitor<DeliveryInfo>() {
            public boolean visit(DeliveryInfo di) throws AdminException {
              boolean shouldBeRestricted=false;
              for(Restriction r : restrictions) {
                if(r.isAllUsers() || r.getUserIds().contains(u.getLogin())) {
                  shouldBeRestricted = true;
                  break;
                }
              }
              adjustDeliveryState(u,di,shouldBeRestricted);
              return true;
            }
          });
    }
  }

  private List<Restriction> getActiveRestrictions(long startDate, User u) {
    RestrictionsFilter rFilter = new RestrictionsFilter();
    rFilter.setStartDate(new Date(startDate));
    rFilter.setEndDate(new Date(startDate+1));
    rFilter.setUserId(u.getLogin());
    return restrictionManager.getRestrictions(rFilter);
  }

  private void adjustDeliveryState(User u, DeliveryInfo di, boolean shouldBeRestricted) throws AdminException {
    System.out.println("adjust delivery "+di.getDeliveryId()+" "+di.getUserId()+" to restricted="+shouldBeRestricted);
    if(shouldBeRestricted) {
      if(di.getStatus()!=DeliveryStatus.Paused) {
        System.out.println("changed");
        deliveryManager.setDeliveryRestriction(u.getLogin(),u.getPassword(),di.getDeliveryId(),true);
        deliveryManager.pauseDelivery(u.getLogin(),u.getPassword(),di.getDeliveryId());
      }
    }
    else {
      if(di.getStatus()==DeliveryStatus.Paused && di.isRestriction()) {
        System.out.println("changed");
        deliveryManager.setDeliveryRestriction(u.getLogin(),u.getPassword(),di.getDeliveryId(),false);
        deliveryManager.activateDelivery(u.getLogin(),u.getPassword(),di.getDeliveryId());
      }
    }

  }

  public synchronized void stop() throws AdminException {
    cancelCurrentTask();
    task=null;
    scheduler.shutdown();
    try {
      // Wait a while for existing tasks to terminate
      if (!scheduler.awaitTermination(60, TimeUnit.SECONDS)) {
        scheduler.shutdownNow(); // Cancel currently executing tasks
        // Wait a while for tasks to respond to being cancelled
        if (!scheduler.awaitTermination(60, TimeUnit.SECONDS))
          System.err.println("Pool did not terminate");
      }
    }
    catch (InterruptedException ie) {
      scheduler.shutdownNow();
      Thread.currentThread().interrupt();
    }
    scheduler = null;
  }


  public boolean isStarted() {
    return scheduler!=null;
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

    public RestrictionTask(int num,long forDate) {
      this.forDate = forDate;
      this.num=num;
    }

    public Object call() throws Exception {
      applyRestrictions(forDate);
      nextSchedule();
      return null;
    }
  }
}
