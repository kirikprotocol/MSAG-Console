package ru.sibinco.smsx.services.secret;

import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.services.InitializationInfo;
import ru.sibinco.smsx.services.InternalService;
import ru.sibinco.smsx.services.ProcessException;
import ru.sibinco.smsx.utils.BlockingQueue;
import ru.sibinco.smsx.utils.Utils;

/**
 * User: artem
 * Date: 24.01.2007
 */
public final class SecretService extends InternalService{

  private static final Category log = Category.getInstance(SecretService.class);

  private SecretRequestProcessor requestProcessor = null;
  private BlockingQueue messagesQueue = null;

  public SecretService() {
    super(log);
  }

  public boolean processMessage(Message message) throws ProcessException {
    if (message == null)
      return false;

    log.info("=====================================================================================");
    log.info("Processing message: from abonent = " + message.getSourceAddress() + ", to abonent = " + message.getDestinationAddress() + ", message = " + message.getMessageString());

    try {
      final SecretRequestParser.ParseResult res = SecretRequestParser.parseRequest(message.getMessageString());

      if (( message.getDestinationAddress().equals(Properties.SERVICE_ADDRESS) && res.getType().equals(SecretRequestParser.ParseResultType.MSG)) ||
          (!message.getDestinationAddress().equals(Properties.SERVICE_ADDRESS) && !res.getType().equals(SecretRequestParser.ParseResultType.MSG))) {
        log.info("Unknown message format");
        return false;
      }

      messagesQueue.put(new ParsedMessage(message, res));
      log.info("Message added into queue.");
      return true;

    } catch (SecretRequestParser.WrongMessageFormatException e) {
      log.info("Unknown message format");
      return false;
    } catch (SecretRequestParser.WrongPasswordException e) {
      log.error("Wrong password exception", e);
      sendResponse(message, Data.ESME_ROK);
      sendMessage(Properties.SERVICE_ADDRESS, message.getSourceAddress(), Properties.WRONG_PASSWORD);
      return true;
    } catch (Throwable e) {
      log.error("Error", e);
      throw new ProcessException(e);
    }
  }

  public void init(InitializationInfo info) {
    super.init(info);
    // Init properties
    Properties.init(serviceAddress);

    messagesQueue = new BlockingQueue(Properties.QUEUE_MAX_SIZE);

    // Start secret processor
    requestProcessor = new SecretRequestProcessor(messagesQueue, outQueue, multiplexor);
    requestProcessor.setDaemon(true);
    requestProcessor.startService();
    requestProcessor.waitStarting();
  }

  public String getName() {
    return "secret";
  }

  public static final class Properties {
    // SECRET ENGINE
    public static String SERVICE_ADDRESS;
    public static int QUEUE_MAX_SIZE;
    // MESSAGEs
    public static String REGISTER_OK_MESSAGE;
    public static String REGISTER_ERROR_MESSAGE;
    public static String UNREGISTER_OK_MESSAGE;
    public static String UNREGISTER_ERROR_MESSAGE;
    public static String INFORM_MESSAGE;
    public static String NO_MESSAGES;
    public static String DELIVERY_REPORT;
    public static String WRONG_PASSWORD;
    public static String DESTINATION_ABONENT_NOT_REGISTERED;
    public static String ABONENT_INVITATION;
    public static String ABONENT_NOT_REGISTERED;
    public static String SYSTEM_ERROR;
    public static String ABONENT_ALREADY_REGISTERED;
    public static String PASSWORD_CHANGED;

    private static void init(String serviceAddress) {
      final java.util.Properties config = Utils.loadConfig("secret/service.properties");
      config.setProperty("config.path", "secret/service.properties");

      SERVICE_ADDRESS = serviceAddress;
      QUEUE_MAX_SIZE = Utils.loadInt(config, "queue.max.size");

      REGISTER_OK_MESSAGE = Utils.loadString(config, "register.ok.message");
      REGISTER_ERROR_MESSAGE = Utils.loadString(config, "register.error.message");
      UNREGISTER_OK_MESSAGE = Utils.loadString(config, "unregister.ok.message");
      UNREGISTER_ERROR_MESSAGE = Utils.loadString(config, "unregister.error.message");
      INFORM_MESSAGE = Utils.loadString(config, "inform.message");
      NO_MESSAGES = Utils.loadString(config, "no.messages");
      DELIVERY_REPORT = Utils.loadString(config, "delivery.report");
      WRONG_PASSWORD = Utils.loadString(config, "wrong.password");
      DESTINATION_ABONENT_NOT_REGISTERED = Utils.loadString(config, "destination.abonent.not.registered");
      ABONENT_NOT_REGISTERED= Utils.loadString(config, "abonent.not.registered");
      SYSTEM_ERROR= Utils.loadString(config, "system.error");
      ABONENT_ALREADY_REGISTERED= Utils.loadString(config, "abonent.already.registered");
      PASSWORD_CHANGED= Utils.loadString(config, "password.changed");
      ABONENT_INVITATION = Utils.loadString(config, "abonent.invitation");

      config.clear();

      SecretMessage.init();
      SecretUser.init();
    }

    public static final class SecretMessage {
      public static String INSERT_MESSAGE_SQL;
      public static String SELECT_MESSAGE_SQL;
      public static String REMOVE_MESSAGE_SQL;

      private static void init() {
        final java.util.Properties config = Utils.loadConfig("secret/secretmessage.properties");
        config.setProperty("config.path", "secret/secretmessage.properties");
        INSERT_MESSAGE_SQL = Utils.loadString(config, "insert.sql");
        REMOVE_MESSAGE_SQL = Utils.loadString(config, "remove.sql");
        SELECT_MESSAGE_SQL = Utils.loadString(config, "select.by.user.sql");
        config.clear();
      }
    }

    public static final class SecretUser {
      public static String INSERT_SQL;
      public static String REMOVE_BY_NUMBER_SQL;
      public static String SELECT_BY_NUMBER_SQL;
      public static String UPDATE_PASSWORD_SQL;

      private static void init() {
        final java.util.Properties config = Utils.loadConfig("secret/secretuser.properties");
        config.setProperty("config.path", "secret/secretuser.properties");
        INSERT_SQL = Utils.loadString(config, "insert.sql");
        REMOVE_BY_NUMBER_SQL = Utils.loadString(config, "remove.sql");
        SELECT_BY_NUMBER_SQL = Utils.loadString(config, "select.by.number.sql");
        UPDATE_PASSWORD_SQL = Utils.loadString(config, "update.password.sql");
      }
    }
  }
}
