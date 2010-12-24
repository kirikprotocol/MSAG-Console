package mobi.eyeline.informer.admin.dep;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.restriction.RestrictionDaemon;
import mobi.eyeline.informer.admin.restriction.RestrictionsFilter;
import mobi.eyeline.informer.admin.restriction.RestrictionsManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManager;

import java.util.List;

/**
 * @author Artem Snopkov
 */
public class Restriction2UserDep {

  private final RestrictionsManager restrictionsManager;
  private final RestrictionDaemon restrictionDaemon;
  private final UsersManager usersManager;

  public Restriction2UserDep(RestrictionsManager restrictionManager, RestrictionDaemon restrictionDaemon, UsersManager usersManager) {
    this.restrictionsManager = restrictionManager;
    this.restrictionDaemon = restrictionDaemon;
    this.usersManager = usersManager;
  }

  public void updateUser(User user) {
    restrictionDaemon.rebuildSchedule();
  }

  public void removeUser(User user) throws AdminException {
    RestrictionsFilter rFilter = new RestrictionsFilter();
    rFilter.setUserId(user.getLogin());

    List<Restriction> rs = restrictionsManager.getRestrictions(rFilter);

    boolean rebuild = false;
    for(Restriction r : rs) {
      if(!r.isAllUsers()) {
        List<String> users = r.getUserIds();
        if(users.size() == 1) {
          restrictionsManager.deleteRestriction(r.getId());
        }else {
          users.remove(user.getLogin());
          r.setUserIds(users);
          restrictionsManager.updateRestriction(r);
        }
        rebuild = true;
      }
    }

    if(rebuild) {
      restrictionDaemon.rebuildSchedule();
    }
  }

  public void updateRestriction(Restriction restriction) throws IntegrityException {
    for (String userId : restriction.getUserIds()) {
      if (usersManager.getUser(userId) == null)
        throw new IntegrityException("user_not_exist", userId);
    }

  }
}
