package mobi.eyeline.informer.web.controllers.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.archive.Request;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.components.data_table.model.EmptyDataTableModel;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class RequestListController extends InformerController{

  private List selectedRows;

  public DataTableModel getRequests() {
    final List<Request> requests;
    try{
      requests = getConfig().getRequests();
      if(!isUserInAdminRole()) {
        Iterator<Request> i = requests.iterator();
        String u = getUserName();
        while(i.hasNext()) {
          Request r = i.next();
          if(!r.getCreater().equals(u)) {
            i.remove();
          }
        }
      }
    }catch(AdminException e) {
      addError(e);
      return new EmptyDataTableModel();
    }
    return new DataTableModel() {
      @Override
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        List<Request> result = new ArrayList<Request>(count);

        if (count <= 0) {
          return result;
        }

        if (sortOrder == null || sortOrder.getColumnId() == null || sortOrder.getColumnId().equals("name")) {
          Collections.sort(requests, new Comparator<Request>() {
            public int compare(Request o1, Request o2) {
              return (o1.getName().compareTo(o2.getName())) * (sortOrder == null || sortOrder.isAsc() ? 1 : -1);
            }
          });
        } else if (sortOrder.getColumnId().equals("type")) {
          Collections.sort(requests, new Comparator<Request>() {
            public int compare(Request o1, Request o2) {
              return (o1.getType().compareTo(o2.getType())) * (sortOrder.isAsc() ? 1 : -1);
            }
          });
        } else if (sortOrder.getColumnId().equals("begin")) {
          Collections.sort(requests, new Comparator<Request>() {
            public int compare(Request o1, Request o2) {
              return (o1.getFrom().compareTo(o2.getFrom())) * (sortOrder.isAsc() ? 1 : -1);
            }
          });
        }

        for (Iterator<Request> i = requests.iterator(); i.hasNext() && count > 0;) {
          Request r = i.next();
          if (--startPos < 0) {
            result.add(r);
            count--;
          }
        }

        return result;
      }

      @Override
      public int getRowsCount() {
        return requests.size();
      }
    };
  }

  public String cancel() {
    String rId = getRequestParameter("requestId");
    if(rId != null && rId.length()>0) {
      try {
        getConfig().cancelRequest(Integer.parseInt(rId));
      } catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public List getSelectedRows() {
    return selectedRows;
  }

  public void setSelectedRows(List selectedRows) {
    this.selectedRows = selectedRows;
  }

  @SuppressWarnings({"unchecked"})
  public String remove() {
    if(selectedRows != null) {
      Configuration c = getConfig();
      for(String s : (List<String>)selectedRows) {
        try {
          c.removeRequest(Integer.parseInt(s));
        } catch (AdminException e) {
          addError(e);
        }
      }
    }
    return null;
  }
}
