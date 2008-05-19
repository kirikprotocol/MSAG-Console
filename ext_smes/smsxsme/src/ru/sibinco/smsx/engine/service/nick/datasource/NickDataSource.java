package ru.sibinco.smsx.engine.service.nick.datasource;

import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: Sep 6, 2007
 */

public interface NickDataSource {

  public void setNick(String abonentAddress, String nick) throws DataSourceException;

  public void deleteNick(String abonentAddress) throws DataSourceException;

  public NickNick getNickByAddress(String abonentAddress) throws DataSourceException;
}
