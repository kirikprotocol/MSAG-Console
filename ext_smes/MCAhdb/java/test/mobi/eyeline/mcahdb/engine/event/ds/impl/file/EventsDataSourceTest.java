package mobi.eyeline.mcahdb.engine.event.ds.impl.file;

import org.junit.Test;

import java.io.IOException;
import java.util.Collection;
import java.util.Date;
import java.util.LinkedList;

import mobi.eyeline.mcahdb.engine.event.ds.EventsDataSource;
import mobi.eyeline.mcahdb.engine.event.ds.Event;
import mobi.eyeline.mcahdb.engine.event.ds.impl.file.EventsDataSourceImpl;
import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.TestUtils;
import mobi.eyeline.mcahdb.GlobalConfig;
import com.eyeline.utils.config.ConfigException;

/**
 * User: artem
 * Date: 01.08.2008
 */

public class EventsDataSourceTest {

  @Test
  public void addTest() throws IOException, DataSourceException, ConfigException {
    final EventsDataSource eventsDS = new EventsDataSourceImpl(new GlobalConfig());

    eventsDS.addEvents(TestUtils.generateEvents(1000, 10));

    Collection<Event> events = new LinkedList<Event>();  
    eventsDS.getEvents("+79139030001", new Date(System.currentTimeMillis() - 3600000), new Date(), events);
    for (Event e : events) {
      System.out.println(e.getType() + " " + e.getCaller() + " " + e.getCalled());
    }

    eventsDS.close();
  }

  @Test
  public void getTest() throws DataSourceException, ConfigException {
    final EventsDataSource eventsDS = new EventsDataSourceImpl(new GlobalConfig());
    for (int i=0; i<10; i++) {
      Collection<Event> events = new LinkedList<Event>();
      eventsDS.getEvents("+7913903100" + i, new Date(System.currentTimeMillis() - 3600000 * 12), new Date(), events);
      for (Event e : events) {
        System.out.println(e.getType() + " " + e.getCaller() + " " + e.getCalled());
      }
    }

    eventsDS.close();
  }
}
