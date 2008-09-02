package mobi.eyeline.mcahdb.engine.scheduler.ds;

import mobi.eyeline.mcahdb.engine.DataSourceException;

import java.util.Date;

/**
 * User: artem
 * Date: 01.09.2008
 */

public interface TimeDataSource {

  public void setTime(Date time) throws DataSourceException;

  public Date getTime() throws DataSourceException;

  public void close();
}
