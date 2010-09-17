package ru.novosoft.smsc.web.controllers.closed_groups;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.config.Configuration;
import ru.novosoft.smsc.web.config.SmscStatusManager;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;
import java.util.ArrayList;
import java.util.List;

/**
 * @author alkhal
 */
public class ClosedGroupsController extends SmscController {

  protected static final Logger logger = Logger.getLogger(ClosedGroupsController.class);

  private boolean outOfDate;

  protected Configuration configuration;

  protected ClosedGroupsController() {
    outOfDate = checkOutOfDate();
    configuration = WebContext.getInstance().getConfiguration();
  }

  private boolean checkOutOfDate() {
    try {
      List<Integer> result = new ArrayList<Integer>();
      SmscStatusManager ssm = getSmscStatusManager();
      for (int i = 0; i < ssm.getSmscInstancesNumber(); i++) {
        if (ssm.getRescheduleState(i) == SmscConfigurationStatus.OUT_OF_DATE)
          result.add(i);
      }
      if (!result.isEmpty()) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.instance.out_of_date", result.toString());
        return true;
      }else {
        return false;
      }
    } catch (AdminException e) {
      logger.error(e, e);
      addError(e);
      return true;
    }
  }

  public boolean isOutOfDate() {
    return outOfDate;
  }
}
