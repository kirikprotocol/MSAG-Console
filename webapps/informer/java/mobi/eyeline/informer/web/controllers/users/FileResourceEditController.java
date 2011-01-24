package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.users.UserCPsettings;

/**
 * @author Aleksandr Khalitov
 */
public class FileResourceEditController extends CPResourceEditController{

  public FileResourceEditController() {
    super(UserCPsettings.Protocol.file);
  }
}
