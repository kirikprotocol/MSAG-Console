package ru.sibinco.smsx.services.sponsored;

import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.services.InitializationInfo;
import ru.sibinco.smsx.services.InternalService;
import ru.sibinco.smsx.services.ProcessException;
import ru.sibinco.smsx.utils.BlockingQueue;
import ru.sibinco.smsx.utils.Utils;

/**
 * User: artem
 * Date: 24.01.2007
 */
public class SponsoredService extends InternalService{

  private static final Category log = Category.getInstance(SponsoredService.class);

  private BlockingQueue messagesQueue;

  private static final String regex = "(S|s)(P|p)(O|o)(N|n)(S|s)(O|o)(R|r)(E|e)(D|d) \\d?\\d";


  public SponsoredService() {
    super(log);
  }

  public boolean processMessage(Message message) throws ProcessException {

    try {
      log.info("=====================================================================================");
      log.info("Processing message: from abonent = " + message.getSourceAddress() + ", to abonent = " + message.getDestinationAddress() + ", message = " + message.getMessageString());


      if (!message.getDestinationAddress().equals(Properties.SERVICE_ADDRESS)) {
        log.info("Unknown destination address");
        return false;
      }

      if (!message.getMessageString().matches(regex)) {
        log.info("Unknown message format");
        return false;
      }

      messagesQueue.put(message);

      return true;
    } catch (BlockingQueue.QueueOverflowException e) {
      sendResponse(message, Data.ESME_RX_P_APPN);
      sendMessage(Properties.SERVICE_ADDRESS, message.getSourceAddress(), Properties.Subscription.SERVICE_BUSY);
      return true;
    } catch (Throwable e) {
      throw new ProcessException(e);
    }
  }

  public void init(InitializationInfo info) throws InitializationException {
    super.init(info);
    // Init properties
    Properties.init(serviceAddress);

    final SponsoredEngine sponsoredEngine = new SponsoredEngine(outQueue);
    sponsoredEngine.setDaemon(true);
    sponsoredEngine.startService();
    sponsoredEngine.waitStarting();

    messagesQueue = new BlockingQueue(Properties.QUEUE_MAX_SIZE);

    final SponsoredRequestProcessor requestProcessor = new SponsoredRequestProcessor(messagesQueue, outQueue, multiplexor);
    requestProcessor.setDaemon(true);
    requestProcessor.startService();
    requestProcessor.waitStarting();
  }

  public String getName() {
    return "sponsored";
  }

  protected static final class Properties {

    public static String SERVICE_ADDRESS;
    public static int QUEUE_MAX_SIZE;

    private static void init(String serviceAddress) {
      Subscription.init();
      Engine.init();

      final java.util.Properties config = Utils.loadConfig("sponsored/service.properties");
      config.setProperty("config.path", "sponsored/service.properties");
      SERVICE_ADDRESS = serviceAddress;
      QUEUE_MAX_SIZE = Utils.loadInt(config, "queue.max.size");
      config.clear();
    }

    public static final class Subscription {
      public static String NOTIFICATION_HOST;
      public static int NOTIFICATION_PORT;
      public static String NOTIFICATION_USER;
      public static String NOTIFICATION_PASSWORD;
      public static String SUCCESSFULLY_MESSAGE;
      public static String SQL_INSERT_ABONENT;
      public static String SQL_UPDATE_ABONENT;
      public static String SQL_GET_ABONENT;
      public static String SERVICE_BUSY;

      private static void init() {
        final String configFileName = "sponsored/service.properties";
        final java.util.Properties config = Utils.loadConfig(configFileName);
        config.setProperty("config.path", configFileName);

        NOTIFICATION_HOST=Utils.loadString(config, "subscription.notification.host");

        NOTIFICATION_PORT= Utils.loadInt(config, "subscription.notification.port");

        NOTIFICATION_USER=Utils.loadString(config, "subscription.notification.user");

        NOTIFICATION_PASSWORD=Utils.loadString(config, "subscription.notification.password");

        SUCCESSFULLY_MESSAGE=Utils.loadString(config, "subscription.service.message.successully");

        SQL_INSERT_ABONENT=Utils.loadString(config, "subscription.sql.insert.abonent");

        SQL_UPDATE_ABONENT=Utils.loadString(config, "subscription.sql.update.abonent");

        SQL_GET_ABONENT=Utils.loadString(config, "subscription.sql.get.abonent");

        SERVICE_BUSY = Utils.loadString(config, "engine.service.busy.message");
        config.clear();
      }
    }

    protected static final class Engine {

      public static long REITERATION_INTERVAL;
      public static long SEND_INTERVAL;

      // queries
      public static String PORTION_QUERY;
      public static String UPDATE_DELIVERY_STATUS_QUERY;
      public static String INSERT_STATS_QUERY;
      public static String UPDATE_STATS_STATUS_QUERY;
      public static String DELETE_DELIVERY_QUERY;

      public static int FROM_HOUR;
      public static int TO_HOUR;

      public static int DELIVERY_PORTION;

      private static void init() {
        final String configFileName = "sponsored/service.properties";
        final java.util.Properties config = Utils.loadConfig(configFileName);
        config.setProperty("config.path", configFileName);

        REITERATION_INTERVAL = Utils.loadLong(config, "engine.reiteration.interval") < 60000L ? 60000L : Utils.loadLong(config, "engine.reiteration.interval");
        SEND_INTERVAL = Utils.loadLong(config, "engine.send.interval") < 60000L ? 60000L : Utils.loadLong(config, "engine.reiteration.interval");
        PORTION_QUERY = Utils.loadString(config, "engine.portion.query");
        UPDATE_DELIVERY_STATUS_QUERY = Utils.loadString(config, "engine.update.delivery.status.query");
        INSERT_STATS_QUERY = Utils.loadString(config, "engine.insert.stats.query");
        UPDATE_STATS_STATUS_QUERY = Utils.loadString(config, "engine.update.stats.status.query");
        DELETE_DELIVERY_QUERY = Utils.loadString(config, "engine.delete.delivery.query");
        FROM_HOUR = Utils.loadInt(config, "engine.from.hour");
        TO_HOUR = Utils.loadInt(config, "engine.to.hour");
        DELIVERY_PORTION = Utils.loadInt(config, "engine.delivery.portion");

        config.clear();
      }
    }
  }
}
