package ru.sibinco.smsx.engine.service.group.datasource;

import ru.sibinco.smsx.utils.DataSourceException;

import java.util.Map;
import java.util.Collection;

/**
 * User: artem
 * Date: 12.03.2009
 */
public interface GroupSendDataSource {

  public int insert(Collection<String> adresses) throws DataSourceException;
  public Map<String, Integer> statuses(int id) throws DataSourceException;
  public int updateStatus(long smppId, int status) throws DataSourceException;
  public int updateStatus(int id, String address, int status) throws DataSourceException;
  public int updateSmppId(int id, String address, long smppId) throws DataSourceException;

  public void release();
}
