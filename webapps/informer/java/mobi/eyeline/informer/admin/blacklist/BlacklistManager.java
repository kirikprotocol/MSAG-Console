package mobi.eyeline.informer.admin.blacklist;

import mobi.eyeline.informer.admin.AdminException;

import java.util.Collection;

/**
 * @author Aleksandr Khalitov
 */
public interface BlacklistManager {

  public void add(String msisdn) throws AdminException;

  public void add(Collection<String> msisdn) throws AdminException;

  public void remove(String msisdn) throws AdminException;

  public boolean contains(String msisdn) throws AdminException;

}
