package ru.sibinco.smsx.engine.service.secret.datasource;

import java.util.Collection;
import java.util.Date;
import java.util.Map;

import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: Jul 1, 2007
 */

public interface SecretDataSource {

  public SecretUser loadSecretUserByAddress(final String address) throws DataSourceException;
  public void saveSecretUser(final SecretUser secretUser) throws DataSourceException;
  public void removeSecretUser(final SecretUser secretUser) throws DataSourceException;
  public Map<String, SecretUser> loadSecretUsersByAddresses(final String[] addresses) throws DataSourceException;

  public DataSourceTransaction createTransaction() throws DataSourceException;

  public Collection<SecretMessage> loadSecretMessagesByAddress(final String address) throws DataSourceException;
  public SecretMessage loadSecretMessageById(int id) throws DataSourceException;
  public int loadMessagesCountByAddress(final String address) throws DataSourceException;

  public void saveSecretMessage(final SecretMessage secretMessage) throws DataSourceException;
  public void saveSecretMessage(SecretMessage secretMessage, DataSourceTransaction tx) throws DataSourceException;

  public void removeSecretMessage(final SecretMessage secretMessage) throws DataSourceException;
  public int removeFinalizedSecretMessages(Date maxDate, int limit) throws DataSourceException;
  public void updateMessageStatus(SecretMessage secretMessage) throws DataSourceException;
  public int updateMessageStatus(long id, int status) throws DataSourceException;
  public void updateMessageSmppId(SecretMessage secretMessage) throws DataSourceException;

  public SecretUserWithMessages loadSecretUserWithMessages(final String address) throws DataSourceException;

  public void release();
}
