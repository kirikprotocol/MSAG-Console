package ru.novosoft.smsc.web.controllers.closed_groups;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupManager;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.controllers.SmscConfigurationController;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;
import java.util.ArrayList;
import java.util.List;

/**
 * @author alkhal
 */
public class ClosedGroupsController extends SmscConfigurationController {

  protected static final Logger logger = Logger.getLogger(ClosedGroupsController.class);

  protected ClosedGroupManager manager;

  protected ClosedGroupsController() {
    super(WebContext.getInstance().getClosedGroupManager());
    manager = WebContext.getInstance().getClosedGroupManager();
  }
}
