package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.components.data_table.model.EmptyDataTableModel;

import javax.faces.event.ActionEvent;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryListController extends DeliveryController {

  private String namePrefix;

  private String userFilter;

  private String status;

  private String idFilter;

  private boolean init = true;

  private List<String> selected;

  private final static int MEMORY_LIMIT = 1000;

  private Date startDateFrom;
  private Date startDateTo;

  public DeliveryListController() {
    String i = getRequestParameter("init");
    if (i != null && i.length() > 0) {
      init = Boolean.valueOf(i);
    }
    startDateFrom = new Date(System.currentTimeMillis() - (7*24*60*60*1000));
    startDateTo = null;
  }

  public String getNamePrefix() {
    return namePrefix;
  }

  public void setNamePrefix(String namePrefix) {
    this.namePrefix = namePrefix;
  }

  public String getUserFilter() {
    return userFilter;
  }

  public void setUserFilter(String userFilter) {
    this.userFilter = userFilter;
  }

  public String getStatus() {
    return status;
  }

  public void setStatus(String status) {
    this.status = status;
  }

  public String getIdFilter() {
    return idFilter;
  }

  public void setIdFilter(String idFilter) {
    this.idFilter = idFilter;
  }

  public void clearFilter() {
    userFilter = null;
    status = null;
    namePrefix = null;
    init = false;
    startDateFrom = new Date(System.currentTimeMillis() - (7*24*60*60*1000));
    startDateTo = null;
    idFilter = null;
  }

  public void query() {
    init = true;
  }

  public Date getStartDateFrom() {
    return startDateFrom;
  }

  public void setStartDateFrom(Date startDateFrom) {
    this.startDateFrom = startDateFrom;
  }

  public Date getStartDateTo() {
    return startDateTo;
  }

  public void setStartDateTo(Date startDateTo) {
    this.startDateTo = startDateTo;
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }


  @SuppressWarnings({"unchecked"})
  public void setSelected(List selected) {
    if (selected != null) {
      this.selected = new ArrayList<String>((List<String>) selected);
    }
  }

  public String getSelectedAsString() {
    if (selected == null || selected.isEmpty())
      return "";

    StringBuilder sb = new StringBuilder();
    for (String s : selected) {
      if (sb.length() > 0)
        sb.append(',');
      sb.append(s);
    }

    return sb.toString();
  }

  public String activate() {
    if (selected != null) {
      User u = config.getUser(getUserName());
      for (String r : selected) {
        try {
          int id = Integer.parseInt(r);
          config.activateDelivery(u.getLogin(), u.getPassword(), id);
        } catch (AdminException e) {
          addError(e);
        }
      }
      selected = null;
    }
    return null;
  }

  public String delete() {
    if (selected != null) {
      User u = config.getUser(getUserName());
      for (String r : selected) {
        try {
          int id = Integer.parseInt(r);
          config.dropDelivery(u.getLogin(), u.getPassword(), id);
        } catch (AdminException e) {
          addError(e);
        }
      }
      selected = null;
    }
    return null;
  }

  public String editGroup() {
    if(selected == null || selected.isEmpty()) {
      return null;
    }
    getRequest().put(DELIVERY_IDS_PARAM, selected);
    selected = null;
    return "DELIVERY_EDIT_GROUP";
  }

  public void statForSelected(ActionEvent e) {
    if(selected == null || selected.isEmpty())
      return;

    getRequest().put("delivery", getSelectedAsString());
    selected = null;
  }

  public String pause() {
    if (selected != null) {
      User u = config.getUser(getUserName());
      for (String r : selected) {
        try {
          int id = Integer.parseInt(r);
          config.pauseDelivery(u.getLogin(), u.getPassword(), id);
        } catch (AdminException e) {
          addError(e);
        }
      }
    }
    selected = null;
    return null;
  }

  /**
   * Вставляет объект info в отсортированный массив infos.
   *
   * @param infos отсортированный массив
   * @param info  объект для вставки
   * @param c     компаратор, при помощи которого отсортирован массив
   * @param <T>   тип элементов массива
   */
  private static <T> void insert(T[] infos, T info, Comparator<T> c) {
    int sRes = Arrays.binarySearch(infos, info, c);

    if (sRes < 0)
      sRes = -sRes - 1;

    if (infos[sRes] != null)
      System.arraycopy(infos, sRes, infos, sRes + 1, infos.length - sRes - 1);

    infos[sRes] = info;
  }

  private DeliveryFilter createFilter(User u) {
    DeliveryFilter filter = new DeliveryFilter();

    if (userFilter != null && (userFilter = userFilter.trim()).length() != 0)
      filter.setUserIdFilter(userFilter);
    else if (!isUserInAdminRole())
      filter.setUserIdFilter(u.getLogin());

    if (namePrefix != null && (namePrefix = namePrefix.trim()).length() != 0)
      filter.setNameFilter(namePrefix);

    if (status != null && status.length() > 0)
      filter.setStatusFilter(DeliveryStatus.valueOf(status));

    if(startDateFrom != null) {
      filter.setStartDateFrom(startDateFrom);
    }

    if(startDateTo != null) {
      filter.setStartDateTo(startDateTo);
    }

    return filter;
  }



  private List<Delivery> getSortedDeliveriesList(User u, DeliveryFilter filter, final int startPos, final int count, DataTableSortOrder sortOrder) throws AdminException {
    final Comparator<Delivery> comparator = getComparator(sortOrder);

    final Delivery infos[] = new Delivery[startPos + count];
    final int lastIdx = infos.length - 1;

    config.getDeliveries(u.getLogin(), u.getPassword(), filter, MEMORY_LIMIT, new Visitor<Delivery>() {
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
    config.getDeliveries(u.getLogin(), u.getPassword(), filter, MEMORY_LIMIT, new Visitor<Delivery>() {
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

  private static boolean accept(Delivery delivery, DeliveryFilter filter) {
    if(filter.getEndDateFrom()!= null && delivery.getEndDate() != null && delivery.getEndDate().before(filter.getEndDateFrom())) {
      return false;
    }
    if(filter.getEndDateTo()!= null && delivery.getEndDate() != null && delivery.getEndDate().after(filter.getEndDateTo())) {
      return false;
    }
    if(filter.getNameFilter() != null) {
      boolean accept = false;
      for(String s : filter.getNameFilter()) {
        if(delivery.getName().contains(s)) {
          accept = true;
          break;
        }
      }
      if(!accept) {
        return false;
      }
    }
    if(filter.getStartDateFrom() != null && delivery.getStartDate().before(filter.getStartDateFrom())) {
      return false;
    }
    if(filter.getStartDateTo() != null && delivery.getStartDate().after(filter.getStartDateTo())) {
      return false;
    }
    if(filter.getStatusFilter() != null) {
      boolean accept = false;
      for(DeliveryStatus s : filter.getStatusFilter()) {
        if(s.equals(delivery.getStatus())) {
          accept = true;
          break;
        }
      }
      if(!accept) {
        return false;
      }
    }
    if(filter.getUserIdFilter() != null) {
      boolean accept = false;
      for(String s : filter.getUserIdFilter()) {
        if(s.equals(delivery.getOwner())) {
          accept = true;
          break;
        }
      }
      if(!accept) {
        return false;
      }
    }
    return true;
  }

  public DataTableModel getDeliviries() {

    if (!init)
      return new EmptyDataTableModel();

    final User u = config.getUser(getUserName());

    final DeliveryFilter filter = createFilter(u);

    if(idFilter != null && idFilter.length()>0) {

      Integer id = Integer.parseInt(idFilter);
      final Delivery[] delivery = new Delivery[]{null};
      final DeliveryStatistics[] stats = new DeliveryStatistics[]{null};
      try {
        Delivery d = config.getDelivery(u.getLogin(), u.getPassword(), id);
        if(accept(d, filter)) {
          delivery[0] = d;
          stats[0] = config.getDeliveryStats(u.getLogin(), u.getPassword(), delivery[0].getId());
        }
      } catch (DeliveryException e){
        if(e.getErrorStatus() != DeliveryException.ErrorStatus.NoSuchEntry) {
          addError(e);
        }
      } catch (AdminException e) {
        addError(e);
      }
      return new DataTableModel() {
        public List getRows(int startPos, int count, DataTableSortOrder sortOrder) {
          List<DeliveryRow> rows = new ArrayList<DeliveryRow>(1);
          if(delivery[0] != null) {
            rows.add(new DeliveryRow(delivery[0], stats[0]));
          }
          return rows;
        }
        public int getRowsCount() {
          return delivery[0] == null ? 0 : 1;
        }
      };

    }else {

      return new DataTableModel() {
        public List getRows(final int startPos, final int count, DataTableSortOrder sortOrder) {
          try {
            List<Delivery> list;
            if (sortOrder != null)
              list = getSortedDeliveriesList(u, filter, startPos, count, sortOrder);
            else
              list = getUnsortedDeliveriesList(u, filter, startPos, count);

            List<DeliveryRow> rows = new ArrayList<DeliveryRow>(list.size());
            for (Delivery di : list)
              if (di != null) {
                DeliveryStatistics stats = config.getDeliveryStats(u.getLogin(), u.getPassword(), di.getId());
                rows.add(new DeliveryRow(di, stats));
              }
            return rows;

          } catch (AdminException e) {
            addError(e);
          }

          return Collections.emptyList();
        }

        public int getRowsCount() {
          try {
            return config.countDeliveries(u.getLogin(), u.getPassword(), filter);
          } catch (AdminException e) {
            addError(e);
            return 0;
          }
        }
      };
    }
  }

  private static Comparator<Delivery> getComparator(final DataTableSortOrder sortOrder) {
    if (sortOrder == null || sortOrder.getColumnId().equals("name")) {
      return new Comparator<Delivery>() {
        public int compare(Delivery o1, Delivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getName().compareTo(o2.getName()) * (sortOrder == null || sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("userId")) {
      return new Comparator<Delivery>() {
        public int compare(Delivery o1, Delivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getOwner().compareTo(o2.getOwner()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("status")) {
      return new Comparator<Delivery>() {
        public int compare(Delivery o1, Delivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getStatus().toString().compareTo(o2.getStatus().toString()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("startDate")) {
      return new Comparator<Delivery>() {
        public int compare(Delivery o1, Delivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getStartDate().compareTo(o2.getStartDate()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("id")) {
      return new Comparator<Delivery>() {
        public int compare(Delivery o1, Delivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getId().compareTo(o2.getId()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else {
      return new Comparator<Delivery>() {
        public int compare(Delivery o1, Delivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          if (o1.getEndDate() == null) {
            return o2.getEndDate() == null ? 0 : (sortOrder.isAsc() ? -1 : 1);
          } else if (o2.getEndDate() == null) {
            return (sortOrder.isAsc() ? 1 : -1);
          }
          return o1.getEndDate().compareTo(o2.getEndDate()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    }
  }


  public class DeliveryRow {

    private final Delivery delivery;

    private final DeliveryStatistics stats;

    public DeliveryRow(Delivery delivery, DeliveryStatistics stats) {
      this.delivery = delivery;
      this.stats = stats;
    }

    public Integer getId() {
      return delivery.getId();
    }

    public Date getEndDate() {
      return delivery.getEndDate();
    }

    public Date getStartDate() {
      return delivery.getStartDate();
    }

    public DeliveryStatus getStatus() {
      return delivery.getStatus();
    }

    public String getUserId() {
      return delivery.getOwner();
    }

    public String getName() {
      return delivery.getName();
    }

    public int getDeliveryId() {
      return delivery.getId();
    }

    public boolean isRenderProgress() {
      return delivery.getStatus() == DeliveryStatus.Active;
    }

    public int getTotal() {
      return (int)(stats.getDeliveredMessages()+stats.getExpiredMessages()+
          stats.getProcessMessages()+stats.getNewMessages()+stats.getFailedMessages());
    }

    public int getFinalized() {
      return (int)(stats.getDeliveredMessages()+stats.getExpiredMessages()+stats.getFailedMessages());
    }

    public long getNewMessages() {
      return stats.getNewMessages();
    }

    public long getProcessMessages() {
      return stats.getProcessMessages();
    }

    public long getDeliveredMessages() {
      return stats.getDeliveredMessages();
    }

    public long getFailedMessages() {
      return stats.getFailedMessages();
    }

    public long getExpiredMessages() {
      return stats.getExpiredMessages();
    }
  }


}