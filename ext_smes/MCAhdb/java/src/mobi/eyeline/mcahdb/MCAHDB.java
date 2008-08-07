package mobi.eyeline.mcahdb;

import java.io.File;
import java.util.Date;
import java.util.Collection;

import mobi.eyeline.mcahdb.engine.ds.JournalDataSource;
import mobi.eyeline.mcahdb.engine.ds.EventsDataSource;
import mobi.eyeline.mcahdb.engine.ds.Event;
import mobi.eyeline.mcahdb.engine.ds.impl.file.JournalDataSourceImpl;
import mobi.eyeline.mcahdb.engine.ds.impl.file.EventsDataSourceImpl;
import mobi.eyeline.mcahdb.engine.JournalsProcessor;
import mobi.eyeline.mcahdb.engine.EventsFetcher;
import mobi.eyeline.mcahdb.engine.InitException;
import mobi.eyeline.mcahdb.soap.SoapServer;
import org.apache.axis2.AxisFault;

/**
 * User: artem
 * Date: 01.08.2008
 */

public class MCAHDB {

  private final EventsDataSource eventsDS;
  private final JournalsProcessor journalsProcessor;
  private final EventsFetcher eventsFetcher;
  private final SoapServer soapServer;

  public MCAHDB() throws InitException {
    try {

      GlobalConfig cfg = new GlobalConfig();

      JournalDataSource journalDS = new JournalDataSourceImpl(cfg);

      eventsDS = new EventsDataSourceImpl(cfg);

      journalsProcessor = new JournalsProcessor(journalDS, eventsDS, cfg);
      eventsFetcher = new EventsFetcher(eventsDS);

      soapServer = new SoapServer(cfg, eventsFetcher);
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

  public Collection<Event> getEvents(String address, Date from, Date till) {
    return eventsFetcher.getEvents(address, from, till);
  }

  public void shutdown() {
    journalsProcessor.shutdown();
    System.out.println("Shutdown journal processor");
    eventsDS.close();
    System.out.println("Shutdown events ds.");
    try {
      soapServer.stop();
      System.out.println("Stop soap server");
    } catch (AxisFault axisFault) {
      axisFault.printStackTrace();
    }
  }

  public static void main(String[] args) {
    try {
      new MCAHDB();
    } catch (InitException e) {
      e.printStackTrace();
    }
  }
}
