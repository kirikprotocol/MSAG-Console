package ru.sibinco.smsx.engine.service.secret.datasource;

import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 13.11.2008
 */
public interface DataSourceTransaction {

  public void commit() throws DataSourceException;

  public void rollback() throws DataSourceException;

  public void close() throws DataSourceException;
}
