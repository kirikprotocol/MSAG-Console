package mobi.eyeline.mcahdb.engine.scheduler.ds.impl.file.page;

import mobi.eyeline.mcahdb.engine.DataSourceException;
import mobi.eyeline.mcahdb.engine.scheduler.ds.Task;

import java.util.Collection;
import java.util.Date;

/**
 * User: artem
 * Date: 27.08.2008
 */

public interface SchedulePage {

  public String getId();

  public TaskPointer add(Task task) throws DataSourceException;

  public Task remove(long taskId) throws DataSourceException;

  public Task get(long taskId) throws DataSourceException;

  public void list(Date from, Date till, Collection<Task> result) throws DataSourceException;

  public void open() throws DataSourceException;

  public void close();
}
