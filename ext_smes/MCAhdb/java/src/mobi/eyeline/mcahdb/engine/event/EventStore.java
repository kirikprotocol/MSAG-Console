package mobi.eyeline.mcahdb.engine.event;

import mobi.eyeline.mcahdb.engine.event.ds.EventsDataSource;
import mobi.eyeline.mcahdb.engine.event.ds.Event;
import mobi.eyeline.mcahdb.engine.event.ds.impl.file.EventsDataSourceImpl;
import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.InitException;

import java.util.Collection;
import java.util.Date;
import java.util.Collections;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 01.08.2008
 */

public class EventStore {

  private static final Category log = Category.getInstance(EventStore.class);

  private final EventsDataSource ds;

  public EventStore(Config config) throws InitException {
    try {
      this.ds = new EventsDataSourceImpl(config);
    } catch (DataSourceException e) {
      throw new InitException(e);
    }
  }

  public void addEvents(Collection<Event> events) {
    try {
      ds.addEvents(events);
    } catch (DataSourceException e) {
      log.error("Can't add events", e);
    }
  }

  public Collection<Event> getEvents(String address, Date from, Date till) {
    try {
      return ds.getEvents(address, from, till);
    } catch (DataSourceException e) {
      log.error("Can't fetch events", e);
      return Collections.emptyList();
    }
  }

  public void shutdown() {
    ds.close();
  }

  public interface Config {
    public String getEventsStoreDir();
  }
}
