package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;

/**
 * author: Aleksandr Khalitov
 */
public interface SmppGWConfigManagerContext {

  public User getUser(String login);

  public boolean containsDelivery(String login, int deliveryId) throws AdminException;

  public FileSystem getFileSystem();
}
