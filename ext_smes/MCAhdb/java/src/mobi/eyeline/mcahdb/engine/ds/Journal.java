package mobi.eyeline.mcahdb.engine.ds;

import java.util.Date;

/**
 * User: artem
 * Date: 31.07.2008
 */

public interface Journal {

  public Date getStartDate();

  public String getName();

  public Event nextEvent() throws DataSourceException;

  public void open() throws DataSourceException;

  public void close();
}
