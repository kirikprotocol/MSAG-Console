package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.dep.IntegrityException;
import mobi.eyeline.informer.admin.siebel.impl.SiebelUserManager;
import mobi.eyeline.informer.admin.users.User;

/**
* @author Artem Snopkov
*/
class SiebelUserManagerImpl implements SiebelUserManager {

  private AdminContext context;

  public SiebelUserManagerImpl(AdminContext context) {
    this.context = context;
  }

  public User getUser(String login) throws AdminException {
    User u = context.getUser(login);
    if (u == null) {
      throw new IntegrityException("user_not_exist", login);
    }
    return u;
  }
}
