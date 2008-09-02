package mobi.eyeline.mcahdb.engine.journal.ds;

import mobi.eyeline.mcahdb.engine.DataSourceException;

import java.util.Collection;

/**
 * User: artem
 * Date: 31.07.2008
 */

public interface JournalDataSource {

  public void getJournals(Collection<Journal> result) throws DataSourceException;

  public Journal getJournal(String journalName) throws DataSourceException;

  public void removeJournal(String journalName) throws DataSourceException;

}
