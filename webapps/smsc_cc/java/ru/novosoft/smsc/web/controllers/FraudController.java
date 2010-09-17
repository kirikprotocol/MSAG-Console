package ru.novosoft.smsc.web.controllers;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.fraud.FraudSettings;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;

import javax.faces.application.FacesMessage;
import java.util.ArrayList;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 16.09.2010
 * Time: 15:32:53
 */
public class FraudController extends SettingsController<FraudSettings> {
  private static final Logger logger = Logger.getLogger(FraudController.class);

  private DynamicTableModel whiteList = new DynamicTableModel();
  private FraudSettings frSettings;
  private Object enableCheck;
  private String tile;


  public FraudController() {
    super(ConfigType.Fraud);
    frSettings = getSettings();
    for(Address addr : frSettings.getWhiteList()) {
      DynamicTableRow row = new DynamicTableRow();
      row.setValue("address",addr.getSimpleAddress());
      whiteList.addRow(row);
    }
  }



  public void setWhiteList(DynamicTableModel whiteList) {
    this.whiteList = whiteList;
  }

  public DynamicTableModel getWhiteList() {
    return whiteList;
  }


  public String save() {

    List<Address> wl = new ArrayList<Address>();
    int i = 0;
    for (DynamicTableRow row : whiteList.getRows()) {
      String value = (String) row.getValue("address");
      if (value == null || value.length() == 0) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.fraud.empty.address");
        return null;
      }
      try {
        Address addr = new Address(value);
        wl.add(addr);
      } catch (Exception e) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.fraud.invalid.address",value);
        return null;
      }
    }
    try {
      frSettings.setWhiteList(wl);
      setSettings(frSettings);
      Revision rev = submitSettings();
      if (rev != null) {
        addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual", rev.getUser());
        return null;
      }

    } catch (AdminException e) {
      logger.error(e, e);
      addError(e);
    }

    return "INDEX";
  }

  public String reset() {
    try {
      resetSettings();
    } catch (AdminException e) {
      addError(e);
    }
    return "FRAUD";
  }

  @Override
  protected FraudSettings loadSettings() throws AdminException {
    return getConfiguration().getFraudSettings();
  }

  @Override
  protected void saveSettings(FraudSettings settings) throws AdminException {
    getConfiguration().updateFraudSettings(settings, getUserPrincipal().getName());
  }

  @Override
  protected FraudSettings cloneSettings(FraudSettings settings) {
    return settings.cloneSettings();
  }

  public void setEnableCheck(boolean enableCheck) {
    frSettings.setEnableCheck(enableCheck);
  }

  public boolean isEnableCheck() {
    return frSettings.isEnableCheck();
  }

  public void setEnableReject(boolean enableReject) {
    frSettings.setEnableReject(enableReject);
  }

  public boolean isEnableReject() {
    return frSettings.isEnableReject();
  }

  public void setTail(int tile) throws AdminException {
    frSettings.setTail(tile);
  }

  public int getTail() {
    return frSettings.getTail();
  }
}
