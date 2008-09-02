package mobi.eyeline.mcahdb.engine.journal.ds;

import mobi.eyeline.mcahdb.engine.DataSourceException;

import java.util.Date;

/**
 * User: artem
 * Date: 31.07.2008
 */

public interface Journal {

  public Date getStartDate();

  public String getName();

  public JournalEvent nextEvent() throws DataSourceException;

  public void open() throws DataSourceException;

  public void close();
}
