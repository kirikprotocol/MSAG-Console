/**
 * User: artem
 * Date: Jul 1, 2007
 */

package ru.sibinco.smsx.engine.service;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.ConfigException;
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
import ru.sibinco.smsx.InitializationException;

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

  private CalendarService calendarService;
  private SecretService secretService;
  private final SenderService senderService;
  private final BlackListService blackListService;
  private final NickService nickService;
  private GroupService groupService;
  private final SubscriptionService subscriptionService;

  private ServiceManagerMBean mbean = null;



  private Services(XmlConfig config, OutgoingQueue outQueue) {
    try {
      System.out.println("  Init Sender...");
      senderService = new SenderServiceImpl(config, outQueue);

      if (config.containsSection("calendar")) {
        System.out.println("  Init Calendar...");
        calendarService = new CalendarServiceImpl(config.getSection("calendar"), outQueue);
      }

      if (config.containsSection("secret")) {
        System.out.println("  Init Secret...");
        secretService = new SecretServiceImpl(config.getSection("secret"), outQueue);
      }

      System.out.println("  Init Black List...");
      blackListService = new BlackListServiceImpl(config);

      System.out.println("  Init Nick...");
      nickService = new NickServiceImpl(config, outQueue);

      if (config.containsSection("group")) {
        System.out.println("  Init Group...");
        groupService = new GroupServiceImpl(config.getSection("group"), outQueue);
      }

      System.out.println("  Init Subscription...");
      subscriptionService = new SubscriptionServiceImpl(config);
    } catch (ConfigException e) {
      throw new InitializationException(e.getMessage(),e);
    }
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

    if (calendarService != null) {
      log.info("Starting: calendar...");
      calendarService.startService();
      log.info("Started: calendar.");
    }

    if (secretService != null) {
      log.info("Starting: secret...");
      secretService.startService();
      log.info("Started: secret.");
    }

    log.info("Starting: black list...");
    blackListService.startService();
    log.info("Started: black list.");

    log.info("Starting: nick...");
    nickService.startService();
    log.info("Started: nick.");

    if (groupService != null) {
      log.info("Starting: group...");
      groupService.startService();
      log.info("Started: group.");
    }

    log.info("Starting: subscription...");
    subscriptionService.startService();
    log.info("Started: subscription.");
  }

  public void stopServices() {
    log.info("Stopping: sender...");
    senderService.stopService();
    log.info("Stopped: sender.");

    if (calendarService != null) {
      log.info("Stopping: calendar...");
      calendarService.stopService();
      log.info("Stopped: calendar.");
    }

    if (secretService != null) {
      log.info("Stopping: secret...");
      secretService.stopService();
      log.info("Stopped: secret.");
    }

    log.info("Stopping: black list...");
    blackListService.stopService();
    log.info("Stopped: black list.");

    log.info("Stopping: nick...");
    nickService.stopService();
    log.info("Stopped: nick.");

    if (groupService != null) {
      log.info("Stopping: group...");
      groupService.stopService();
      log.info("Stopped: group.");
    }

    log.info("Stopping: subscription...");
    subscriptionService.stopService();
    log.info("Stopped: subscription.");
  }
}
