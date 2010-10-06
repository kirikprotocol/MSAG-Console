package ru.novosoft.smsc.web.controllers.acl;

import ru.novosoft.smsc.admin.acl.AclManager;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscConfigurationController;

/**
 * @author Artem Snopkov
 */
public class AclController extends SmscConfigurationController {

  protected AclManager aclManager;

  public AclController() {
    super(WebContext.getInstance().getAclManager());
    aclManager = WebContext.getInstance().getAclManager();
  }

}
