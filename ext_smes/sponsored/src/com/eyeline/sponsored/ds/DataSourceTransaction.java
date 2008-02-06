package com.eyeline.sponsored.ds;

/**
 *
 * @author artem
 */
public interface DataSourceTransaction {

  /**
   * Commit transaction
   */
  public void commit() throws DataSourceException;

  /**
   * Rollback transaction
   */
  public void rollback() throws DataSourceException;

  /**
   * Close transaction
   */
  public void close();
}
