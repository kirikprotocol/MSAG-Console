package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.users.UserCPsettings;

/**
 * @author Aleksandr Khalitov
 */
public class FtpResourceEditController extends CPResourceEditController{

  public FtpResourceEditController() {
    super(UserCPsettings.Protocol.ftp);
  }

}
