package ru.sibinco.smsx.engine.service.secret.datasource;

import com.eyeline.sme.utils.ds.DataSourceException;

import java.util.Collection;
import java.util.Map;

/**
 * User: artem
 * Date: Jul 1, 2007
 */

public interface SecretDataSource {

  public SecretUser loadSecretUserByAddress(final String address) throws DataSourceException;
  public void saveSecretUser(final SecretUser secretUser) throws DataSourceException;
  public void removeSecretUser(final SecretUser secretUser) throws DataSourceException;
  public Map loadSecretUsersByAddresses(final String[] addresses) throws DataSourceException;

  public Collection loadSecretMessagesByAddress(final String address) throws DataSourceException;
  public SecretMessage loadSecretMessageById(int id) throws DataSourceException;
  public int loadMessagesCountByAddress(final String address) throws DataSourceException;
  public void saveSecretMessage(final SecretMessage secretMessage) throws DataSourceException;
  public void removeSecretMessage(final SecretMessage secretMessage) throws DataSourceException;
  public void updateMessageStatus(SecretMessage secretMessage) throws DataSourceException;
  public int updateMessageStatus(long smppId, int status) throws DataSourceException;
  public void updateMessageSmppId(SecretMessage secretMessage) throws DataSourceException;

  public SecretUserWithMessages loadSecretUserWithMessages(final String address) throws DataSourceException;

  public void release();
}
