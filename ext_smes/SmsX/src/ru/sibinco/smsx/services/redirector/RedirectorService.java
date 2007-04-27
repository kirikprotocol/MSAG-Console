package ru.sibinco.smsx.services.redirector;

import com.logica.smpp.Data;
import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Message;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.services.InitializationInfo;
import ru.sibinco.smsx.services.InternalService;
import ru.sibinco.smsx.services.ProcessException;
import ru.sibinco.smsx.services.redirector.redirects.Redirects;
import ru.sibinco.smsx.services.redirector.redirects.Redirect;
import ru.sibinco.smsx.utils.BlockingQueue;
import ru.sibinco.smsx.utils.Utils;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * User: artem
 * Date: 24.01.2007
 */
public class RedirectorService extends InternalService {

  private static final Category log = Category.getInstance(RedirectorService.class);

  private BlockingQueue messagesQueue;

  public RedirectorService() {
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

      final Redirect redirect = Redirects.getRedirectByMessage(message.getMessageString());
      if (redirect == null) {
        log.info("Unknown message format");
        return false;
      }

      log.info("Message format ok. Redirect message to address " + redirect.getAddress());
      messagesQueue.put(new ParsedMessage(message, redirect));

      return true;

    } catch (BlockingQueue.QueueOverflowException e) {
      sendResponse(message, Data.ESME_RX_P_APPN);
      sendMessage(Properties.SERVICE_ADDRESS, message.getSourceAddress(), Properties.SERVICE_BUSY);
      return true;
    } catch (Throwable e) {
      log.error("Error", e);
      throw new ProcessException(e);
    }
  }

  public void init(InitializationInfo info) throws InitializationException {
    super.init(info);
    // Init properties
    Properties.init(serviceAddress);

    messagesQueue = new BlockingQueue(Properties.QUEUE_MAX_SIZE);

    final RedirectorRequestProcessor requestProcessor = new RedirectorRequestProcessor(messagesQueue, outQueue, multiplexor);
    requestProcessor.setDaemon(true);
    requestProcessor.startService();
    requestProcessor.waitStarting();
  }

  public String getName() {
    return "redirector";
  }

  public static final class Properties {
    public static String SERVICE_ADDRESS;
    public static String REDIRECTS_XML;
    public static int QUEUE_MAX_SIZE;
    public static String SERVICE_BUSY;
    public static int RELOAD_REDIRECTS_INTERVAL;

    private static void init(String serviceAddress){
      final java.util.Properties config = Utils.loadConfig("redirector/service.properties");
      config.setProperty("config.path", "redirector/service.properties");
      SERVICE_ADDRESS = serviceAddress;
      REDIRECTS_XML = Utils.loadString(config,"redirects.xml");
      QUEUE_MAX_SIZE = Utils.loadInt(config, "queue.max.size");
      SERVICE_BUSY = Utils.loadString(config, "engine.service.busy.message");
      RELOAD_REDIRECTS_INTERVAL = Utils.loadInt(config, "reload.redirects.interval");
      config.clear();

      Redirects.init();
    }
  }
}
