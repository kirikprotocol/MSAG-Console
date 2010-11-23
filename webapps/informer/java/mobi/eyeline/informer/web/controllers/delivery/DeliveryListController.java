package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.components.data_table.model.EmptyDataTableModel;

import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryListController extends DeliveryController {

  private String namePrefix;

  private String userFilter;

  private String status;

  private boolean init = false;

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
    startDateTo = new Date();
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

  public void clearFilter() {
    userFilter = null;
    status = null;
    namePrefix = null;
    init = false;
    startDateFrom = new Date(System.currentTimeMillis() - (7*24*60*60*1000));
    startDateTo = new Date();
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


  public String activate() {
    if (selected != null) {
      User u = config.getUser(getUserName());
      for (String r : selected) {
        try {
          int id = Integer.parseInt(r);
          DeliveryStatistics d = config.getDeliveryStats(u.getLogin(), u.getPassword(), id);
          if (d.getDeliveryState() != null) {
            DeliveryStatus status = d.getDeliveryState().getStatus();
            if (status == DeliveryStatus.Planned || status == DeliveryStatus.Paused) {
              config.activateDelivery(u.getLogin(), u.getPassword(), id);
            }
          }
        } catch (AdminException e) {
          addError(e);
        }
      }
    }
    return null;
  }

  public String editGroup() {
    if(selected == null || selected.isEmpty()) {
      return null;
    }
    getRequest().put(DELIVERY_IDS_PARAM, selected);
    return "DELIVERY_EDIT_GROUP";
  }

  public String pause() {
    if (selected != null) {
      User u = config.getUser(getUserName());
      for (String r : selected) {
        try {
          int id = Integer.parseInt(r);
          DeliveryStatistics d = config.getDeliveryStats(u.getLogin(), u.getPassword(), id);
          if (d.getDeliveryState() != null) {
            DeliveryStatus status = d.getDeliveryState().getStatus();
            if (status == DeliveryStatus.Active) {
              config.pauseDelivery(u.getLogin(), u.getPassword(), id);
            }
          }
        } catch (AdminException e) {
          addError(e);
        }
      }
    }
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

    filter.setResultFields(DeliveryFields.Name, DeliveryFields.UserId, DeliveryFields.Status, DeliveryFields.StartDate, DeliveryFields.EndDate);

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



  private List<DeliveryInfo> getSortedDeliveriesList(User u, DeliveryFilter filter, final int startPos, final int count, DataTableSortOrder sortOrder) throws AdminException {
    final Comparator<DeliveryInfo> comparator = getComparator(sortOrder);

    final DeliveryInfo infos[] = new DeliveryInfo[startPos + count];
    final int lastIdx = infos.length - 1;

    config.getDeliveries(u.getLogin(), u.getPassword(), filter, MEMORY_LIMIT, new Visitor<DeliveryInfo>() {
      public boolean visit(DeliveryInfo value) throws AdminException {
        if (infos[lastIdx] == null || comparator.compare(value, infos[lastIdx]) < 0)
          insert(infos, value, comparator);

        return true;
      }
    });

    return Arrays.asList(infos).subList(startPos, startPos + count);
  }

  private List<DeliveryInfo> getUnsortedDeliveriesList(User u, DeliveryFilter filter, final int startPos, final int count) throws AdminException {
    final DeliveryInfo infos[] = new DeliveryInfo[count];
    config.getDeliveries(u.getLogin(), u.getPassword(), filter, MEMORY_LIMIT, new Visitor<DeliveryInfo>() {
      int pos = 0;

      public boolean visit(DeliveryInfo value) throws AdminException {
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

  public DataTableModel getDeliviries() {

    if (!init)
      return new EmptyDataTableModel();

    final User u = config.getUser(getUserName());
    final DeliveryFilter filter = createFilter(u);

    return new DataTableModel() {
      public List getRows(final int startPos, final int count, DataTableSortOrder sortOrder) {
        try {
          List<DeliveryInfo> list;
          if (sortOrder != null)
            list = getSortedDeliveriesList(u, filter, startPos, count, sortOrder);
          else
            list = getUnsortedDeliveriesList(u, filter, startPos, count);

          List<DeliveryRow> rows = new ArrayList<DeliveryRow>(list.size());
          for (DeliveryInfo di : list)
            if (di != null)
              rows.add(new DeliveryRow(di));
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

  private static Comparator<DeliveryInfo> getComparator(final DataTableSortOrder sortOrder) {
    if (sortOrder == null || sortOrder.getColumnId().equals("name")) {
      return new Comparator<DeliveryInfo>() {
        public int compare(DeliveryInfo o1, DeliveryInfo o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getName().compareTo(o2.getName()) * (sortOrder == null || sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("userId")) {
      return new Comparator<DeliveryInfo>() {
        public int compare(DeliveryInfo o1, DeliveryInfo o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getUserId().compareTo(o2.getUserId()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("status")) {
      return new Comparator<DeliveryInfo>() {
        public int compare(DeliveryInfo o1, DeliveryInfo o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getStatus().toString().compareTo(o2.getStatus().toString()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("startDate")) {
      return new Comparator<DeliveryInfo>() {
        public int compare(DeliveryInfo o1, DeliveryInfo o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getStartDate().compareTo(o2.getStartDate()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else {
      return new Comparator<DeliveryInfo>() {
        public int compare(DeliveryInfo o1, DeliveryInfo o2) {
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

    private final DeliveryInfo deliveryInfo;

    public DeliveryRow(DeliveryInfo deliveryInfo) {
      this.deliveryInfo = deliveryInfo;
    }

    public Date getEndDate() {
      return deliveryInfo.getEndDate();
    }

    public Date getStartDate() {
      return deliveryInfo.getStartDate();
    }

    public DeliveryStatus getStatus() {
      return deliveryInfo.getStatus();
    }

    public String getUserId() {
      return deliveryInfo.getUserId();
    }

    public String getName() {
      return deliveryInfo.getName();
    }

    public int getDeliveryId() {
      return deliveryInfo.getDeliveryId();
    }
  }


}