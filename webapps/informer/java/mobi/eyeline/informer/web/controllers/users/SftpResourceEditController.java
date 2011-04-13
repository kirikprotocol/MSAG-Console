package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.users.UserCPsettings;

/**
 * @author Aleksandr Khalitov
 */
public class SftpResourceEditController extends CPResourceEditController{

  public SftpResourceEditController() {
    super(UserCPsettings.Protocol.sftp, 22);
  }
}
