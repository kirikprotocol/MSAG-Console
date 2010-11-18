package mobi.eyeline.informer.admin.blacklist;

import com.eyelinecom.whoisd.personalization.InfinitTimePolicy;
import com.eyelinecom.whoisd.personalization.PersonalizationClient;
import com.eyelinecom.whoisd.personalization.PersonalizationClientPool;
import com.eyelinecom.whoisd.personalization.Property;
import com.eyelinecom.whoisd.personalization.exceptions.PersonalizationClientException;
import mobi.eyeline.informer.admin.AdminException;

import java.util.Collection;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

/**
 * @author Aleksandr Khalitov
 */
public class BlackListManagerImpl implements BlacklistManager {

  private static final String INFOSME_BLACK_LIST = "infosme_black_list";

  private final PersonalizationClientPool pool;

  private final ReadWriteLock lock = new ReentrantReadWriteLock();

  public BlackListManagerImpl(PersonalizationClientPool pool) {
    this.pool = pool;
  }

  public void add(String msisdn) throws AdminException {
    if (msisdn == null || (msisdn = msisdn.trim()).length() == 0) {
      throw new IllegalArgumentException("Argument is empty: " + msisdn);
    }
    msisdn = convertNumber(msisdn);
    PersonalizationClient client = null;
    try {
      client = pool.getClient();
      try {
        lock.writeLock().lock();
        client.setProperty(msisdn, new Property(INFOSME_BLACK_LIST, true, new InfinitTimePolicy()));
      } finally {
        lock.writeLock().unlock();
      }
    } catch (PersonalizationClientException e) {
      throw new BlacklistException("interaction_error", e);
    } finally {
      close(client);
    }
  }


  public void add(Collection<String> msisdns) throws AdminException {
    if (msisdns == null) {
      throw new IllegalArgumentException("Argument is null");
    }
    PersonalizationClient client = null;
    try {
      client = pool.getClient();
      try {
        lock.writeLock().lock();
        for (String msisdn : msisdns) {
          if (msisdn != null && (msisdn = msisdn.trim()).length() != 0) {
            msisdn = convertNumber(msisdn);
            client.setProperty(msisdn, new Property(INFOSME_BLACK_LIST, true, new InfinitTimePolicy()));
          }
        }
      } finally {
        lock.writeLock().unlock();
      }
    } catch (PersonalizationClientException e) {
      throw new BlacklistException("interaction_error", e);
    } finally {
      close(client);
    }
  }

  public void remove(String msisdn) throws AdminException {
    if (msisdn == null || (msisdn = msisdn.trim()).length() == 0) {
      throw new IllegalArgumentException("Argument is empty: " + msisdn);
    }
    msisdn = convertNumber(msisdn);
    PersonalizationClient client = null;
    try {
      client = pool.getClient();

      try {
        lock.writeLock().lock();
        client.deleteProperty(msisdn, INFOSME_BLACK_LIST);
      } finally {
        lock.writeLock().unlock();

      }
    } catch (PersonalizationClientException e) {
      throw new BlacklistException("interaction_error", e);
    } finally {
      close(client);
    }
  }

  public void remove(Collection<String> msisdns) throws AdminException {
    if (msisdns == null) {
      throw new IllegalArgumentException("Argument is null");
    }
    PersonalizationClient client = null;
    try {
      client = pool.getClient();

      try {
        lock.writeLock().lock();
        for (String msisdn : msisdns) {
          if (msisdn != null && (msisdn = msisdn.trim()).length() != 0) {
            msisdn = convertNumber(msisdn);
            client.deleteProperty(msisdn, INFOSME_BLACK_LIST);
          }
        }
      } finally {
        lock.writeLock().unlock();

      }
    } catch (PersonalizationClientException e) {
      throw new BlacklistException("interaction_error", e);
    } finally {
      close(client);
    }
  }

  public boolean contains(String msisdn) throws AdminException {
    if (msisdn == null || (msisdn = msisdn.trim()).length() == 0) {
      throw new IllegalArgumentException("Arguemnt is empty: " + msisdn);
    }
    msisdn = convertNumber(msisdn);
    PersonalizationClient client = null;
    try {
      client = pool.getClient();
      try {
        lock.readLock().lock();
        return client.getProperty(msisdn, INFOSME_BLACK_LIST) != null;
      } finally {
        lock.readLock().unlock();
      }
    } catch (PersonalizationClientException e) {
      throw new BlacklistException("interaction_error", e);
    } finally {
      close(client);
    }
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  private static void close(PersonalizationClient client) {
    if (client != null) {
      try {
        client.close();
      } catch (PersonalizationClientException e) {
      }
    }
  }

  private static String convertNumber(String ms) {
    char c = ms.charAt(0);
    switch (c) {
      case '7':
        return '+' + ms;
      case '8':
        return "+7" + ms.substring(1);
      default:
        return ms;
    }
  }

}
