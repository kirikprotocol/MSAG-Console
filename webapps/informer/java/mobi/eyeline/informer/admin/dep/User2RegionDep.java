package mobi.eyeline.informer.admin.dep;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.regions.RegionsManager;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UsersManager;

import java.util.Iterator;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class User2RegionDep {

  private final UsersManager usersManager;
  private final RegionsManager regionsManager;

  public User2RegionDep(UsersManager userManager, RegionsManager regionsManager) {
    this.usersManager = userManager;
    this.regionsManager = regionsManager;
  }

  public void updateUser(User u) throws AdminException {
    if (u.getRegions() != null) {
      for (Integer rId : u.getRegions()) {
        if (null == regionsManager.getRegion(rId)) {
          throw new IntegrityException("user.region.not.exists", u.getLogin(), rId.toString());
        }
      }
    }
  }

  public void removeRegion(int regionId) throws AdminException {
    for (User u : usersManager.getUsers()) {
      if (u.getRegions() == null)
        continue;
      List<Integer> regions = u.getRegions();
      for (Iterator<Integer> iter = regions.iterator(); iter.hasNext();) {
        if (iter.next().equals(regionId)) {
          iter.remove();
          u.setRegions(regions);
          usersManager.updateUser(u);
          break;
        }
      }
    }
  }
}
