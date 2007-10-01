package ru.sibinco.smsx.engine;

import com.eyeline.sme.utils.worker.IterativeWorker;
import org.apache.log4j.Category;
import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;
import ru.sibinco.smsx.InitializationException;
import ru.sibinco.smsx.engine.service.ServiceManager;
import ru.sibinco.smsx.engine.smpphandler.SMPPHandler;
import ru.sibinco.smsx.engine.smpphandler.SMPPHandlerFactory;
import ru.sibinco.smsx.engine.soaphandler.blacklist.BlacklistSoapFactory;
import ru.sibinco.smsx.engine.soaphandler.smsxsender.SmsXSenderFactory;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;
import ru.sibinco.smsx.network.advertising.AdvertisingClientFactory;
import ru.sibinco.smsx.network.smppnetwork.SMPPMultiplexor;
import ru.sibinco.smsx.utils.operators.OperatorsList;

import java.io.File;

/**
 * User: artem
 * Date: 03.07.2007
 */

public class SmeEngine {

  private static final Category log = Category.getInstance("SME ENGINE");

  private final SMPPWorker smppWorker;

  private final SmscTimezonesList timezonesList;
  private final OperatorsList operatorsList;
  private final AdvertisingClient senderAdvertisingClient;

  private final IterativeWorker configsReloader;


  public SmeEngine(String configDir, SMPPMultiplexor multiplexor) {
    try {
      // Init lists
      timezonesList = new SmscTimezonesList(new File(configDir, "timezones.xml").getAbsolutePath(), new File(configDir, "routes.xml").getAbsolutePath());
      operatorsList = new OperatorsList(new File(configDir, "operators.xml").getAbsolutePath());

      // Init configs reloader
      configsReloader = new IterativeWorker(log, 600000) {
        protected void iterativeWork() {
          try {
            timezonesList.reload();
          } catch (Throwable e) {
            log.error("Can't reload timezones list", e);
          }

          operatorsList.reloadOperators();
        }

        protected void stopCurrentWork() {
        }
      };
      configsReloader.start();

      // Init services
      ServiceManager.init(configDir, multiplexor.getOutQueue());
      ServiceManager.getInstance().startServices();

      // Init SMPP
      final SMPPHandler smppHandler = SMPPHandlerFactory.createSMPPHandler(configDir, timezonesList, multiplexor);
      smppWorker = new SMPPWorker(multiplexor.getInQueue(), smppHandler);
      smppWorker.start();

      // Init SOAP
      BlacklistSoapFactory.init(configDir);
      senderAdvertisingClient = AdvertisingClientFactory.createAdvertisingClient();
      senderAdvertisingClient.connect();
      SmsXSenderFactory.init(configDir, operatorsList, senderAdvertisingClient);

    } catch (Throwable e) {
      throw new InitializationException(e);
    }
  }

  public void release() {
    log.info("Stopping services...");
    ServiceManager.getInstance().stopServices();
    log.info("Services stopped.");

    log.info("Stopping smpp worker...");
    smppWorker.stop();
    log.info("Smpp worker stopped.");

    log.info("Close sender advertising client...");
    senderAdvertisingClient.close();
    log.info("Sender advertising client closed.");

    log.info("Stopping reload config executor...");
    configsReloader.stop();
    log.info("Reload config executor stopped.");
  }

}
