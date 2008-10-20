package mobi.eyeline.mcahdb;

import java.util.Date;
import java.util.Collection;
import java.io.File;

import mobi.eyeline.mcahdb.engine.event.EventStore;
import mobi.eyeline.mcahdb.engine.event.ds.Event;
import mobi.eyeline.mcahdb.engine.journal.JournalsProcessor;
import mobi.eyeline.mcahdb.engine.InitException;
import mobi.eyeline.mcahdb.engine.scheduler.Scheduler;
import mobi.eyeline.mcahdb.soap.SoapServer;
import mobi.eyeline.mcahdb.smpp.TestTransportMultiplexor;
//import mobi.eyeline.mcahdb.smpp.TestTransportMultiplexor;
import org.apache.axis2.AxisFault;
import com.eyeline.sme.smpp.SMPPTransceiver;
import com.eyeline.utils.config.properties.PropertiesConfig;
import com.eyeline.utils.config.Arguments;
import ru.aurorisoft.smpp.SMPPException;

/**
 * User: artem
 * Date: 01.08.2008
 */
                                                                                  
public class MCAHDB {

  private final JournalsProcessor journalsProcessor;
  private final EventStore eventStore;
  private final SoapServer soapServer;
  private final Scheduler scheduler;
  private final SMPPTransceiver transceiver;

  public MCAHDB(boolean test) throws InitException {
    try {

      GlobalConfig cfg = new GlobalConfig();

      final PropertiesConfig smppConfig = new PropertiesConfig();
      smppConfig.load(new File("conf/smpp.properties"));
      if (test) {
        System.out.println("MCAHDB started in test mode.");
        transceiver = new SMPPTransceiver(new TestTransportMultiplexor(), smppConfig, "");
      } else
        transceiver = new SMPPTransceiver(smppConfig, "");

      scheduler = new Scheduler(cfg, transceiver);

      eventStore = new EventStore(cfg);
      journalsProcessor = new JournalsProcessor(eventStore, scheduler, cfg);

      transceiver.connect();
      
      soapServer = new SoapServer(cfg, eventStore);
      soapServer.start();
    } catch (Exception e) {
      throw new InitException(e);
    }

    Runtime.getRuntime().addShutdownHook(new Thread() {
      @Override
      public void run() {
        System.out.println("Shutdown called.");
        shutdown();
      }
    });

    System.out.println("MCAHDB started.");
  }

  public void shutdown() {
    System.out.println("Shutdown journal processor.");
    journalsProcessor.shutdown();
    System.out.println("Shutdown events.");
    eventStore.shutdown();
    System.out.println("Shutown scheduler.");
    scheduler.shutdown();
    System.out.println("Shutdown smpp.");
    try {
      transceiver.shutdown();
    } catch (SMPPException e) {
      e.printStackTrace();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    try {
      System.out.println("Stop soap server");
      soapServer.stop();
    } catch (AxisFault axisFault) {
      axisFault.printStackTrace();
    }
  }

  public static void main(String[] args) {
    try {
      Arguments a = new Arguments(args);

      new MCAHDB(a.containsAttr("-t"));
    } catch (InitException e) {
      e.printStackTrace();
    }
  }
}
