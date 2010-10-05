package ru.novosoft.smsc.web.controllers.service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.sme.SmeManager;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.config.SmscStatusManager;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class ServiceController extends SmscController {

  private boolean outOfDate;
  protected SmeManager mngr;

  protected ServiceController() {
    outOfDate = checkOutOfDate();
    mngr = WebContext.getInstance().getSmeManager();
  }

  private boolean checkOutOfDate() {
    try {
      List<Integer> result = new ArrayList<Integer>();
      SmscStatusManager ssm = getSmscStatusManager();
      for (int i = 0; i < ssm.getSmscInstancesNumber(); i++) {
        if (ssm.getSmeConfigState(i) == SmscConfigurationStatus.OUT_OF_DATE)
          result.add(i);
      }
      if (!result.isEmpty()) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.instance.out_of_date", result.toString());
        return true;
      }
    } catch (AdminException e) {
      addError(e);
      return true;
    }
    return false;
  }

  public boolean isOutOfDate() {
    return outOfDate;
  }
}
