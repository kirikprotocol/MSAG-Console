package ru.novosoft.smsc.web.controllers.fraud;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.fraud.FraudSettings;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.config.SmscStatusManager;
import ru.novosoft.smsc.web.controllers.SettingsController;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 16.09.2010
 * Time: 15:32:53
 */
public class FraudController extends SettingsController<FraudSettings> {
  private static final Logger logger = Logger.getLogger(FraudController.class);

  //private DynamicTableModel whiteList = new DynamicTableModel();

  private String newAddr;
  FraudSettings frSettings;
  List<Address> addresses;

  public FraudController() {
    super(ConfigType.Fraud);
    frSettings = getSettings();
    checkOutOfDate();
    addresses= new ArrayList(frSettings.getWhiteList());
  }



//  public void setWhiteList(DynamicTableModel whiteList) {
//    this.whiteList = whiteList;
//  }
//
//  public DynamicTableModel getWhiteList() {
//    return whiteList;
//  }


  private List selectedRows;
  public void setSelectedRows(List rows) {
    selectedRows = rows;
  }


  public DataTableModel getAddressTableModel() {
    final Collection<Address> whitelist = addresses;

    return new DataTableModel() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        List<Address> result = new ArrayList<Address>(whitelist);
        Collections.sort(result, new Comparator<Address>() {
          public int compare(Address o1, Address o2) {
            if (sortOrder != null) {
              int mul = sortOrder.isAsc() ? 1 : -1;
              if ("address".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getSimpleAddress().compareTo(o2.getSimpleAddress());
              }
            }
            return o1.getSimpleAddress().compareTo(o2.getSimpleAddress());
          }
        });
        int fromIndex = startPos>=whitelist.size() ?   0 : startPos;
        int toIndex   = startPos+count;
        if(toIndex>whitelist.size()) toIndex = whitelist.size();
        return result.subList(fromIndex,toIndex);
      }

      public int getRowsCount() {
        return whitelist.size();
      }
    };
  }

   public void removeSelected(ActionEvent e) {
    if (selectedRows != null && !selectedRows.isEmpty()) {
      for (String s : (List<String>) selectedRows) {
        addresses.remove(new Address(s));
      }
      try {
        frSettings.setWhiteList(addresses);
        setSettings(frSettings);
        viewChanges();
      } catch (AdminException e1) {
        addError(e1);
      }
    }
  }

  public String save() {

    try {
      //frSettings.setWhiteList(wl);
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

  public void viewChanges() {
    addLocalizedMessage(FacesMessage.SEVERITY_INFO, "smsc.users.submit.hint");
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


  public boolean isEnableCheck() {
    return frSettings.isEnableCheck();
  }
  public void  setEnableCheck(boolean v) {
    frSettings.setEnableCheck(v);
  }


  public boolean isEnableReject() {
    return frSettings.isEnableReject();
  }
  public void  setEnableReject(boolean v) {
    frSettings.setEnableReject(v);
  }


  public int getTail() {
    return frSettings.getTail();
  }

  public void setTail(int v) throws AdminException {
      frSettings.setTail(v);
  }


  public void setNewAddr(String newAddr) {
    this.newAddr = newAddr;
  }

  public String getNewAddr() {
    return newAddr;
  }

  public void addLine(ActionEvent e) {

    if (newAddr == null || newAddr.length() == 0) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.fraud.empty.address");
      return;
    }
    try {
      Address addr = new Address(newAddr);
      if(addresses.contains(addr)) {
          addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.fraud.duplicate.address",newAddr);
          return;
      }
      addresses.add(addr);
      frSettings.setWhiteList(addresses);
      setSettings(frSettings);
      viewChanges();
    }
    catch (Exception ex) {
      addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.fraud.invalid.address",newAddr);
      return;
    }

  }

  private void checkOutOfDate() {
    try {
      List<Integer> result = new ArrayList<Integer>();
      SmscStatusManager ssm = getSmscStatusManager();
      for (int i = 0; i < ssm.getSmscInstancesNumber(); i++) {
        if (ssm.getFraudConfigState(i) == SmscConfigurationStatus.OUT_OF_DATE)
          result.add(i);
      }
      if (!result.isEmpty())
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "smsc.config.instance.out_of_date", result.toString());
    } catch (AdminException e) {
      logger.error(e, e);
      addError(e);
    }
  }
}
