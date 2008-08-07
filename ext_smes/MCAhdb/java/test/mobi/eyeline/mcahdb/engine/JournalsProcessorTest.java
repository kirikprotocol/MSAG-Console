package mobi.eyeline.mcahdb.engine;

import org.junit.Test;
import com.eyeline.utils.config.xml.XmlConfig;
import com.eyeline.utils.config.xml.XmlConfigSection;

import java.io.File;

import mobi.eyeline.mcahdb.engine.ds.impl.file.EventsDataSourceImpl;
import mobi.eyeline.mcahdb.engine.ds.impl.file.JournalDataSourceImpl;
import mobi.eyeline.mcahdb.engine.ds.JournalDataSource;
import mobi.eyeline.mcahdb.engine.ds.EventsDataSource;
import mobi.eyeline.mcahdb.GlobalConfig;

/**
 * User: artem
 * Date: 05.08.2008
 */

public class JournalsProcessorTest {

  @Test
  public void test() {
    XmlConfig conf = new XmlConfig();
    try {
      conf.load(new File("conf", "config.xml"));

      XmlConfigSection eSection = conf.getSection("eventStore");
      EventsDataSource eventsDS = new EventsDataSourceImpl(new GlobalConfig());

      XmlConfigSection jSection = conf.getSection("journalStore");
      JournalDataSource journalDS = new JournalDataSourceImpl(new GlobalConfig());

      JournalsProcessor journalsProcessor = new JournalsProcessor(journalDS, eventsDS, new GlobalConfig());

      Thread.sleep(10000);

      journalsProcessor.shutdown();
      eventsDS.close();

    } catch (Exception e) {
      e.printStackTrace();
    }
  }
}
