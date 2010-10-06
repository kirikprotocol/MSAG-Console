package ru.novosoft.smsc.web.controllers.service;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.sme.SmeConnectType;
import ru.novosoft.smsc.admin.sme.SmeServiceStatus;
import ru.novosoft.smsc.admin.sme.SmeSmscStatus;
import ru.novosoft.smsc.admin.sme.SmeSmscStatuses;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;

import javax.faces.model.SelectItem;
import java.io.Serializable;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class ServiceListController extends ServiceController {

  private List selectedRows;
  private Map<String, String> switchToHost = new HashMap<String, String>();

  public ServiceListController() {
  }

  public void setSelectedRows(List selectedRows) {
    this.selectedRows = selectedRows;
  }

  public Map<String, String> getSwitchToHost() {
    return switchToHost;
  }

  public String switchSme() {
    String smeId = getRequestParameter("smeId");
    try {
      mngr.switchSme(smeId, switchToHost.get(smeId));
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String stopSme() {
    String smeId = getRequestParameter("smeId");
    try {
      mngr.stopSme(smeId);
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String startSme() {
    String smeId = getRequestParameter("smeId");
    try {
      mngr.startSme(smeId);
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String disconnectSmes() {
    if (selectedRows != null) {
      try {
        mngr.disconnectSmeFromSmsc(selectedRows);
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public String deleteSmes() {
    if (selectedRows != null) {
      try {
        for (Object smeId : selectedRows)
          mngr.removeSme((String) smeId);
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }


  public DataTableModel getServices() {
    final List<String> smeIds = new ArrayList<String>();
    try {
      smeIds.addAll(mngr.smes().keySet());
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
        try {
          Map<String, SmeSmscStatuses> smeSmscStats = mngr.getSmesSmscStatuses();
          // Заполняем список
          for (int i = startPos; i < Math.min(startPos + count, smeIds.size()); i++) {
            String smeId = smeIds.get(i);
            SmeServiceStatus smeServiceStatus = mngr.getSmeServiceStatus(smeId);
            SmeSmscStatuses smeSmscStatuses = smeSmscStats.get(smeId);
            result.add(new Esme(smeId, smeServiceStatus, smeSmscStatuses));
          }
        } catch (AdminException e) {
          addError(e);
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
