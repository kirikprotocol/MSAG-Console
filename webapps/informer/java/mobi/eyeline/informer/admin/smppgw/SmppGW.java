package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;

/**
 * author: Aleksandr Khalitov
 */
public interface SmppGW {

  public void updateConfig() throws AdminException;

  public void shutdown();
}
