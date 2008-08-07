package mobi.eyeline.mcahdb.engine;

import mobi.eyeline.mcahdb.engine.ds.Event;
import mobi.eyeline.mcahdb.engine.ds.EventsDataSource;
import mobi.eyeline.mcahdb.engine.ds.DataSourceException;

import java.util.Collection;
import java.util.Date;
import java.util.Collections;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 01.08.2008
 */

public class EventsFetcher {

  private static final Category log = Category.getInstance(EventsFetcher.class);    

  private final EventsDataSource ds;

  public EventsFetcher(EventsDataSource ds) {
    this.ds = ds;
  }

  public Collection<Event> getEvents(String address, Date from, Date till) {
    try {
      return ds.getEvents(address, from, till);
    } catch (DataSourceException e) {
      log.error("Can't fetch events", e);
      return Collections.emptyList();
    }
  }
}
