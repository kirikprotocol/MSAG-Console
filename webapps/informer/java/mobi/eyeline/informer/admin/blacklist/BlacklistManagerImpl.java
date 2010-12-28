package mobi.eyeline.informer.admin.blacklist;

import com.eyelinecom.whoisd.personalization.*;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;

import java.util.Collection;

/**
 * @author Aleksandr Khalitov
 */
public class BlacklistManagerImpl implements BlacklistManager {

  private static final String INFOSME_BLACK_LIST = "infosme_black_list";

  private final PersonalizationClientPool pool;

  public BlacklistManagerImpl(PersonalizationClientPool pool) {
    this.pool = pool;
  }

  public void add(Address msisdn) throws AdminException {
    if (msisdn == null) {
      return;
    }
    PersonalizationClient client = null;
    try {
      client = pool.getClient();
      client.setProperty(msisdn.getSimpleAddress(), new Property(INFOSME_BLACK_LIST, true, new InfinitTimePolicy()));
    } catch (PersonalizationClientException e) {
      throw new BlacklistException("pvss_unavailable", e);
    } finally {
      close(client);
    }
  }

  public void add(Collection<Address> msisdns) throws AdminException {
    if (msisdns == null) {
      return;
    }
    PersonalizationClient client = null;
    try {
      client = pool.getClient();
      for (Address msisdn : msisdns) {
        if (msisdn != null) {
          client.setProperty(msisdn.getSimpleAddress(), new Property(INFOSME_BLACK_LIST, true, new InfinitTimePolicy()));
        }
      }
    } catch (PersonalizationClientException e) {
      throw new BlacklistException("pvss_unavailable", e);
    } finally {
      close(client);
    }
  }

  public void remove(Address msisdn) throws AdminException {
    if (msisdn == null) {
      return;
    }
    PersonalizationClient client = null;
    try {
      client = pool.getClient();
      client.deleteProperty(msisdn.getSimpleAddress(), INFOSME_BLACK_LIST);
    } catch (PersonalizationClientException e) {
      throw new BlacklistException("pvss_unavailable", e);
    } finally {
      close(client);
    }
  }

  public void remove(Collection<Address> msisdns) throws AdminException {
    if (msisdns == null) {
      return;
    }
    PersonalizationClient client = null;
    try {
      client = pool.getClient();

      for (Address msisdn : msisdns) {
        if (msisdn != null) {
          client.deleteProperty(msisdn.getSimpleAddress(), INFOSME_BLACK_LIST);
        }
      }
    } catch (PersonalizationClientException e) {
      throw new BlacklistException("pvss_unavailable", e);
    } finally {
      close(client);
    }
  }

  public boolean contains(Address msisdn) throws AdminException {
    if (msisdn == null) {
      return false;
    }
    PersonalizationClient client = null;
    try {
      client = pool.getClient();
      return client.getProperty(msisdn.getSimpleAddress(), INFOSME_BLACK_LIST) != null;
    } catch (PersonalizationClientException e) {
      return false; // Если произошла ошибка при взаимодействии с персонализацией, то считаем, что юзер не в блек-листе
    } finally {
      close(client);
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  private static void close(PersonalizationClient client) {
    if (client != null) {
      try {
        client.close();
      } catch (PersonalizationClientException ignored) {
      }
    }
  }


}
