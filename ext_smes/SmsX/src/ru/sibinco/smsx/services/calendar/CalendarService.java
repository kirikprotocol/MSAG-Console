package ru.sibinco.smsx.services.calendar;

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

public final class CalendarService extends InternalService {

  private static final Category log = Category.getInstance(CalendarService.class);

  private CalendarEngine calendarEngine = null;
  private CalendarRequestProcessor requestProcessor = null;
  private BlockingQueue messagesQueue;

  public CalendarService() {
    super(log);
  }

  public boolean processMessage(Message message) throws ProcessException {
    if (message == null)
      return false;

    log.info("=====================================================================================");
    log.info("Processing message: from abonent = " + message.getSourceAddress() + ", to abonent = " + message.getDestinationAddress() + ", message = " + message.getMessageString());

    try {
      final CalendarRequestParser.ParseResult parseResult = CalendarRequestParser.parseRequest(message.getMessageString());
      messagesQueue.put(new ParsedMessage(message, parseResult));
      log.info("Message added into queue.");
      return true;

    } catch (CalendarRequestParser.WrongMessageFormatException e) {
      log.info("Unknown message format");
      return false;

    } catch (CalendarRequestParser.WrongSendDateException e) {
      log.info("Send date in message is wrong. Notify abonent");
      sendResponse(message, Data.ESME_RX_P_APPN);
      sendMessage(message.getDestinationAddress(), message.getSourceAddress(), CalendarService.Properties.CALENDAR_SEND_DATE_IS_WRONG);
      return true;

    } catch (Throwable e) {
      log.error("Error!!!", e);
      throw new ProcessException(e);
    }
  }

  public void init(final InitializationInfo info) throws InitializationException {
    super.init(info);

    // Init properties
    Properties.init(serviceAddress);

    final CalendarMessagesList calendarMessagesList = new CalendarMessagesList(Properties.CALENDAR_MESSAGES_LIST_MAX_SIZE);
    this.messagesQueue = new BlockingQueue(Properties.CALENDAR_MESSAGES_LIST_MAX_SIZE);

    // Start calendar engine
    calendarEngine = new CalendarEngine(outQueue, calendarMessagesList);
    calendarEngine.setDaemon(true);
    calendarEngine.startService();
    calendarEngine.waitStarting();

    // Start processor
    requestProcessor = new CalendarRequestProcessor(messagesQueue, outQueue, calendarMessagesList, multiplexor);
    requestProcessor.setDaemon(true);
    requestProcessor.startService();
    requestProcessor.waitStarting();
  }

  public String getName() {
    return "calendar";
  }


  public static final class Properties {
    // CALENDAR ENGINE
    public static String SERVICE_ADDRESS;
    public static String ERROR_MESSAGE;
    public static long CALENDAR_ENGINE_WORKING_INTERVAL;
    public static String CALENDAR_SEND_DATE_IS_IN_THE_PAST;
    public static String CALENDAR_SEND_DATE_IS_WRONG;
    public static int CALENDAR_SEND_DATE_MAX_YEAR;
    // SQL
    public static String CALENDAR_ENGINE_LOAD_LIST_SQL;
    public static String CALENDAR_ENGINE_REMOVE_MESSAGE_SQL;
    public static String CALENDAR_ENGINE_INSERT_MESSAGE_SQL;

    // CALENDAR MESSAGES LIST
    public static int CALENDAR_MESSAGES_LIST_MAX_SIZE;

    private static void init(String serviceAddress) {
      final java.util.Properties config = Utils.loadConfig("calendar/service.properties");
      config.setProperty("config.path", "calendar/service.properties");
      SERVICE_ADDRESS = serviceAddress;
      ERROR_MESSAGE = Utils.loadString(config, "error.message");
      CALENDAR_ENGINE_WORKING_INTERVAL = Utils.loadInt(config, "working.interval");
      CALENDAR_SEND_DATE_IS_IN_THE_PAST = Utils.loadString(config, "send.date.is.in.the.past");
      CALENDAR_SEND_DATE_IS_WRONG = Utils.loadString(config, "send.date.is.wrong");
      CALENDAR_SEND_DATE_MAX_YEAR = Utils.loadInt(config, "send.date.max.year");
      CALENDAR_ENGINE_LOAD_LIST_SQL = Utils.loadString(config, "load.list.sql");
      CALENDAR_ENGINE_REMOVE_MESSAGE_SQL = Utils.loadString(config, "remove.message.sql");
      CALENDAR_ENGINE_INSERT_MESSAGE_SQL = Utils.loadString(config, "insert.message.sql");
      CALENDAR_MESSAGES_LIST_MAX_SIZE = Utils.loadInt(config, "messages.list.max.size");
      config.clear();

      Timezones.init();
      ru.sibinco.smsx.services.calendar.timezones.Timezones.init();
    }

    public static final class Timezones {
      public static String TIMEZONES_XML;
      public static String ROUTES_XML;
      public static int RELOAD_TIMEZONES_INTERVAL;

      private static void init() {
        final java.util.Properties config = Utils.loadConfig("calendar/service.properties");
        config.setProperty("config.path", "service.properties");

        TIMEZONES_XML = Utils.loadString(config, "timezones.xml");
        ROUTES_XML = Utils.loadString(config, "routes.xml");
        RELOAD_TIMEZONES_INTERVAL = Utils.loadInt(config, "reload.timezones.interval");

        config.clear();
      }
    }
  }
}
