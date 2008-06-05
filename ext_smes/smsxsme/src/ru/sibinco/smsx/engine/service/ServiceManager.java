/**
 * User: artem
 * Date: Jul 1, 2007
 */

package ru.sibinco.smsx.engine.service;

import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.service.blacklist.BlackListService;
import ru.sibinco.smsx.engine.service.blacklist.BlackListServiceFactory;
import ru.sibinco.smsx.engine.service.calendar.CalendarService;
import ru.sibinco.smsx.engine.service.calendar.CalendarServiceFactory;
import ru.sibinco.smsx.engine.service.nick.NickService;
import ru.sibinco.smsx.engine.service.nick.NickServiceFactory;
import ru.sibinco.smsx.engine.service.secret.SecretService;
import ru.sibinco.smsx.engine.service.secret.SecretServiceFactory;
import ru.sibinco.smsx.engine.service.sender.SenderService;
import ru.sibinco.smsx.engine.service.sender.SenderServiceFactory;
import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfig;

public class ServiceManager {
  private static final Category log = Category.getInstance("SERVICES");

  private static ServiceManager ourInstance;

  public static ServiceManager getInstance() {
    return ourInstance;
  }

  public static void init(XmlConfig config, OutgoingQueue outQueue) {
    ourInstance = new ServiceManager(config, outQueue);
  }

  private final CalendarService calendarService;
  private final SecretService secretService;
  private final SenderService senderService;
  private final BlackListService blackListService;
  private final NickService nickService;

  private ServiceManagerMBean mbean = null;

  private ServiceManager(XmlConfig config, OutgoingQueue outQueue) {
    senderService = SenderServiceFactory.getSenderService(config, outQueue);
    calendarService = CalendarServiceFactory.getCalendarService(config, outQueue);
    secretService = SecretServiceFactory.getSecretService(config, outQueue);
    blackListService = BlackListServiceFactory.getBlackListService(config);
    nickService = NickServiceFactory.getNickService(config, outQueue);
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
  }
}
