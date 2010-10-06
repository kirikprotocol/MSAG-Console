package ru.novosoft.smsc.web.controllers;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfiguration;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;

import javax.faces.application.FacesMessage;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class SmscConfigurationController extends SmscController {

  private boolean outOfDate;

  protected SmscConfigurationController(SmscConfiguration configuration) {
    try {
      Map<Integer, SmscConfigurationStatus> statuses = configuration.getStatusForSmscs();
      if (statuses.containsValue(SmscConfigurationStatus.OUT_OF_DATE)) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.instance.out_of_date");
        outOfDate = true;
      }
    } catch (AdminException e) {
      addError(e);
    }
  }

  public boolean isOutOfDate() {
    return outOfDate;
  }
}
