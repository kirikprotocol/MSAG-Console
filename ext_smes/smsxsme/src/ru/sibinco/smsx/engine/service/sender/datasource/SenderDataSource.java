package ru.sibinco.smsx.engine.service.sender.datasource;

import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 06.07.2007
 */

public interface SenderDataSource {

  public SenderMessage loadSenderMessageById(int id) throws DataSourceException;
  public void saveSenderMessage(final SenderMessage msg) throws DataSourceException;
  public void removeSenderMessage(final SenderMessage msg) throws DataSourceException;
  public int updateMessageStatus(long smppId, int status) throws DataSourceException;
  public void updateMessageSmppId(SenderMessage msg) throws DataSourceException;

  public void release();
}
