package mobi.eyeline.mcahdb.engine.event.ds;

import mobi.eyeline.mcahdb.engine.DataSourceException;

import java.util.Collection;
import java.util.Date;

/**
 * User: artem
 * Date: 31.07.2008
 */

public interface EventsDataSource {

  public Collection<Event> getEvents(String address, Date from, Date till) throws DataSourceException;

  public void addEvents(Collection<Event> events) throws DataSourceException;

  public void close();
}
