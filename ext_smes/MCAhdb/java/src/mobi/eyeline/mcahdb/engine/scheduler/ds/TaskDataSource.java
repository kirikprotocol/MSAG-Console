package mobi.eyeline.mcahdb.engine.scheduler.ds;

import mobi.eyeline.mcahdb.engine.DataSourceException;

import java.util.Collection;
import java.util.Date;

/**
 * User: artem
 * Date: 28.08.2008
 */

public interface TaskDataSource {

  public void add(Task task) throws DataSourceException;

  public void remove(Task task) throws DataSourceException;

  public Task remove(String caller, String called) throws DataSourceException;

  public void list(Date fromDate, Date tillDate, Collection<Task> result) throws DataSourceException;

  public Task get(String caller, String called) throws DataSourceException;

  public void get(String called, Collection<Task> result) throws DataSourceException;

  public int size();
}
