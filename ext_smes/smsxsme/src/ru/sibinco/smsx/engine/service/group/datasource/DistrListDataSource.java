package ru.sibinco.smsx.engine.service.group.datasource;

import ru.sibinco.smsx.utils.DataSourceException;

import java.util.Collection;

/**
 * User: artem
 * Date: 18.11.2008
 */
public interface DistrListDataSource {

  public DistrList createDistrList(String name, String owner, int maxElements) throws DataSourceException;

  public void removeDistrList(String name, String owner) throws DataSourceException;

  public void removeDistrLists(String owner) throws DataSourceException;

  public DistrList getDistrList(String name, String owner) throws DataSourceException;

  public Collection<DistrList> getDistrLists(String owner) throws DataSourceException;

  public boolean containsDistrList(String name, String owner) throws DataSourceException;

  public DistrList getDistrList(long id) throws DataSourceException;

  public int size() throws DataSourceException;

  public void close();
}
