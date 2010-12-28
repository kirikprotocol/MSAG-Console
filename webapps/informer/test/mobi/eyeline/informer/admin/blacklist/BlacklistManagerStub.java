package mobi.eyeline.informer.admin.blacklist;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

/**
 * @author Aleksandr Khalitov                           
 */
public class BlacklistManagerStub implements BlacklistManager{

  private Set<String> blacklist = new HashSet<String>();

  public synchronized void add(Address msisdn) throws AdminException {
    if(msisdn == null) {
      throw new IllegalArgumentException("Args are null");
    }
    blacklist.add(msisdn.getSimpleAddress());
  }

  public synchronized void add(Collection<Address> msisdn) throws AdminException {
    if(msisdn == null) {
      throw new IllegalArgumentException("Args are null");
    }
    for(Address m : msisdn) {
      if(m != null) {
        blacklist.add(m.getSimpleAddress());
      }
    }
  }

  public synchronized void remove(Collection<Address> msisdn) throws AdminException {
    if(msisdn == null) {
      throw new IllegalArgumentException("Args are null");
    }
    for(Address m : msisdn) {
      if(m != null) {
        blacklist.remove(m.getSimpleAddress());
      }
    }
  }

  public synchronized void remove(Address msisdn) throws AdminException {
    if(msisdn == null) {
      throw new IllegalArgumentException("Args are null");
    }
    blacklist.remove(msisdn.getSimpleAddress());
  }

  public synchronized boolean contains(Address msisdn) throws AdminException {
    if(msisdn == null) {
      throw new IllegalArgumentException("Args are null");
    }
    return blacklist.contains(msisdn.getSimpleAddress());
  }

}
