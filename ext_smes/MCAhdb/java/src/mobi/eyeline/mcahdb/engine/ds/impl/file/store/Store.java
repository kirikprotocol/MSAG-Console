package mobi.eyeline.mcahdb.engine.ds.impl.file.store;

import mobi.eyeline.mcahdb.engine.ds.Event;
import mobi.eyeline.mcahdb.engine.ds.DataSourceException;

import java.io.IOException;
import java.util.Collection;
import java.util.Date;

/**
 * User: artem
 * Date: 31.07.2008
 */

public interface Store {

  public void open() throws IOException, DataSourceException;

  public void close() throws IOException;

  public void commit() throws DataSourceException, IOException;

  public void rollback() throws DataSourceException, IOException;

  public Collection<Event> getEvents(String address, Date from, Date till) throws DataSourceException, IOException;

  public void addEvent(Event event) throws DataSourceException, IOException;
}
