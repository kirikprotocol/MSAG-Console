/**
 * User: artem
 * Date: Jul 1, 2007
 */

package ru.sibinco.smsx.engine.service;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.blacklist.BlackListService;
import ru.sibinco.smsx.engine.service.blacklist.BlackListServiceImpl;
import ru.sibinco.smsx.engine.service.calendar.CalendarService;
import ru.sibinco.smsx.engine.service.calendar.CalendarServiceImpl;
import ru.sibinco.smsx.engine.service.nick.NickService;
import ru.sibinco.smsx.engine.service.nick.NickServiceImpl;
import ru.sibinco.smsx.engine.service.secret.SecretService;
import ru.sibinco.smsx.engine.service.secret.SecretServiceImpl;
import ru.sibinco.smsx.engine.service.sender.SenderService;
import ru.sibinco.smsx.engine.service.sender.SenderServiceImpl;
import ru.sibinco.smsx.engine.service.group.GroupService;
import ru.sibinco.smsx.engine.service.group.GroupServiceImpl;
import ru.sibinco.smsx.engine.service.subscription.SubscriptionService;
import ru.sibinco.smsx.engine.service.subscription.SubscriptionServiceImpl;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.ThreadFactoryWithCounter;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.ArrayBlockingQueue;

public class Services {
  private static final Category log = Category.getInstance("SERVICES");

  private static Services ourInstance;

  public static Services getInstance() {
    return ourInstance;
  }

  public static void init(XmlConfig config, OutgoingQueue outQueue) {
    ourInstance = new Services(config, outQueue);
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
    senderService = new SenderServiceImpl(config, outQueue);
    calendarService = new CalendarServiceImpl(config, outQueue);
    secretService = new SecretServiceImpl(config, outQueue);
    blackListService = new BlackListServiceImpl(config);
    nickService = new NickServiceImpl(config, outQueue);
    groupService = new GroupServiceImpl(config, outQueue);
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
