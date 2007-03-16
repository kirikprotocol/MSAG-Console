package ru.sibinco.smsx.services.sponsored;

import ru.sibinco.smsx.services.InternalService;
import ru.sibinco.smsx.services.ProcessException;
import ru.sibinco.smsx.services.InitializationInfo;
import ru.sibinco.smsx.utils.BlockingQueue;
import ru.sibinco.smsx.utils.Utils;
import ru.sibinco.smsx.InitializationException;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;
import com.logica.smpp.Data;

import java.util.StringTokenizer;

/**
 * User: artem
 * Date: 13.03.2007
 */

public class SponsoredServiceNew extends InternalService{

  private static final Category log = Category.getInstance(SponsoredServiceNew.class);

  private BlockingQueue messagesQueue;



  public SponsoredServiceNew() {
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

      final ParsedMessage msg = SponsoredRequestParser.parseRequest(message);

      if (msg == null) {
        log.info("Unknown message format");
        return false;
      }

      messagesQueue.put(msg);

      return true;
    } catch (BlockingQueue.QueueOverflowException e) {
      sendResponse(message, Data.ESME_RX_P_APPN);
      sendMessage(Properties.SERVICE_ADDRESS, message.getSourceAddress(), Properties.Subscription.SERVICE_BUSY);
      return true;

    } catch (SponsoredRequestParser.WrongSubscriptionCountException e) {
      sendResponse(message, Data.ESME_ROK);
      sendMessage(Properties.SERVICE_ADDRESS, message.getSourceAddress(), Properties.Subscription.WRONG_FORMAT_MESSAGE);
      return true;

    } catch (Throwable e) {
      throw new ProcessException(e);
    }
  }

  public void init(InitializationInfo info) throws InitializationException {
    super.init(info);
    // Init properties
    SponsoredServiceNew.Properties.init(serviceAddress);

    final SponsoredEngineNew sponsoredEngine = new SponsoredEngineNew(outQueue);
//    sponsoredEngine.setDaemon(true);
//    sponsoredEngine.startService();
//    sponsoredEngine.waitStarting();

    messagesQueue = new BlockingQueue(Properties.QUEUE_MAX_SIZE);

    final SponsoredRequestProcessorNew requestProcessor = new SponsoredRequestProcessorNew(messagesQueue, outQueue, multiplexor, sponsoredEngine);
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
    public static int[] ALLOWED_COUNTS;


    private static void init(String serviceAddress) {
      SponsoredServiceNew.Properties.Subscription.init();
      SponsoredServiceNew.Properties.Engine.init();

      final java.util.Properties config = Utils.loadConfig("sponsored/service.properties");
      config.setProperty("config.path", "sponsored/service.properties");
      SERVICE_ADDRESS = serviceAddress;
      QUEUE_MAX_SIZE = Utils.loadInt(config, "queue.max.size");

      try {
        final StringTokenizer allowedCounts = new StringTokenizer(Utils.loadString(config, "allowed.counts"), ",");
        ALLOWED_COUNTS = new int[allowedCounts.countTokens()];
        for (int i=0; i< ALLOWED_COUNTS.length; i++)
          ALLOWED_COUNTS[i] = Integer.parseInt(allowedCounts.nextToken().trim());
      } catch (Throwable e) {
        throw new InitializationException(e);
      }
      config.clear();

    }

    public static final class Subscription {
      public static String NOTIFICATION_HOST;
      public static int NOTIFICATION_PORT;
      public static String NOTIFICATION_USER;
      public static String NOTIFICATION_PASSWORD;
      public static String SUCCESSFULLY_MESSAGE;
      public static String SUCCESSFULLY_MESSAGE_TOMORROW;
      public static String WRONG_FORMAT_MESSAGE;
      public static String SUCCESSFULLY_MESSAGE_OFF;
      public static String NOT_SUBSCRIBED_MESSAGE;
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

        SUCCESSFULLY_MESSAGE_TOMORROW=Utils.loadString(config, "subscription.service.message.successully.tomorrow");

        WRONG_FORMAT_MESSAGE=Utils.loadString(config, "subscription.service.message.wrong.format");

        SUCCESSFULLY_MESSAGE_OFF=Utils.loadString(config, "subscription.service.message.successully.off");

        NOT_SUBSCRIBED_MESSAGE=Utils.loadString(config, "subscription.service.message.not.subscribed");

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

      public static String SELECT_ABONENTS_QUERY;
      public static String SELECT_TOTAL_MESSAGES;
      public static String SELECT_TOTAL_ABONENTS;
      public static String DECREASE_MESSAGES_COUNT;

      public static int FROM_HOUR;
      public static int TO_HOUR;

      public static int DELIVERY_PORTION;

      public static java.util.Properties ADVERTISING_PROPERTIES;
      public static String ADVERTISING_SERVICE_NAME;

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
        SELECT_ABONENTS_QUERY = Utils.loadString(config, "engine.sql.get.abonents");
        SELECT_TOTAL_MESSAGES = Utils.loadString(config, "engine.sql.get.total.messages");
        SELECT_TOTAL_ABONENTS = Utils.loadString(config, "engine.sql.get.total.abonents");
        DECREASE_MESSAGES_COUNT = Utils.loadString(config, "engine.sql.decrease.messages.count");

        config.clear();

        ADVERTISING_PROPERTIES = Utils.loadConfig("sponsored/advertising.properties");
        ADVERTISING_SERVICE_NAME = Utils.loadString(ADVERTISING_PROPERTIES, "service.name");
      }
    }
  }
}
