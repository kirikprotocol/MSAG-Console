package mobi.eyeline.informer.web.controllers.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.smppgw.SmppGWProvider;
import mobi.eyeline.informer.admin.smppgw.SmppGWProviderSettings;
import mobi.eyeline.informer.admin.smppgw.SmppGWRoute;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.web.components.data_table.model.*;
import mobi.eyeline.informer.web.controllers.InformerController;

import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWProvidersController extends InformerController{

  private String namePrefix;

  private Address address;

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
    address = null;
  }


  public String getNamePrefix() {
    return namePrefix;
  }

  public void setNamePrefix(String namePrefix) {
    this.namePrefix = namePrefix == null || namePrefix.length() == 0 ? null : namePrefix;
  }

  public Address getAddress() {
    return address;
  }

  public void setAddress(Address address) {
    this.address = address;
  }

  public String remove() {
    if(selected != null && !selected.isEmpty()) {
      SmppGWProviderSettings settings = getConfig().getSmppGWProviderSettings();
      for(Object o : selected) {
        settings.removeProvider((String)o);
      }
      try {
        getConfig().updateSmppGWSettings(settings);
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }


  private List<SmppGWProvider> getFilteredProviders(SmppGWProviderSettings ss) {
    final List<SmppGWProvider> es = new LinkedList<SmppGWProvider>();
    for(SmppGWProvider e : ss.getProviders()) {
      if(namePrefix != null && !e.getName().startsWith(namePrefix)) {
        continue;
      }
      if(address != null) {
        boolean accept = false;
        for(SmppGWRoute r : e.getRoutes()) {
          for(Address a : r.getServiceNumbers()) {
            if(a.equals(address)) {
              accept = true;
              break;
            }
            if(accept) {
              break;
            }
          }
        }
        if(!accept) {
          continue;
        }
      }
      es.add(e);
    }
    return es;
  }

  public DataTableModel getProviders() {
    if(!getConfig().isSmppGWDeployed()) {
      return new EmptyDataTableModel();
    }
    SmppGWProviderSettings ss = getConfig().getSmppGWProviderSettings();

    final List<SmppGWProvider> es = getFilteredProviders(ss);

    return new ModelWithObjectIds() {
      @Override
      public String getId(Object o) throws ModelException {
        return ((SmppGWProvider)o).getName();
      }

      @Override
      public List getRows(int startPos, int count, DataTableSortOrder sortOrder) throws ModelException {
        if(sortOrder != null) {
          Collections.sort(es, getComparator(sortOrder));
        }
        List<SmppGWProvider>  result = new LinkedList<SmppGWProvider>();
        for (Iterator<SmppGWProvider> i = es.iterator(); i.hasNext() && count > 0;) {
          SmppGWProvider r = i.next();
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

  private static Comparator<SmppGWProvider> getComparator(final DataTableSortOrder sortOrder) {
    return new Comparator<SmppGWProvider>() {
      @Override
      public int compare(SmppGWProvider o1, SmppGWProvider o2) {
        int asc = sortOrder.isAsc() ? 1 : -1;
        if (sortOrder.getColumnId() == null || sortOrder.getColumnId().equals("name")) {
          return (o1.getName().compareTo(o2.getName())) * asc;
        }
        return 1;
      }
    };
  }

}
