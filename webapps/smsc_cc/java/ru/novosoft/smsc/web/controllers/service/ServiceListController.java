package ru.novosoft.smsc.web.controllers.service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.sme.SmeConnectType;
import ru.novosoft.smsc.admin.sme.SmeServiceStatus;
import ru.novosoft.smsc.admin.sme.SmeSmscStatus;
import ru.novosoft.smsc.admin.sme.SmeSmscStatuses;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.config.Configuration;
import ru.novosoft.smsc.web.config.SmscStatusManager;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.application.FacesMessage;
import javax.faces.model.SelectItem;
import java.io.Serializable;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class ServiceListController extends ServiceController {

  private List selectedRows;
  private Map<String, String> switchToHost = new HashMap<String, String>();

  public void setSelectedRows(List selectedRows) {
    this.selectedRows = selectedRows;
  }

  public Map<String, String> getSwitchToHost() {
    return switchToHost;
  }

  public String switchSme() {
    String smeId = getRequestParameter("smeId");
    try {
      getConfiguration().switchSme(smeId, switchToHost.get(smeId), getUserName());
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String stopSme() {
    String smeId = getRequestParameter("smeId");
    try {
      getConfiguration().stopSme(smeId, getUserName());
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String startSme() {
    String smeId = getRequestParameter("smeId");
    try {
      getConfiguration().startSme(smeId, getUserName());
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String disconnectSmes() {
    if (selectedRows != null) {
      try {
        getConfiguration().disconnectSmeFromSmsc(selectedRows, getUserName());
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String deleteSmes() {
    if (selectedRows != null) {
      try {
        Configuration conf = getConfiguration();
        String userName = getUserName();
        for (Object smeId : selectedRows)
          conf.removeSme((String) smeId, userName);
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }



  public DataTableModel getServices() {
    final Configuration conf = getConfiguration();
    final List<String> smeIds = new ArrayList<String>();
    try {
      smeIds.addAll(conf.smes().keySet());
    } catch (AdminException e) {
      addError(e);
    }

    return new DataTableModel() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        if (sortOrder != null && sortOrder.getColumnId().equals("systemId")) {
          // Сортируем список
          Collections.sort(smeIds, new Comparator<String>() {
            public int compare(String o1, String o2) {
              int res = o1.compareTo(o2);
              return (sortOrder.isAsc()) ? res : -res;
            }
          });
        }

        List<Esme> result = new ArrayList<Esme>(count);
        // Заполняем список
        for (int i = startPos; i < Math.min(startPos + count, smeIds.size()); i++) {
          String smeId = smeIds.get(i);
          try {
            SmeServiceStatus smeServiceStatus = conf.getSmeServiceStatus(smeId);
            SmeSmscStatuses smeSmscStatuses = conf.getSmesSmscStatuses().get(smeId);
            result.add(new Esme(smeId, smeServiceStatus, smeSmscStatuses));
          } catch (AdminException e) {
            addError(e);
          }
        }

        return result;
      }

      public int getRowsCount() {
        return smeIds.size();
      }
    };
  }


  public static class Esme implements Serializable {

    private String systemId;
    private SmeServiceStatus smeServiceStatus;
    private SmeSmscStatuses connectStatuses;

    public Esme(String systemId, SmeServiceStatus smeServiceStatus, SmeSmscStatuses connectStatuses) {
      this.systemId = systemId;
      this.smeServiceStatus = smeServiceStatus;
      this.connectStatuses = connectStatuses;
    }

    public String getSystemId() {
      return systemId;
    }

    public List<SelectItem> getHosts() {
      if (smeServiceStatus != null) {
        List<SelectItem> res = new ArrayList<SelectItem>();
        for (String host : smeServiceStatus.getHosts())
          res.add(new SelectItem(host));
        return res;
      } else
        return null;
    }

    public String getOnlineHost() {
      if (smeServiceStatus != null)
        return smeServiceStatus.getOnlineHost();
      else
        return null;
    }

    public SmeSmscStatuses getConnectStatuses() {
      return connectStatuses;
    }

    public boolean isConnectedViaLoadBalancer() {
      return connectStatuses.getConnectType() == SmeConnectType.VIA_LOAD_BALANCER;
    }

    public String getLoadBalancerStatus() {
      if (!isConnectedViaLoadBalancer())
        return null;

      return connectStatuses.getStatuses()[0].getBindMode() + " " + connectStatuses.getStatuses()[0].getPeerOut();
    }

    public List<String> getDirectStatuses() {
      if (!isConnectedViaLoadBalancer())
        return null;

      List<String> result = new ArrayList<String>();

      SmeSmscStatus[] statuses = connectStatuses.getStatuses();
      for (SmeSmscStatus st : statuses)
        result.add(st.getSmscInstanceNumber() + ": " + st.getBindMode() + " " + st.getPeerOut());
      return result;
    }

    /**
     * Возвращает true, если Sme можно запускать и останавливать средствами морды
     *
     * @return true, если Sme можно запускать и останавливать средствами морды
     */
    public boolean isManaged() {
      return smeServiceStatus != null;
    }

    public boolean isSwitchAllowed() {
      return isManaged() && smeServiceStatus.getHosts().size() > 1;
    }
  }
}
