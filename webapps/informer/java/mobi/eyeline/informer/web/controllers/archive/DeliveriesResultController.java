package mobi.eyeline.informer.web.controllers.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.archive.ArchiveDelivery;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.Identificator;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.components.data_table.model.EmptyDataTableModel;
import mobi.eyeline.informer.web.controllers.InformerController;
import mobi.eyeline.informer.web.controllers.users.UserEditController;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import javax.faces.model.SelectItem;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveriesResultController extends InformerController{

  private Date from;

  private Date till;

  private String name;

  private String id;

  private String owner;

  private Integer reqId;

  private List selected;

  private String error;

  public DeliveriesResultController() {
    String id = getRequestParameter("reqId");
    if(id == null || id.length() == 0) {
      id = getRequestParameter(UserEditController.COME_BACK_PARAMS);
    }
    if(id != null && id.length()>0) {
      reqId = Integer.parseInt(id);
    }else {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "archive.request.not.found", "");
    }
    try {
      if(!getConfig().isArchiveDaemonDeployed() || getConfig().getArchiveDaemonOnlineHost() == null) {
        error = getLocalizedString("archive.daemon.offline");
      }
    } catch (AdminException e) {
      addError(e);
    }
  }

  public String getError() {
    return error;
  }

  public boolean isOffline() {
    return error != null;
  }

  public void clearFilter() {
    owner = null;
    name = null;
    from = null;
    till = null;
    id = null;
  }

  public void query() {
  }

  public List<SelectItem> getUniqueUsers() {
    List<SelectItem> ss = new LinkedList<SelectItem>();
    ss.add(new SelectItem("",""));
    for(User u : getConfig().getUsers()) {
      ss.add(new SelectItem(u.getLogin(), u.getLogin()));
    }
    Collections.sort(ss, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return ss;
  }

  public void statForSelected(ActionEvent e) {
    if(selected == null || selected.isEmpty())
      return;

    getRequest().put("delivery", getSelectedAsString());
    getRequest().put("archive", "true");
    selected = null;
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

  @SuppressWarnings({"RedundantIfStatement"})
  private boolean accept(ArchiveDelivery value) {
    if(id != null && id.length()>0 && value.getId() != Integer.parseInt(id)) {
        return false;
    }
    if(name != null && name.length()>0 && !value.getName().startsWith(name)) {
      return false;
    }
    if(owner != null && owner.length()>0 && !value.getOwner().equals(owner)) {
      return false;
    }
    if(from != null && value.getStartDate().before(from)) {
      return false;
    }
    if(till != null && value.getStartDate().after(till)) {
      return false;
    }
    return true;
  }

  private List<ArchiveDelivery> getDeliveries(final int startPos, final int count, DataTableSortOrder sortOrder) throws AdminException {
    final Comparator<ArchiveDelivery> comparator = getComparator(sortOrder);

    final ArchiveDelivery infos[] = new ArchiveDelivery[startPos + count];
    final int lastIdx = infos.length - 1;

    getConfig().getDeliveriesResult(reqId, new Visitor<ArchiveDelivery>() {
      public boolean visit(ArchiveDelivery value) throws AdminException {
        if(!accept(value)) {
          return true;
        }
        if (infos[lastIdx] == null || comparator.compare(value, infos[lastIdx]) < 0)
          insert(infos, value, comparator);
        return true;
      }
    });

    return Arrays.asList(infos).subList(startPos, startPos + count);
  }


  public DataTableModel getDeliveries() {
    if(reqId == null) {
      return new EmptyDataTableModel();
    }

    List<User> users = getConfig().getUsers();
    final Set<String>existUsers = new HashSet<String>(users.size());
    for(User u : users) {
      existUsers.add(u.getLogin());
    }

    class DataTableModelImpl implements DataTableModel, Identificator{
      public List getRows(final int startPos, final int count, DataTableSortOrder sortOrder) {
        try {
          List<ArchiveDelivery> deliveries = getDeliveries(startPos, count, sortOrder);
          List<DeliveryRow> rows = new ArrayList<DeliveryRow>(deliveries.size());
          for(ArchiveDelivery d : deliveries) {
            if(d != null) {
              rows.add(new DeliveryRow(d, existUsers.contains(d.getOwner())));
            }
          }
          return rows;
        } catch (AdminException e) {
          addError(e);
          return Collections.emptyList();
        }
      }

      @Override
      public String getId(Object value) {
        return ((DeliveryRow)value).getId().toString();
      }

      public int getRowsCount() {
        try {
          final int[] count = new int[]{0};
          getConfig().getDeliveriesResult(reqId, new Visitor<ArchiveDelivery>() {
            @Override
            public boolean visit(ArchiveDelivery value) throws AdminException {
              if(accept(value)) {
                count[0]++;
              }
              return true;
            }
          });
          return count[0];
        } catch (AdminException e) {
          addError(e);
          return 0;
        }
      }
    };
    return new DataTableModelImpl();
  }

  public static class DeliveryRow {

    private ArchiveDelivery delivery;

    private boolean userExist;

    public DeliveryRow(ArchiveDelivery delivery, boolean userExist) {
      this.delivery = delivery;
      this.userExist = userExist;
    }

    public Integer getId() {
      return delivery.getId();
    }

    public String getName() {
      return delivery.getName();
    }

    public String getOwner() {
      return delivery.getOwner();
    }

    public Date getStartDate() {
      return delivery.getStartDate();
    }

    public Date getEndDate() {
      return delivery.getEndDate();
    }

    public boolean isUserExist() {
      return userExist;
    }
  }

  private static Comparator<ArchiveDelivery> getComparator(final DataTableSortOrder sortOrder) {
    if (sortOrder == null || sortOrder.getColumnId().equals("name")) {
      return new Comparator<ArchiveDelivery>() {
        public int compare(ArchiveDelivery o1, ArchiveDelivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getName().compareTo(o2.getName()) * (sortOrder == null || sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("userId")) {
      return new Comparator<ArchiveDelivery>() {
        public int compare(ArchiveDelivery o1, ArchiveDelivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getOwner().compareTo(o2.getOwner()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("startDate")) {
      return new Comparator<ArchiveDelivery>() {
        public int compare(ArchiveDelivery o1, ArchiveDelivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return o1.getStartDate().compareTo(o2.getStartDate()) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else if (sortOrder.getColumnId().equals("id")) {
      return new Comparator<ArchiveDelivery>() {
        public int compare(ArchiveDelivery o1, ArchiveDelivery o2) {
          if (o1 == null) return 1;
          if (o2 == null) return -1;
          return (o1.getId() < o2.getId() ? -1 : o1.getId() > o2.getId() ? 1 : 0) * (sortOrder.isAsc() ? 1 : -1);
        }
      };
    } else {
      return new Comparator<ArchiveDelivery>() {
        public int compare(ArchiveDelivery o1, ArchiveDelivery o2) {
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


  public List getSelected() {
    return selected;
  }

  public void setSelected(List selected) {
    this.selected = selected;
  }

  public String getSelectedAsString() {
    if (selected == null || selected.isEmpty())
      return "";

    StringBuilder sb = new StringBuilder();
    for (Object s : selected) {
      if (sb.length() > 0)
        sb.append(',');
      sb.append(s);
    }

    return sb.toString();
  }

  public Date getFrom() {
    return from;
  }

  public void setFrom(Date from) {
    this.from = from;
  }

  public Date getTill() {
    return till;
  }

  public void setTill(Date till) {
    this.till = till;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getId() {
    return id;
  }

  public void setId(String id) {
    this.id = id;
  }

  public String getOwner() {
    return owner;
  }

  public void setOwner(String owner) {
    this.owner = owner;
  }

  public Integer getReqId() {
    return reqId;
  }

  public void setReqId(Integer reqId) {
    this.reqId = reqId;
  }
}
