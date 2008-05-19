package ru.sibinco.smsx.engine.service.blacklist.datasource;

import com.eyelinecom.whoisd.personalization.InfinitTimePolicy;
import com.eyelinecom.whoisd.personalization.PersonalizationClient;
import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.Property;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import ru.sibinco.smsx.network.personalization.PersonalizationClientPoolFactory;
import ru.sibinco.smsx.utils.DataSourceException;

/**
 * User: artem
 * Date: 10.07.2007
 */

public class PersonalizationBlackListDataSource implements BlackListDataSource{

  private static final String IN_BLACK_LIST = "inblacklist";

  private final PersonalizationClientPool pool;

  public PersonalizationBlackListDataSource() throws DataSourceException {
    try {
      pool = PersonalizationClientPoolFactory.createPool();
    } catch (PersonalizationClientException e) {
      throw new DataSourceException(e);
    }
  }

  public void addMsisdnToBlackList(String msisdn) throws DataSourceException {
    PersonalizationClient client = null;

    try {
      client = pool.getClient();

      client.setProperty(msisdn, new Property(IN_BLACK_LIST, true, new InfinitTimePolicy()));

    } catch (PersonalizationClientException e) {
      throw new DataSourceException(e);
    } finally {
      close(client);
    }
  }

  private static void close(PersonalizationClient client) {
    try {
      if (client != null)
        client.close();
    } catch (PersonalizationClientException e) {
    }
  }

  public void removeMsisdnFromBlackList(String msisdn) throws DataSourceException {
    PersonalizationClient client = null;
    try {
      client = pool.getClient();

      client.deleteProperty(msisdn, IN_BLACK_LIST);

    } catch (PersonalizationClientException e) {
      throw new DataSourceException(e);
    } finally {
      close(client);
    }
  }

  public boolean isMsisdnInBlackList(String msisdn) throws DataSourceException {
    PersonalizationClient client = null;

    try {
      client = pool.getClient();

      return client.getProperty(msisdn, IN_BLACK_LIST) != null;

    } catch (PersonalizationClientException e) {
      throw new DataSourceException(e);
    } finally {
      close(client);
    }
  }

  public void release() {
    try {
      pool.shutdown();
    } catch (PersonalizationClientException e) {
    }
  }
}
