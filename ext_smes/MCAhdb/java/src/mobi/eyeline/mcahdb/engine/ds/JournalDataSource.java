package mobi.eyeline.mcahdb.engine.ds;

import java.util.Collection;

/**
 * User: artem
 * Date: 31.07.2008
 */

public interface JournalDataSource {

  public Collection<Journal> getJournals() throws DataSourceException;

  public Journal getJournal(String journalName) throws DataSourceException;

  public void removeJournal(String journalName) throws DataSourceException;

}
