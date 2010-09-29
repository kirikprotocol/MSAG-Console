package mobi.eyeline.informer.admin.blacklist;

import mobi.eyeline.informer.admin.AdminException;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

/**
 * @author Aleksandr Khalitov
 */
public class TestBlacklistManager implements BlacklistManager{

  private Set<String> blacklist = new HashSet<String>();

  public synchronized void add(String msisdn) throws AdminException {
    if(msisdn == null || (msisdn = msisdn.trim()).length() == 0) {
      throw new IllegalArgumentException("Args are null");
    }
    blacklist.add(convertNumber(msisdn));
  }

  public synchronized void add(Collection<String> msisdn) throws AdminException {
    if(msisdn == null) {
      throw new IllegalArgumentException("Args are null");
    }
    for(String m : msisdn) {
      if(m != null && (m = m.trim()).length() > 0) {
        blacklist.add(convertNumber(m));
      }
    }
  }

  public synchronized void remove(Collection<String> msisdn) throws AdminException {
    if(msisdn == null) {
      throw new IllegalArgumentException("Args are null");
    }
    for(String m : msisdn) {
      if(m != null && (m = m.trim()).length() > 0) {
        blacklist.remove(convertNumber(m));
      }
    }
  }

  public synchronized void remove(String msisdn) throws AdminException {
    if(msisdn == null || (msisdn = msisdn.trim()).length() == 0) {
      throw new IllegalArgumentException("Args are null");
    }
    blacklist.remove(convertNumber(msisdn));
  }

  public synchronized boolean contains(String msisdn) throws AdminException {
    if(msisdn == null || (msisdn = msisdn.trim()).length() == 0) {
      throw new IllegalArgumentException("Args are null");
    }
    return blacklist.contains(convertNumber(msisdn));
  }

  private static String convertNumber(String ms) {
    char c = ms.charAt(0);
    switch (c) {
      case '7': return '+'+ms;
      case '8': return "+7"+ms.substring(1);
      default: return ms;
    }
  }
}
