package mobi.eyeline.informer.web.controllers.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smppgw.SmppGWEndpoint;
import mobi.eyeline.informer.admin.smppgw.SmppGWEndpointSettings;
import mobi.eyeline.informer.web.components.data_table.model.*;
import mobi.eyeline.informer.web.controllers.InformerController;

import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWEndpointsController extends InformerController {

  private String namePrefix;

  private List selected;

  public List getSelected() {
    return selected;
  }

  public void setSelected(List selected) {
    this.selected = selected;
  }

  public void apply() {
  }

  public void clear() {
    namePrefix = null;
  }


  public String getNamePrefix() {
    return namePrefix;
  }

  public void setNamePrefix(String namePrefix) {
    this.namePrefix = namePrefix == null || namePrefix.length() == 0 ? null : namePrefix;
  }

  public String remove() {
    if(selected != null && !selected.isEmpty()) {
      SmppGWEndpointSettings settings = getConfig().getSmppGWEndpointSettings();
      for(Object o : selected) {
        settings.removeEndpoint((String)o);
      }
      try {
        getConfig().updateSmppGWSettings(settings, getUserName());
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }


  public DataTableModel getConnections() {
    if(!getConfig().isSmppGWDeployed()) {
      return new EmptyDataTableModel();
    }

    SmppGWEndpointSettings ss = getConfig().getSmppGWEndpointSettings();

    final List<SmppGWEndpoint> es = new LinkedList<SmppGWEndpoint>();
    for(SmppGWEndpoint e : ss.getEndpoints()) {
      if(namePrefix != null && !e.getName().startsWith(namePrefix)) {
        continue;
      }
      es.add(e);
    }


    return new ModelWithObjectIds() {
      @Override
      public String getId(Object o) throws ModelException {
        return ((SmppGWEndpoint)o).getName();
      }

      @Override
      public List getRows(int startPos, int count, DataTableSortOrder sortOrder) throws ModelException {
        if(sortOrder != null) {
          Collections.sort(es, getComparator(sortOrder));
        }
        List<SmppGWEndpoint>  result = new LinkedList<SmppGWEndpoint>();
        for (Iterator<SmppGWEndpoint> i = es.iterator(); i.hasNext() && count > 0;) {
          SmppGWEndpoint r = i.next();
          if (--startPos < 0) {
            result.add(r);
            count--;
          }
        }
        return result;
      }

      @Override
      public int getRowsCount() throws ModelException {
        return es.size();
      }
    };
  }

  private static Comparator<SmppGWEndpoint> getComparator(final DataTableSortOrder sortOrder) {
    return new Comparator<SmppGWEndpoint>() {
      @Override
      public int compare(SmppGWEndpoint o1, SmppGWEndpoint o2) {
        int asc = sortOrder.isAsc() ? 1 : -1;
        if (sortOrder.getColumnId() == null || sortOrder.getColumnId().equals("name")) {
          return (o1.getName().compareTo(o2.getName())) * asc;
        } else if (sortOrder.getColumnId().equals("systemId")) {
          return (o1.getSystemId().compareTo(o2.getSystemId())) * asc;
        }
        return 1;
      }
    };
  }

}
