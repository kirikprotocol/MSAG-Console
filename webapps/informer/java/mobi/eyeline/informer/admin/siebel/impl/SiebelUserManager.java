package mobi.eyeline.informer.admin.siebel.impl;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;

/**
 * @author Aleksandr Khalitov
 */
public interface SiebelUserManager {

  public User getUser(String login) throws AdminException;

}
