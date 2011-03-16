package mobi.eyeline.informer.admin.dep;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.restriction.RestrictionProvider;
import mobi.eyeline.informer.admin.restriction.RestrictionsFilter;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManager;

import java.util.List;

/**
 * @author Artem Snopkov
 */
public class Restriction2UserDep {

  private final RestrictionProvider restrictionProvider;
  private final UsersManager usersManager;

  public Restriction2UserDep(RestrictionProvider restrictionProvider, UsersManager usersManager) {
    this.restrictionProvider = restrictionProvider;
    this.usersManager = usersManager;
  }

  public void updateUser(User user) {
    restrictionProvider.reschedule();
  }

  public void removeUser(User user) throws AdminException {
    RestrictionsFilter rFilter = new RestrictionsFilter();
    rFilter.setUserId(user.getLogin());

    List<Restriction> rs = restrictionProvider.getRestrictions(rFilter);

    boolean rebuild = false;
    for(Restriction r : rs) {
      if(!r.isAllUsers()) {
        List<String> users = r.getUserIds();
        if(users.size() == 1) {
          restrictionProvider.deleteRestriction(r.getId());
        }else {
          users.remove(user.getLogin());
          r.setUserIds(users);
          restrictionProvider.updateRestriction(r);
        }
        rebuild = true;
      }
    }

    if(rebuild) {
      restrictionProvider.reschedule();
    }
  }

  public void updateRestriction(Restriction restriction) throws IntegrityException {
    if(restriction.getUserIds() != null) {
      for (String userId : restriction.getUserIds()) {
        if (usersManager.getUser(userId) == null)
          throw new IntegrityException("user_not_exist", userId);
      }
    }
  }
}
