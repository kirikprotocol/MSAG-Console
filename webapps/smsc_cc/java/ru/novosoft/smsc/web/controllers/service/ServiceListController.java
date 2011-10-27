package ru.novosoft.smsc.web.controllers.service;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import mobi.eyeline.util.jsf.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.util.jsf.components.data_table.model.ModelException;
import mobi.eyeline.util.jsf.components.data_table.model.ModelWithObjectIds;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.sme.*;

import javax.faces.model.SelectItem;
import java.io.Serializable;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class ServiceListController extends ServiceController {

  private List selectedRows;

  private String nameFilter;

  public ServiceListController() {
  }

  public void apply() {
  }

  public void clear() {
    nameFilter = null;
  }

  public String getNameFilter() {
    return nameFilter;
  }

  public void setNameFilter(String nameFilter) {
    this.nameFilter = nameFilter == null || (nameFilter = nameFilter.trim()).length() == 0 ? null : nameFilter;
  }

  public void setSelectedRows(List selectedRows) {
    this.selectedRows = selectedRows;
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
      for(String s : mngr.smes().keySet()) {
        if(nameFilter == null || s.startsWith(nameFilter)) {
          smeIds.add(s);
        }
      }
    } catch (AdminException e) {
      addError(e);
    }

    return new ModelWithObjectIds() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) throws ModelException {

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
            Sme sme = mngr.getSme(smeId);
            SmeSmscStatuses smeSmscStatuses = smeSmscStats.get(smeId);
            result.add(new Esme(smeId, smeSmscStatuses, sme.isDisabled()));
          }
        } catch (AdminException e) {
          throw new ModelException(e.getMessage(getLocale()));
        }

        return result;
      }

      public int getRowsCount() {
        return smeIds.size();
      }

      public String getId(Object o) throws ModelException {
        return ((Esme)o).getSystemId();
      }
    };
  }


  public static class Esme implements Serializable {

    private String systemId;
    private SmeSmscStatuses connectStatuses;
    private boolean disabled;

    public Esme(String systemId, SmeSmscStatuses connectStatuses, boolean disabled) {
      this.systemId = systemId;
      this.connectStatuses = connectStatuses;
      this.disabled = disabled;
    }

    public String getSystemId() {
      return systemId;
    }

    public SmeSmscStatuses getConnectStatuses() {
      return connectStatuses;
    }

    public boolean isConnectedViaLoadBalancer() {
      return connectStatuses != null && connectStatuses.getConnectType() == SmeConnectType.VIA_LOAD_BALANCER;
    }

    public boolean isConnectedDirectly() {
      if (connectStatuses == null ||  connectStatuses.getConnectType() != SmeConnectType.DIRECT_CONNECT)
        return false;

      SmeSmscStatus[] statuses = connectStatuses.getStatuses();
      for (SmeSmscStatus st : statuses)
        if (st.getConnectionStatus() == SmeConnectStatus.CONNECTED)
          return true;
      return false;
    }

    public boolean isInternal() {
      SmeSmscStatus[] statuses = connectStatuses.getStatuses();
      for (SmeSmscStatus st : statuses)
        if (st.getConnectionStatus() == SmeConnectStatus.INTERNAL)
          return true;
      return false;
    }

    public boolean isConnected() {
      return isConnectedDirectly() || isConnectedViaLoadBalancer() || isInternal();
    }

    public String getLoadBalancerStatus() {
      if (!isConnectedViaLoadBalancer())
        return null;

      return connectStatuses.getStatuses()[0].getBindMode() + " " + connectStatuses.getStatuses()[0].getPeerOut();
    }

    public List<String> getDirectStatuses() {
      List<String> result = new ArrayList<String>();

      SmeSmscStatus[] statuses = connectStatuses.getStatuses();
      for (SmeSmscStatus st : statuses) {
        if (st.getConnectionStatus() == SmeConnectStatus.CONNECTED)
          result.add(st.getSmscInstanceNumber() + ": " + st.getBindMode() + " " + st.getPeerOut());
      }
      return result;
    }

    public boolean isDisabled() {
      return disabled;
    }
  }
}
