package mobi.eyeline.informer.web.controllers.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryFilter;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.smppgw.SmppGWProvider;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.components.data_table.model.ModelException;

import javax.faces.model.SelectItem;
import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class ChooseDeliveryPage extends ProviderEditPage{

  private SmppGWProvider provider;

  private String nameFilter;

  private String userFilter;

  private final boolean isNew;

  public void apply() {

  }

  public void clear() {
    nameFilter = null;
    userFilter = null;
  }


  ChooseDeliveryPage(SmppGWProvider provider, boolean aNew) {
    this.provider = provider;
    isNew = aNew;
  }

  @Override
  public ProviderEditPage nextPage() throws AdminException {
    String delivery = getRequestParameter("delivery");
    System.out.println("Delivery:" + delivery);
    return new RouteSettingsPage(provider, Integer.parseInt(delivery), isNew);
  }

  @Override
  public ProviderEditPage backPage() throws AdminException {
    return null;
  }

  @Override
  public String getPageId() {
    return "CHOOSE_DELIVERY";
  }

  public String getNameFilter() {
    return nameFilter;
  }

  public void setNameFilter(String nameFilter) {
    this.nameFilter = nameFilter == null || nameFilter.length() == 0 ? null : nameFilter;
  }

  public String getUserFilter() {
    return userFilter;
  }

  public void setUserFilter(String userFilter) {
    this.userFilter = userFilter == null || userFilter.length() == 0 ? null : userFilter;
  }

  public List<SelectItem> getUniqueUsers() {
    List<SelectItem> ss = new LinkedList<SelectItem>();
    for(User u : getConfig().getUsers()) {
      ss.add(new SelectItem(u.getLogin(),u.getLogin()));
    }
    Collections.sort(ss, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return ss;
  }


  private List<Delivery> getSortedDeliveriesList(final User u, DeliveryFilter filter, final int startPos, final int count, final DataTableSortOrder sortOrder, final boolean calculateDates) throws AdminException {
    final Comparator<Delivery> comparator = getComparator(sortOrder);

    final Delivery infos[] = new Delivery[startPos + count];
    final int lastIdx = infos.length - 1;

    getConfig().getDeliveries(u.getLogin(), filter, 1000, new Visitor<Delivery>() {
      public boolean visit(Delivery value) throws AdminException {
        if (infos[lastIdx] == null || comparator.compare(value, infos[lastIdx]) < 0)
          insert(infos, value, comparator);

        return true;
      }
    });

    return Arrays.asList(infos).subList(startPos, startPos + count);
  }

  private List<Delivery> getUnsortedDeliveriesList(User u, DeliveryFilter filter, final int startPos, final int count) throws AdminException {
    final Delivery infos[] = new Delivery[count];
    getConfig().getDeliveries(u.getLogin(), filter, 1000, new Visitor<Delivery>() {
      int pos = 0;

      public boolean visit(Delivery value) throws AdminException {
        if (pos >= startPos + count)
          return false;

        if (pos >= startPos)
          infos[pos - startPos] = value;

        pos++;
        return true;
      }
    });
    return Arrays.asList(infos);
  }

  private static Comparator<Delivery> getComparator(final DataTableSortOrder sortOrder) {
    if (sortOrder.getColumnId().equals("name")) {
      return new Comparator<Delivery>() {
        public int compare(Delivery o1, Delivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getName().compareTo(o2.getName()) * ( sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else {
      return new Comparator<Delivery>() {
        public int compare(Delivery o1, Delivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getOwner().compareTo(o2.getOwner()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    }
  }

  private static <T> void insert(T[] infos, T info, Comparator<T> c) {
    int sRes = Arrays.binarySearch(infos, info, c);

    if (sRes < 0)
      sRes = -sRes - 1;

    if (infos[sRes] != null)
      System.arraycopy(infos, sRes, infos, sRes + 1, infos.length - sRes - 1);

    infos[sRes] = info;
  }

  private DeliveryFilter createFilter() {
    DeliveryFilter filter = new DeliveryFilter();
    if(userFilter != null) {
      filter.setUserIdFilter(userFilter);
    }
    if(nameFilter != null) {
      filter.setNameFilter(nameFilter);
    }
    return filter;
  }

  public DataTableModel getDeliveries() {

    final User u = getConfig().getUser(getUserName());

    final DeliveryFilter filter = createFilter();

    return new DataTableModel() {
      public List getRows(final int startPos, final int count, DataTableSortOrder sortOrder) throws ModelException {
        try {
          final boolean calculateDates = sortOrder != null && sortOrder.getColumnId().equals("startDate");

          List<Delivery> list;
          if (sortOrder != null)
            list = getSortedDeliveriesList(u, filter, startPos, count, sortOrder, calculateDates);
          else
            list = getUnsortedDeliveriesList(u, filter, startPos, count);

          List<DeliveryRow> rows = new ArrayList<DeliveryRow>(list.size());
          for (Delivery di : list) {
            if(di != null) {
              rows.add(new DeliveryRow(di));
            }
          }
          return rows;

        } catch (AdminException e) {
          throw new ModelException(e.getMessage(getLocale()));
        }
      }

      public int getRowsCount() throws ModelException{
        try {
          return getConfig().countDeliveries(u.getLogin(), filter);
        } catch (AdminException e) {
          throw new ModelException(e.getMessage(getLocale()));
        }
      }
    };
  }

  public static class DeliveryRow {

    private int id;
    private String name;
    private String user;

    public DeliveryRow(Delivery di) {
      id = di.getId();
      name = di.getName();
      user = di.getOwner();
    }

    public int getId() {
      return id;
    }

    public String getName() {
      return name;
    }

    public String getUser() {
      return user;
    }
  }
}
