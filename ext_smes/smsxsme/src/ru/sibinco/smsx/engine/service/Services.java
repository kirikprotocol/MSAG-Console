/**
 * User: artem
 * Date: Jul 1, 2007
 */

package ru.sibinco.smsx.engine.service;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfig;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.blacklist.BlackListService;
import ru.sibinco.smsx.engine.service.blacklist.BlackListServiceImpl;
import ru.sibinco.smsx.engine.service.calendar.CalendarService;
import ru.sibinco.smsx.engine.service.calendar.CalendarServiceImpl;
import ru.sibinco.smsx.engine.service.group.GroupService;
import ru.sibinco.smsx.engine.service.group.GroupServiceImpl;
import ru.sibinco.smsx.engine.service.nick.NickService;
import ru.sibinco.smsx.engine.service.nick.NickServiceImpl;
import ru.sibinco.smsx.engine.service.secret.SecretService;
import ru.sibinco.smsx.engine.service.secret.SecretServiceImpl;
import ru.sibinco.smsx.engine.service.sender.SenderService;
import ru.sibinco.smsx.engine.service.sender.SenderServiceImpl;
import ru.sibinco.smsx.engine.service.subscription.SubscriptionService;
import ru.sibinco.smsx.engine.service.subscription.SubscriptionServiceImpl;

import java.util.concurrent.CountDownLatch;

public class Services {

  private static final Category log = Category.getInstance("SERVICES");

  private static Services ourInstance;

  private static final CountDownLatch initLatch = new CountDownLatch(1);

  public static Services getInstance() {
    try {
      initLatch.await();
    } catch (InterruptedException e) {
      return null;
    }
    return ourInstance;
  }

  public static void init(XmlConfig config, OutgoingQueue outQueue) {
    ourInstance = new Services(config, outQueue);
    initLatch.countDown();
  }

  private final CalendarService calendarService;
  private final SecretService secretService;
  private final SenderService senderService;
  private final BlackListService blackListService;
  private final NickService nickService;
  private final GroupService groupService;
  private final SubscriptionService subscriptionService;

  private ServiceManagerMBean mbean = null;



  private Services(XmlConfig config, OutgoingQueue outQueue) {
    System.out.println("  Init Sender...");
    senderService = new SenderServiceImpl(config, outQueue);
    System.out.println("  Init Calendar...");
    calendarService = new CalendarServiceImpl(config, outQueue);
    System.out.println("  Init Secret...");
    secretService = new SecretServiceImpl(config, outQueue);
    System.out.println("  Init Black List...");
    blackListService = new BlackListServiceImpl(config);
    System.out.println("  Init Nick...");
    nickService = new NickServiceImpl(config, outQueue);
    System.out.println("  Init Group...");
    groupService = new GroupServiceImpl(config, outQueue);
    System.out.println("  Init Subscription...");
    subscriptionService = new SubscriptionServiceImpl(config);
  }

  public ServiceManagerMBean getMBean(String domain) {
    if (mbean == null)
      mbean = new ServiceManagerMBean(domain);
    return mbean;
  }

  public CalendarService getCalendarService() {
    return calendarService;
  }

  public SecretService getSecretService() {
    return secretService;
  }

  public SenderService getSenderService() {
    return senderService;
  }

  public BlackListService getBlackListService() {
    return blackListService;
  }

  public NickService getNickService() {
    return nickService;
  }

  public GroupService getGroupService() {
    return groupService;
  }

  public SubscriptionService getSubscriptionService() {
    return subscriptionService;
  }

  public void startServices() {
    log.info("Starting: sender...");
    senderService.startService();
    log.info("Started: sender.");

    log.info("Starting: calendar...");
    calendarService.startService();
    log.info("Started: calendar.");

    log.info("Starting: secret...");
    secretService.startService();
    log.info("Started: secret.");

    log.info("Starting: black list...");
    blackListService.startService();
    log.info("Started: black list.");

    log.info("Starting: nick...");
    nickService.startService();
    log.info("Started: nick.");

    log.info("Starting: group...");
    groupService.startService();
    log.info("Started: group.");

    log.info("Starting: subscription...");
    subscriptionService.startService();
    log.info("Started: subscription.");
  }

  public void stopServices() {
    log.info("Stopping: sender...");
    senderService.stopService();
    log.info("Stopped: sender.");

    log.info("Stopping: calendar...");
    calendarService.stopService();
    log.info("Stopped: calendar.");

    log.info("Stopping: secret...");
    secretService.stopService();
    log.info("Stopped: secret.");

    log.info("Stopping: black list...");
    blackListService.stopService();
    log.info("Stopped: black list.");

    log.info("Stopping: nick...");
    nickService.stopService();
    log.info("Stopped: nick.");

    log.info("Stopping: group...");
    groupService.stopService();
    log.info("Stopped: group.");

    log.info("Stopping: subscription...");
    subscriptionService.stopService();
    log.info("Stopped: subscription.");
  }
}
