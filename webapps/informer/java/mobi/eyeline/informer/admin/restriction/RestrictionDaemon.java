package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.users.User;
import org.apache.log4j.Logger;

import java.util.*;
import java.util.concurrent.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 08.11.2010
 * Time: 16:35:50
 */
class RestrictionDaemon {

  private static final Logger log = Logger.getLogger("RestrictionDaemon");

  private RestrictionContext context;
  private final RestrictionsManager manager;

  private ScheduledExecutorService scheduler = null;
  private ScheduledFuture task;

  private Map<String, Boolean> usersRestrictions = new HashMap<String, Boolean>();

  public RestrictionDaemon(RestrictionContext context, RestrictionsManager manager) {
    this.context = context;
    this.manager = manager;
  }

  public synchronized void start() throws AdminException {
    if (!isStarted()) {
      scheduler = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
        public Thread newThread(Runnable runnable) {
          return new Thread(runnable, "RestrictionDaemon");
        }
      });
      rebuildSchedule();
    }
  }

  public synchronized void rebuildSchedule() {
    cancelCurrentTask();
    nextSchedule();
  }

  private synchronized void nextSchedule() {
    log.debug("next schedule<-");
    if (isStarted()) {
      task = scheduler.scheduleAtFixedRate(new RestrictionTask(), 0, 60000, TimeUnit.MILLISECONDS);
    }
    log.debug("next schedule->");
  }

  private void cancelCurrentTask() {
    if (task != null) {
      task.cancel(false);
      task = null;
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
          log.error("Unable to shutdown RestrictionDaemon in 60 sec");
        }
      }
      catch (InterruptedException e) {
        log.error("Unable to shutdown RestrictionDaemon", e);
      }
    }
  }


  public boolean isStarted() {
    return scheduler != null;
  }


  private class RestrictionTask implements Runnable {

    private boolean isShouldBeRestricted(User u, long forDate) {
      if (u.getStatus() == User.Status.DISABLED)
        return true;

      final List<Restriction> restrictions = getActiveRestrictions(forDate, u);
      for (Restriction r : restrictions) {
        if (r.isAllUsers() || (r.getUserIds()!=null && r.getUserIds().contains(u.getLogin()))) {
          log.debug("should be restricted r.allUsers="+r.isAllUsers()+" userIds="+r.getUserIds());
          return true;
        }
      }

      return false;
    }

    private void applyRestrictions(long forDate) throws AdminException {
      log.debug("apply restrictions<-");
      List<User> users = context.getUsers();
      for (final User u : users) {

        final boolean shouldBeRestricted = isShouldBeRestricted(u, forDate);
        Boolean currentRestriction = usersRestrictions.get(u.getLogin());
        if (currentRestriction == null || currentRestriction != shouldBeRestricted) {

          usersRestrictions.remove(u.getLogin());

          DeliveryFilter dFilter = new DeliveryFilter();
          dFilter.setUserIdFilter(u.getLogin());
          dFilter.setStatusFilter(DeliveryStatus.Planned, DeliveryStatus.Active, DeliveryStatus.Paused);

          log.debug("get deliveries for user = "+u.getLogin());
          final Collection<Delivery> deliveries = new ArrayList<Delivery>();

          context.getDeliveries(u.getLogin(), dFilter,
              new Visitor<Delivery>() {
                public boolean visit(Delivery di) throws AdminException {
                  deliveries.add(di);
                  return true;
                }
              });

          for (Delivery di : deliveries) {
            adjustDeliveryState(u, di, shouldBeRestricted);
          }

          usersRestrictions.put(u.getLogin(), shouldBeRestricted);
        }
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

    public void run()  {
      try {
        applyRestrictions(System.currentTimeMillis());
      } catch (Exception e) {
        log.error("Error applying restrictions", e);
      }
    }
  }
}
