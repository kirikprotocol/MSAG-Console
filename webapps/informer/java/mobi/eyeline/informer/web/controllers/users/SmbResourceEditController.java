package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.users.UserCPsettings;

/**
 * @author Aleksandr Khalitov
 */
public class SmbResourceEditController extends CPResourceEditController{

  public SmbResourceEditController() {
    super(UserCPsettings.Protocol.smb);
  }
}
