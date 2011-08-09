package mobi.eyeline.informer.admin.dep;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smppgw.SmppGWConfigManager;
import mobi.eyeline.informer.admin.smppgw.SmppGWProvider;
import mobi.eyeline.informer.admin.smppgw.SmppGWRoute;
import mobi.eyeline.informer.admin.users.User;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGW2UserDep {

  private final SmppGWConfigManager smppGwManager;

  public SmppGW2UserDep(SmppGWConfigManager smppGWManager) {
    this.smppGwManager = smppGWManager;
  }

  public void updateUser(User u) throws AdminException {
    String login = u.getLogin();

    for(SmppGWProvider p : smppGwManager.getProviderSettings().getProviders()) {
      for(SmppGWRoute r : p.getRoutes()) {
        if(r.getUser().equals(login)) {
          smppGwManager.updateSettings();
          return;
        }
      }
    }
  }
}
