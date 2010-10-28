package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.config.Configuration;

import javax.faces.model.SelectItem;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryListController extends DeliveryController {

  private String namePrefix;

  private String userFilter;

  private String status;

  private Configuration config;

  private boolean init = false;


  private List<String> selected;

  private final static int MEMORY_LIMIT = 100;


  public DeliveryListController() {
    config = getConfig();
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
  }

  public void query() {
    init = true;
  }

  public boolean isInit() {
    return init;
  }

  public void setInit(boolean init) {
    this.init = init;
  }

  public List<SelectItem> getUniqueStatuses() {
    List<SelectItem> result = new ArrayList<SelectItem>();
    result.add(new SelectItem(null));
    Locale l = getLocale();
    for (DeliveryStatus s : DeliveryStatus.values()) {
      result.add(new SelectItem(s.toString(), DeliveryStatusConverter.getAsString(l, s)));
    }
    return result;
  }


  @SuppressWarnings({"unchecked"})
  public void setSelected(List selected) {
    if(selected != null) {
      this.selected = new ArrayList<String>((List<String>) selected);
    }
  }


  public String activate() {
    if(selected != null) {
      User u = config.getUser(getUserName());
      for(String r : selected) {
        try{
          int id =  Integer.parseInt(r);
          DeliveryStatistics d = config.getDeliveryStats(u.getLogin(), u.getPassword(), id);
          if(d.getDeliveryState() != null) {
            DeliveryStatus status = d.getDeliveryState().getStatus();
            if(status == DeliveryStatus.Planned || status == DeliveryStatus.Paused) {
              config.activateDelivery(u.getLogin(), u.getPassword(), id);
            }
          }
        }catch (AdminException e){
          addError(e);
        }
      }
    }
    return null;
  }

  public String pause() {
    if(selected != null) {
      User u = config.getUser(getUserName());
      for(String r : selected) {
        try{
          int id =  Integer.parseInt(r);
          DeliveryStatistics d = config.getDeliveryStats(u.getLogin(), u.getPassword(), id);
          if(d.getDeliveryState() != null) {
            DeliveryStatus status = d.getDeliveryState().getStatus();
            if(status == DeliveryStatus.Active) {
              config.pauseDelivery(u.getLogin(), u.getPassword(), id);
            }
          }
        }catch (AdminException e){
          addError(e);
        }
      }
    }
    return null;
  }

  private int getDeliveryInfos(final Comparator<DeliveryInfo> comparator, final DeliveryInfo infimum, final int count, final List<DeliveryInfo> result) {
    try{
      User u = config.getUser(getUserName());
      DeliveryFilter filter = new DeliveryFilter();
      if(userFilter != null && (userFilter = userFilter.trim()).length() != 0) {
        filter.setUserIdFilter(new String[]{userFilter});
      }else if(!isUserInAdminRole()) {
        filter.setUserIdFilter(new String[]{u.getLogin()});
      }
      filter.setResultFields(new DeliveryFields[]{DeliveryFields.Name, DeliveryFields.UserId, DeliveryFields.Status,
          DeliveryFields.StartDate, DeliveryFields.EndDate});
      if(status != null && status.length() > 0) {
        filter.setStatusFilter(new DeliveryStatus[]{DeliveryStatus.valueOf(status)});
      }
      final int total[] = new int[]{0};
      getConfig().getDeliveries(u.getLogin(), u.getPassword(), filter, 1, new Visitor<DeliveryInfo>() {
        public boolean visit(DeliveryInfo value) throws AdminException {
          if(namePrefix != null && (namePrefix = namePrefix.trim()).length() != 0 &&
              !value.getName().startsWith(namePrefix)) {
            return true;
          }
          int compare;
          if(infimum == null || (compare = comparator.compare(infimum, value)) < 0 || (compare == 0 && infimum.getDeliveryId() != value.getDeliveryId())) {
            result.add(value);
            total[0]++;
          }
          if(result.size() == MEMORY_LIMIT) {
            Collections.sort(result, comparator);
            if(count < result.size()) {
              result.subList(count, result.size()).clear();
            }
          }
          return true;
        }
      });
      if(result.size() > count) {
        Collections.sort(result, comparator);
        result.subList(count, result.size()).clear();
      }
      return total[0];
    }catch (AdminException e){
      addError(e);
      return 0;
    }
  }

  public DataTableModel getDeliviries() {
    return new DataTableModel() {

      private int count = 0;
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        if(!init) {
          return Collections.emptyList();
        }
        LinkedList<DeliveryInfo> result = new LinkedList<DeliveryInfo>();
        int r = startPos/MEMORY_LIMIT;
        int os = startPos%MEMORY_LIMIT;
        DeliveryInfo last = null;
        for (int i=1;i<=r;i++){
          result.clear();
          int c = getDeliveryInfos(getComparator(sortOrder), last, MEMORY_LIMIT, result);
          if(this.count == 0) {
            this.count = c;
          }
          if(result.isEmpty()) {
            return Collections.emptyList();
          }
          last = result.getLast();
        }
        result.clear();
        int c = getDeliveryInfos(getComparator(sortOrder), last, os+count, result);
        if(result.isEmpty()) {
          return Collections.emptyList();
        }
        if(this.count == 0) {
          this.count = c;
        }
        if(os != 0) {
        result.subList(0, os).clear();
        }


        return result;
      }

      public int getRowsCount() {
        return count;
      }
    };


  }

  private static Comparator<DeliveryInfo> getComparator(final DataTableSortOrder sortOrder) {
    if(sortOrder == null || sortOrder.getColumnId().equals("name")) {
      return  new Comparator<DeliveryInfo>() {
        public int compare(DeliveryInfo o1, DeliveryInfo o2) {
          return o1.getName().compareTo(o2.getName())*(sortOrder == null || sortOrder.isAsc() ? 1 : -1);
        }
      };
    }else if (sortOrder.getColumnId().equals("userId")) {
      return new Comparator<DeliveryInfo>() {
        public int compare(DeliveryInfo o1, DeliveryInfo o2) {
          return o1.getUserId().compareTo(o2.getUserId())*(sortOrder.isAsc() ? 1 : -1);
        }
      };
    }else if (sortOrder.getColumnId().equals("status")) {
      return new Comparator<DeliveryInfo>() {
        public int compare(DeliveryInfo o1, DeliveryInfo o2) {
          return o1.getStatus().toString().compareTo(o2.getStatus().toString())*(sortOrder.isAsc() ? 1 : -1);
        }
      };
    }else if (sortOrder.getColumnId().equals("startDate")) {
      return new Comparator<DeliveryInfo>() {
        public int compare(DeliveryInfo o1, DeliveryInfo o2) {
          return o1.getStartDate().compareTo(o2.getStartDate())*(sortOrder.isAsc() ? 1 : -1);
        }
      };
    }else  {
      return new Comparator<DeliveryInfo>() {
        public int compare(DeliveryInfo o1, DeliveryInfo o2) {
          return o1.getEndDate().compareTo(o2.getEndDate())*(sortOrder.isAsc() ? 1 : -1);
        }
      };
    }
  }


}
