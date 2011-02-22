package mobi.eyeline.informer.admin.archive;

import mobi.eyeline.informer.admin.delivery.UnmodifiableDeliveryManager;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;

/**
 * @author Aleksandr Khalitov
 */
public interface ArchiveContext {

  public User getUser(String login);

  public UnmodifiableDeliveryManager getDeliveryManager();

  public FileSystem getFileSystem();

}
