package mobi.eyeline.mcahdb.engine.event.ds.impl.file;

import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.event.ds.Event;

import java.io.IOException;
import java.util.Collection;
import java.util.Date;

/**
 * User: artem
 * Date: 31.07.2008
 */

public interface Store {

  public void open(boolean readonly) throws IOException, DataSourceException;

  public void close() throws IOException;

  public void commit() throws DataSourceException, IOException;

  public void rollback() throws DataSourceException, IOException;

  public boolean exists();

  public void getEvents(String address, Date from, Date till, Collection<Event> result) throws DataSourceException, IOException;

  public void addEvent(Event event) throws DataSourceException, IOException;
}
