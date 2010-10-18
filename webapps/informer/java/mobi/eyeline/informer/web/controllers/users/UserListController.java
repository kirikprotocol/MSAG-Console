package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.context.FacesContext;
import javax.faces.model.SelectItem;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 14.10.2010
 * Time: 18:25:48
 */
public class UserListController extends InformerController {
  private List<String> selectedRows;
  private String firstNamePrefix;
  private String lastNamePrefix;
  private String loginPrefix;
  private User.Status statusFilter;
  private String organizationPrefix;


  public UserListController() throws AdminException {
    super();
  }

  public void clearFilter() {
    firstNamePrefix=null;
    lastNamePrefix=null;
    loginPrefix=null;
    statusFilter=null;
    organizationPrefix=null;
  }

  public DataTableModel getUsersModel() throws AdminException {
    final List<User> users = new ArrayList<User>();
    for(User u : getConfig().getUsers()) {
      if(loginPrefix!=null && u.getLogin().indexOf(loginPrefix)<0) {
        continue;
      }
      if(firstNamePrefix!=null && u.getFirstName().indexOf(firstNamePrefix)<0) {
        continue;
      }
      if(lastNamePrefix!=null && u.getLastName().indexOf(lastNamePrefix)<0) {
        continue;
      }
      if(statusFilter!=null && u.getStatus()!=statusFilter) {
        continue;
      }
      if(organizationPrefix!=null && u.getOrganization().indexOf(organizationPrefix)<0) {
        continue;
      }
      users.add(u);
    }

    return new DataTableModel() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        if (sortOrder != null) {
          Collections.sort(users, new Comparator<User>() {
            public int compare(User o1, User o2) {
              int mul = sortOrder.isAsc() ? 1 : -1;
              if ("login".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getLogin().compareTo(o2.getLogin());
              }
              if ("firstName".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getFirstName().compareTo(o2.getFirstName());
              }
              if ("lastName".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getLastName().compareTo(o2.getLastName());
              }
              if ("organization".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getOrganization().compareTo(o2.getOrganization());
              }
              if ("status".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getStatus().compareTo(o2.getStatus());
              }
              return 0;
            }
          });
        }
        List<User> result = new LinkedList<User>();
        for (Iterator<User> i = users.iterator(); i.hasNext() && count > 0;) {
          User u = i.next();
          if (--startPos < 0) {
            result.add(u);
            count--;
          }
        }
        return result;
      }

      public int getRowsCount() {
        return users.size();
      }
    };

  }


  public void setSelectedRows(List<String> selectedRows) {
    this.selectedRows = selectedRows;
  }

  public List<String> getSelectedRows() {
    return selectedRows;
  }



  public String removeSelected() throws AdminException {
    for(String id : selectedRows) {
      try {
        getConfig().removeUser(id,getUserName());
      }
      catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }



  public void setFirstNamePrefix(String firstNamePrefix) {
    if(firstNamePrefix==null || firstNamePrefix.trim().length()==0) firstNamePrefix=null;
    this.firstNamePrefix = firstNamePrefix;
  }

  public String getFirstNamePrefix() {
    return firstNamePrefix;
  }

  public void setLastNamePrefix(String lastNamePrefix) {
    if(lastNamePrefix==null || lastNamePrefix.trim().length()==0) lastNamePrefix=null;
    this.lastNamePrefix = lastNamePrefix;
  }

  public String getLastNamePrefix() {
    return lastNamePrefix;
  }

  public void setLoginPrefix(String loginPrefix) {
    if(loginPrefix==null || loginPrefix.trim().length()==0) loginPrefix=null;
    this.loginPrefix = loginPrefix;
  }

  public String getLoginPrefix() {
    return loginPrefix;
  }

  public void setStatusFilter(String statusFilter) {
    if(statusFilter==null || statusFilter.length()==0) {
      this.statusFilter=null;
    }
    else {
      this.statusFilter = User.Status.valueOf(statusFilter);
    }
  }

  public String getStatusFilter() {
    return statusFilter==null ? "" : statusFilter.toString();
  }

  public List<SelectItem> getStatusItems() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem(User.Status.ENABLED.toString()));
    ret.add(new SelectItem(User.Status.DISABLED.toString()));
    return ret;
  }

  public void setOrganizationPrefix(String organizationPrefix) {
    if(organizationPrefix==null || organizationPrefix.trim().length()==0) organizationPrefix=null;
    this.organizationPrefix = organizationPrefix;
  }

  public String getOrganizationPrefix() {
    return organizationPrefix;
  }

  public String editSelected() {
    if(selectedRows!=null) {
      if(selectedRows.size()>=1) {
        getSession(true).setAttribute("userIds",selectedRows);
        return "USER_EDIT_GROUP";
      }
    }
    return null;
  }
}



