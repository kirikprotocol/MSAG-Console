package ru.sibinco.smsx.engine.service.blacklist.datasource;

import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 10.07.2007
 */

public interface BlackListDataSource {

  public void addMsisdnToBlackList(String msisdn) throws DataSourceException;

  public void removeMsisdnFromBlackList(String msisdn) throws DataSourceException;

  public boolean isMsisdnInBlackList(String msisdn) throws DataSourceException;

  public void release();
}
