package ru.novosoft.smsc.infosme.backend;

import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.PersonalizationClient;
import com.eyelinecom.whoisd.personalization.Property;
import com.eyelinecom.whoisd.personalization.InfinitTimePolicy;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import ru.novosoft.smsc.admin.AdminException;

/**
 * User: artem
 * Date: 26.05.2008
 */

public class BlackListManager {

  private static final String INFOSME_BLACK_LIST = "infosme_black_list";

  private final PersonalizationClientPool pool;

  public BlackListManager(PersonalizationClientPool pool) {
    this.pool = pool;
  }

  public void add(String msisdn) throws AdminException {
    PersonalizationClient client = null;
    try {
      client = pool.getClient();

      client.setProperty(msisdn, new Property(INFOSME_BLACK_LIST, true, new InfinitTimePolicy()));

    } catch (PersonalizationClientException e) {
      throw new AdminException(e.getMessage());
    } finally {
      close(client);
    }
  }

  public void remove(String msisdn) throws AdminException {
    PersonalizationClient client = null;
    try {
      client = pool.getClient();

      client.deleteProperty(msisdn, INFOSME_BLACK_LIST);

    } catch (PersonalizationClientException e) {
      throw new AdminException(e.getMessage());
    } finally {
      close(client);
    }
  }

  public boolean contains(String msisdn) throws AdminException {
    PersonalizationClient client = null;
    try {
      client = pool.getClient();

      return client.getProperty(msisdn, INFOSME_BLACK_LIST) != null;

    } catch (PersonalizationClientException e) {
      throw new AdminException(e.getMessage());
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
}
