package ru.sibinco.smsx.engine.service.sponsored;

import com.eyeline.sme.utils.ds.DataSourceException;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.ServiceInitializationException;
import ru.sibinco.smsx.engine.service.sponsored.commands.*;
import ru.sibinco.smsx.engine.service.sponsored.datasource.DistributionInfo;
import ru.sibinco.smsx.engine.service.sponsored.datasource.SponsoredDataSource;

import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;

/**
 * User: artem
 * Date: 29.06.2007
 */

class SponsoredProcessor implements SponsoredRegisterAbonentCmd.Receiver, SponsoredUnregisterAbonentCmd.Receiver,
                                    SponsoredRegisterDeliveryCmd.Receiver {

  private static final Category log = Category.getInstance("SPONSORED");

  private static final long DAY_COUNT = 1000 * 3600 * 24;

  // Properties
  private final int fromHour;
  private final int toHour;
  private final Date subscriptionStartDate;
  private final int subscriptionInterval;

  private final int totalWorkingTime;
  private final SponsoredEngine sponsoredEngine;
  private final SponsoredDataSource ds;
//  private final SmscConsoleClient smscConsoleClient;
  private final int[] allowedCounts;

  private boolean subscriptionLocked = false;

  SponsoredProcessor(SponsoredEngine sponsoredEngine, SponsoredDataSource ds, int fromHour, int toHour, Date subscriptionStartDate, int subscriptionInterval, int[] allowedCounts) {
    try {

      this.sponsoredEngine = sponsoredEngine;
      this.ds = ds;
//      this.smscConsoleClient = SmscConsoleClientFactory.getSmscConsoleClient();
//      smscConsoleClient.connect();
      this.fromHour = fromHour;
      this.toHour = toHour;
      this.totalWorkingTime = (toHour - fromHour) * 3600000;
      this.subscriptionStartDate = subscriptionStartDate;
      this.subscriptionInterval = subscriptionInterval;
      this.allowedCounts = allowedCounts;
      Arrays.sort(this.allowedCounts);

    } catch (Exception e) {
      throw new ServiceInitializationException(e);
    }
  }

  public void execute(SponsoredRegisterAbonentCmd cmd) {
    try {
      if (log.isInfoEnabled())
        log.info("Subscr req srcaddr=" + cmd.getAbonentAddress() + "; cnt=" + cmd.getCount());

      if (subscriptionLocked) {
        log.warn("Subscription locked");
        cmd.update(SponsoredRegisterAbonentCmd.STATUS_SUBSCRIPTION_LOCKED);
        return;
      }

      // Chack count
      int pos = Arrays.binarySearch(allowedCounts, cmd.getCount());
      if (pos < 0 || pos > allowedCounts.length || allowedCounts[pos] != cmd.getCount()) {
        cmd.update(SponsoredRegisterAbonentCmd.STATUS_INVALID_SUBSCRIPTION_COUNT);
        return;
      }

// DO NOT DELETE THIS ==================================================================================================
//      DistributionInfo subscription = ds.getDistributionInfoByAbonent(abonentAddress);
//      if (subscription == null) {
//        subscription = new DistributionInfo();
//        subscription.setAbonent(abonentAddress);
//      }
//
//      subscription.setCount(count);
//      subscription.setProfileChangeDate(getSubscriptionStartDate());
//
//      if (!subscription.isExists() && subscription.getProfileChangeDate().before(getNextDayStartTime(System.currentTimeMillis()))) {
//        subscription.setTodayCount(count);
//
//        final Calendar calendar = Calendar.getInstance();
//        calendar.setTime(new java.util.Date());
//        if (calendar.get(Calendar.HOUR_OF_DAY) > toHour)
//          calendar.setTimeInMillis(calendar.getTimeInMillis() + DAY_COUNT); // Add one day
//        calendar.set(Calendar.HOUR_OF_DAY, toHour);
//
//        subscription.setCurrentCount(count * (calendar.getTimeInMillis() - System.currentTimeMillis()) / totalWorkingTime);
//        if (subscription.getCurrentCount() > count)
//          subscription.setCurrentCount(count);
//        if (subscription.getCurrentCount() < 0)
//          subscription.setCurrentCount(0);
//
//        notification(abonentAddress, count);
//        sponsoredEngine.abonentSubscribed(count, subscription.getCurrentCount());
//      }
//
//      boolean exists = subscription.isExists();
//      ds.saveDistributionInfo(subscription);
// =====================================================================================================================

      final DistributionInfo info = getOrCreateSubscriptionInfo(cmd.getAbonentAddress());
      
      info.setCnt(cmd.getCount());
      ds.saveDistributionInfo(info);

      cmd.setAbonentExists(info.isExists());
      cmd.setProfileChangeDate(getNextDayStartTime(System.currentTimeMillis()));
      cmd.update(SponsoredRegisterAbonentCmd.STATUS_SUCCESS);

    } catch (Throwable e) {
      log.error("Subscr req error for " + cmd.getAbonentAddress(),e);
      cmd.update(SponsoredRegisterAbonentCmd.STATUS_SYSTEM_ERROR);
    }
  }

  public void execute(SponsoredUnregisterAbonentCmd cmd) {
    try {
      if (log.isInfoEnabled())
        log.info("Unsubscr req srcaddr=" + cmd.getAbonentAddress());

      DistributionInfo distribution = ds.getDistributionInfoByAbonent(cmd.getAbonentAddress());

      if (distribution != null) { // Abonent subscribed

        distribution.setCnt(0);
        ds.saveDistributionInfo(distribution);

        cmd.setProfileChangeDate(getNextDayStartTime(System.currentTimeMillis()));
        cmd.update(SponsoredUnregisterAbonentCmd.STATUS_SUCCESS);

      } else { // Abonent not subscribed

        cmd.update(SponsoredUnregisterAbonentCmd.STATUS_ABONENT_NOT_REGISTERED);
      }

    } catch (Throwable e) {
      log.error("Unsubscr req error for " + cmd.getAbonentAddress(),e);
      cmd.update(SponsoredUnregisterAbonentCmd.STATUS_SYSTEM_ERROR);
    }
  }

  private DistributionInfo getOrCreateSubscriptionInfo(String abonentAddress) throws DataSourceException {
    DistributionInfo info = ds.getDistributionInfoByAbonent(abonentAddress);
    if (info == null) {
      info = new DistributionInfo();
      info.setAbonent(abonentAddress);
    }
    return info;
  }

  private static Date getNextDayStartTime(long time) {
    final Calendar calendar = Calendar.getInstance();
    final Calendar dayStart = Calendar.getInstance();
    calendar.setTimeInMillis(time);
    dayStart.set(Calendar.DAY_OF_MONTH, calendar.get(Calendar.DAY_OF_MONTH));
    dayStart.set(Calendar.MONTH, calendar.get(Calendar.MONTH));
    dayStart.set(Calendar.YEAR, calendar.get(Calendar.YEAR));
    dayStart.set(Calendar.HOUR_OF_DAY, 0);
    dayStart.set(Calendar.MINUTE, 0);
    dayStart.set(Calendar.SECOND, 0);
    dayStart.set(Calendar.MILLISECOND, 0);
    return new Date(dayStart.getTimeInMillis() + DAY_COUNT);
  }

  public void lockSubscription() {
    subscriptionLocked = true;
    if (log.isInfoEnabled())
      log.info("Subscription locked");
  }

  public void unlockSubscription() {
    subscriptionLocked = false;
    if (log.isInfoEnabled())
      log.info("Subscription unlocked");
  }

  public void execute(SponsoredRegisterDeliveryCmd cmd) {
    try {
      ds.updateSponsoredStatus(cmd.getDestinationAddress());
      cmd.update(SponsoredRegisterDeliveryCmd.STATUS_SUCCESS);
    } catch (DataSourceException e) {
      log.error("Can't update sponsored status for abonent " + cmd.getDestinationAddress(), e);
      cmd.update(SponsoredRegisterDeliveryCmd.STATUS_SYSTEM_ERROR);
    }
  }

//  private void notification(String abonent,int NN) throws SmscConsoleClientException, IOException {
//    final String alterProfile = "alter profile "+abonent+" sponsored "+NN;
//
//    final SmscConsoleResponse resp = smscConsoleClient.sendCommand(alterProfile);
//
//    if (resp == null || !resp.isSuccess())
//      throw new IOException("Notification failed.Answer " + resp.getResponseLines()[resp.getResponseLines().length - 1] + " for " + abonent);
//
//    log.info("Finish notification successfully for " + abonent);
//  }

}
