package mobi.eyeline.informer.web.controllers.archive;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.archive.Request;
import mobi.eyeline.informer.admin.archive.RequestFilter;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.components.data_table.model.EmptyDataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.ModelWithObjectIds;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.model.SelectItem;
import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class RequestListController extends InformerController{

  private List selectedRows;

  private String error;

  private RequestFilter filter = new RequestFilter();

  public RequestListController() {
    try {
      if(!getConfig().isArchiveDaemonDeployed() || getConfig().getArchiveDaemonOnlineHost() == null) {
        error = getLocalizedString("archive.daemon.offline");
      }
    } catch (AdminException e) {
      addError(e);
    }
  }

  public void query() {
  }

  public void clear() {
    filter = new RequestFilter();
  }

  public String getError() {
    return error;
  }

  public boolean isOffline() {
    return error != null;
  }

  public DataTableModel getRequests() {
    if(!isUserInAdminRole()) {
      filter.setCreator(getUserName());
    }
    final List<Request> requests;
    try{
      requests = getConfig().getRequests(filter);

    }catch(AdminException e) {
      addError(e);
      return new EmptyDataTableModel();
    }

    return new ModelWithObjectIds() {
      @Override
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        List<RequestWrapper> result = new ArrayList<RequestWrapper>(count);

        if (count <= 0) {
          return result;
        }

        if (sortOrder == null || sortOrder.getColumnId() == null || sortOrder.getColumnId().equals("id")) {
          Collections.sort(requests, new Comparator<Request>() {
            public int compare(Request o1, Request o2) {
              return (o1.getId() < o2.getId() ? -1 : o1.getId() > o2.getId() ? 1 : 0) * (sortOrder == null || sortOrder.isAsc() ? 1 : -1);
            }
          });
        }else if (sortOrder.getColumnId().equals("name")) {
          Collections.sort(requests, new Comparator<Request>() {
            public int compare(Request o1, Request o2) {
              return (o1.getName().compareTo(o2.getName())) * (sortOrder.isAsc() ? 1 : -1);
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
            result.add(new RequestWrapper(r));
            count--;
          }
        }

        return result;
      }

      @Override
      public String getId(Object value) {
        return Integer.toString(((RequestWrapper)value).getId());
      }

      @Override
      public int getRowsCount() {
        return requests.size();
      }
    };
  }

  public RequestFilter getFilter() {
    return filter;
  }

  public void setFilter(RequestFilter filter) {
    this.filter = filter;
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

  public List<SelectItem> getUniqueTypes() {
    List<SelectItem> result = new LinkedList<SelectItem>();
    for (Request.Type st : Request.Type.values()) {
      result.add(new SelectItem(st, RequestTypeConverter.getAsString(getLocale(), st)));
    }
    return result;
  }

  public List<SelectItem> getUniqueUsers() {
    List<SelectItem> result = new LinkedList<SelectItem>();
    for (User st : getConfig().getUsers()) {
      result.add(new SelectItem(st.getLogin(), st.getLogin()));
    }
    Collections.sort(result, new Comparator<SelectItem>() {
      @Override
      public int compare(SelectItem o1, SelectItem o2) {
        return o1.getLabel().compareTo(o2.getLabel());
      }
    });
    return result;
  }

  public String getCreator() {
    return filter.getCreator();
  }

  public void setCreator(String creator) {
    filter.setCreator(creator == null || creator.length() == 0 ? null : creator);
  }

  public String getId() {
    return filter.getId() == null ? null : filter.getId().toString();
  }

  public void setId(String id) {
    if(id != null && id.length()>0) {
      filter.setId(Integer.parseInt(id));
    }
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

  public static class RequestWrapper {

    private Request request;

    public RequestWrapper(Request request) {
      this.request = request;
    }

    public boolean isInProgress() {
      return request.getStatus() == Request.Status.IN_PROCESS;
    }

    public boolean isFinished() {
      return request.getStatus() == Request.Status.FINISHED;
    }

    public boolean isDeliveriesSearch() {
      return request.getType() == Request.Type.deliveries;
    }

    public String getCreater() {
      return request.getCreater();
    }

    public Date getFrom() {
      return request.getFrom();
    }

    public Date getTill() {
      return request.getTill();
    }

    public Request.Status getStatus() {
      return request.getStatus();
    }

    public int getProgress() {
      return request.getProgress();
    }

    public int getId() {
      return request.getId();
    }

    public String getName() {
      return request.getName();
    }

    public Request.Type getType() {
      return request.getType();
    }
  }
}
