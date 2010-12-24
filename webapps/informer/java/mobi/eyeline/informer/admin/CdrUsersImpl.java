package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.cdr.CdrUsers;
import mobi.eyeline.informer.admin.users.User;

/**
* @author Artem Snopkov
*/
class CdrUsersImpl implements CdrUsers {

  private AdminContext context;

  public CdrUsersImpl(AdminContext context) {
    this.context = context;
  }

  public User getUser(String login) {
    return context.getUser(login);
  }
}
