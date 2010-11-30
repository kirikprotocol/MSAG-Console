package mobi.eyeline.informer.admin.cdr;

import mobi.eyeline.informer.admin.users.User;

/**
 * @author Aleksandr Khalitov
 */
public interface CdrUsers {

  public User getUser(String login);
}
