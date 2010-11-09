package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.Daemon;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManager;

import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
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
  Map<Integer,ScheduledFuture> startTasks;
  Map<Integer,ScheduledFuture> stopTasks;

  public RestrictionDaemon(DeliveryManager deliveryManager, RestrictionsManager restrictionManager, UsersManager userManager ) {
    this.deliveryManager=deliveryManager;
    this.restrictionManager = restrictionManager;
    this.userManager = userManager;

    startTasks = new HashMap<Integer,ScheduledFuture>();
    stopTasks = new HashMap<Integer,ScheduledFuture>();
  }

  public String getName() {
    return NAME;
  }

  public synchronized void start() throws AdminException {
    scheduler = Executors.newSingleThreadScheduledExecutor();
    cancelAllTasks();
    Date startDate = new Date();
    startTasks.put(0,scheduler.schedule(
        new RestrictionTask(STATUSES_FOR_BOTH_EVENTS),
        0L,
        TimeUnit.MILLISECONDS));

    RestrictionsFilter rFilter = new RestrictionsFilter();
    rFilter.setStartDate(startDate);
    final List<Restriction> restrictions = restrictionManager.getRestrictions(rFilter);
    for(Restriction r : restrictions) {
      createTasksForRestriction(startDate.getTime(), r);
    }
  }

  private void createTasksForRestriction(long time, Restriction r) {


    long startDelay = r.getStartDate().getTime() - time ;
    if(startDelay<0) startDelay=0;

    long endDelay  = r.getEndDate().getTime() - time;
    if(endDelay<0) endDelay=0;

    //schedule restriction start
    //create start task for this restriction only when stop is in the future
    if(endDelay>0) {
      startTasks.put(r.getId(),scheduler.schedule(
          new RestrictionTask(STATUSES_FOR_START_EVENT),
          startDelay,
          TimeUnit.MILLISECONDS));
    }
    //schedule restriction end
    stopTasks.put(r.getId(),scheduler.schedule(
        new RestrictionTask(STATUSES_FOR_STOP_EVENT),
        endDelay,
        TimeUnit.MILLISECONDS));
  }

  /**
   * Method to notify daemon about restrictions changes
   * @param oldRestrictionId - id of updated or deleted restriction or null when new added
   * @param newRestriction - updated or added restriction, or null when deleted
   */
  public synchronized void restrictionChanged(Integer oldRestrictionId, Restriction newRestriction) {
    if(isStarted()) {
      if(oldRestrictionId!=null) {
        ScheduledFuture task = startTasks.get(oldRestrictionId);
        if(task!=null){
          if(!task.isDone()) task.cancel(false);
          startTasks.remove(oldRestrictionId);
        }

        task = stopTasks.get(oldRestrictionId);
        if(task!=null) {
          if(!task.isDone()) task.cancel(false);
          stopTasks.remove(oldRestrictionId);
        }
      }

      if(newRestriction!=null) {
        createTasksForRestriction(System.currentTimeMillis(), newRestriction);
      }
      else { //restriction was removed adjust only paused
        stopTasks.put(oldRestrictionId,scheduler.schedule(
            new RestrictionTask( STATUSES_FOR_STOP_EVENT),
            0L,
            TimeUnit.MILLISECONDS));
      }
    }
  }


  private void applyRestrictions(DeliveryStatus[] deliveryStatuses) throws AdminException {

    List<User> users  = userManager.getUsers();
    for(final User u : users) {
      final List<Restriction> restrictions = getActiveRestrictions(new Date(),  u);
      DeliveryFilter dFilter = new DeliveryFilter();
      dFilter.setUserIdFilter(new String[]{u.getLogin()});
      dFilter.setStatusFilter(deliveryStatuses);
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

  private List<Restriction> getActiveRestrictions(Date startDate, User u) {
    RestrictionsFilter rFilter = new RestrictionsFilter();
    rFilter.setStartDate(startDate);
    rFilter.setEndDate(startDate);
    rFilter.setUserId(u.getLogin());
    return restrictionManager.getRestrictions(rFilter);
  }

  private void adjustDeliveryState(User u, DeliveryInfo di, boolean shouldBeRestricted) throws AdminException {

    if(shouldBeRestricted) {
      if(di.getStatus()!=DeliveryStatus.Paused) {
        deliveryManager.setDeliveryRestriction(u.getLogin(),u.getPassword(),di.getDeliveryId(),true);
        deliveryManager.pauseDelivery(u.getLogin(),u.getPassword(),di.getDeliveryId());
      }
    }
    else {
      if(di.getStatus()==DeliveryStatus.Paused && di.isRestriction()) {
        deliveryManager.setDeliveryRestriction(u.getLogin(),u.getPassword(),di.getDeliveryId(),false);
        deliveryManager.activateDelivery(u.getLogin(),u.getPassword(),di.getDeliveryId());
      }
    }

  }

  public synchronized void stop() throws AdminException {
    cancelAllTasks();
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

  private void cancelAllTasks() {
    for( ScheduledFuture task: startTasks.values()) {
      if(!task.isDone()) task.cancel(false);
    }
    startTasks.clear();
    for( ScheduledFuture task: stopTasks.values()) {
      if(!task.isDone()) task.cancel(false);
    }
    stopTasks.clear();
  }

  public boolean isStarted() {
    return scheduler!=null;
  }

  class RestrictionTask implements Callable<Object> {
    DeliveryStatus[] statuses;


    public RestrictionTask(DeliveryStatus[] statuses) {
      this.statuses = statuses;
    }

    public Object call() throws Exception {
      applyRestrictions(statuses);      
      return null;

    }
  }


}
