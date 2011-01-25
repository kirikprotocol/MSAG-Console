package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.users.UserCPsettings;

/**
 * @author Aleksandr Khalitov
 */
public class LocalFtpResourceEditController extends CPResourceEditController{

  public LocalFtpResourceEditController() {
    super(UserCPsettings.Protocol.localFtp);
  }

}
