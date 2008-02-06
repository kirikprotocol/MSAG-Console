package com.eyeline.sponsored.ds;

/**
 *
 * @author artem
 */
public interface ResultSet<T> {

  public T get() throws DataSourceException;

  public boolean next() throws DataSourceException;
  
  public void close() throws DataSourceException;
}
