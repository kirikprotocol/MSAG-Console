package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class CPResourceController extends InformerController{

  protected static final String USER_ID_PARAMETER = "user_ID";
  protected static final String CP_SETTINGS_ID_PARAMETER = "cp_settings_ID";

  public List<SelectItem> getUniqueUsers() {
    List<User> us = getConfig().getUsers();
    List<SelectItem> res = new ArrayList<SelectItem>(us.size());
    for(User u : us) {
      res.add(new SelectItem(u.getLogin(),u.getLogin()));
    }
    return res;
  }

  public List<SelectItem> getProtocols() {
    UserCPsettings.Protocol[] protocols = UserCPsettings.Protocol.values();
    List<SelectItem> ret = new ArrayList<SelectItem>(protocols.length);
    for(UserCPsettings.Protocol p : protocols) {
      ret.add(new SelectItem(p, getLocalizedString("cp.resource.protocol."+p.toString())));
    }
    return ret;
  }

  public List<SelectItem> getWorkTypes() {
    UserCPsettings.WorkType[] workTypes = UserCPsettings.WorkType.values();
    List<SelectItem> ret = new ArrayList<SelectItem>(workTypes.length);
    for(UserCPsettings.WorkType p : workTypes) {
      ret.add(new SelectItem(p, getLocalizedString("cp.resource.workType." + p.toString())));
    }
    return ret;
  }

  public boolean isLocalFtpDeployed() {
    return getConfig().isFtpServerDeployed();
  }

}
