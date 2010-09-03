package ru.novosoft.smsc.web.controllers.users;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.config.AppliableConfiguration;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import java.security.Principal;
import java.util.*;

/**
 * author: alkhal
 */
@SuppressWarnings({"unchecked", "UnusedDeclaration"})
public class UsersListController extends UsersController{

  private Map<String, User> users;

  private AppliableConfiguration conf;

  private static final Logger logger = Logger.getLogger(UsersListController.class);

  private boolean index_initialized;

  private String filterByLogin;

  public UsersListController() {
    conf = WebContext.getInstance().getAppliableConfiguration();
    if(getRequestParameter("index_initialized") == null) {
      initUsers();
      index_initialized = true;
    }
  }

  public void clearFilter() {
    filterByLogin = null;
  }

  private void initUsers() {
    if(isChanged()) {
      users = getUsersFromSession(false);
      viewChanges();
    }else {
      setLastUpdate(conf.getUsersSettingsUpdateInfo().getLastUpdateTime());
      try {
        users = conf.getUsersSettings().getUsersMap();
      } catch (AdminException e) {
        logger.error(e,e);
        addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));
      }
    }
  }


  private List selectedRows;

  public void setSelectedRows(List rows) {
    selectedRows = rows;
  }

  public void removeSelected(ActionEvent e) {
    if(selectedRows != null && !selectedRows.isEmpty()) {
      for(String s : (List<String>)selectedRows) {
        users.remove(s);
      }
      setChanged(true);
      viewChanges();
      putUsersToSession(users);
    }
  }

  public String submit() {

    Long lastChange = getLastUpdate();
    if(lastChange != conf.getUsersSettingsUpdateInfo().getLastUpdateTime()) {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual");
      return null;
    }

    try{
      UsersSettings settings = conf.getUsersSettings();

      Collection<User> newUsers = new ArrayList<User>(users.values());
      settings.setUsers(newUsers);

      Principal p = getUserPrincipal();

      conf.setUsersSettings(settings, p.getName());

      cleanSession();
      setChanged(false);

      return "INDEX";

    } catch (AdminException e) {
      logger.warn(e,e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));

      return null;
    }
  }

  public void reset(ActionEvent ev) {
    cleanSession();
    setChanged(false);
    initUsers();
  }

  public String edit() {
    putUsersToSession(users);
    return "USER_EDIT";
  }

  public DataTableModel getUsersModel() {
    return new DataTableModel() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        List<User> result = new ArrayList<User>(count);
        if(count <= 0) {
          return result;
        }
        for(Iterator<User> i = users.values().iterator();i.hasNext() && count>0;) {
          User r = i.next();
          if(filterByLogin != null && (filterByLogin = filterByLogin.trim()).length()>0 && !r.getLogin().startsWith(filterByLogin)) {
            continue;
          }
          if(--startPos < 0) {
            result.add(r);
            count--;
          }
        }
        Collections.sort(result, new Comparator<User>() {
          public int compare(User o1, User o2) {
            if(sortOrder != null) {
              int mul = sortOrder.isAsc() ? 1 : -1;
              if("firstName".equals(sortOrder.getColumnId())) {
                return (mul)*o1.getFirstName().compareTo(o2.getFirstName());
              }else if("lastName".equals(sortOrder.getColumnId())) {
                return (mul)*o1.getLastName().compareTo(o2.getLastName());
              }else if("dept".equals(sortOrder.getColumnId())) {
                return (mul)*o1.getDept().compareTo(o2.getDept());
              }else if("login".equals(sortOrder.getColumnId())) {
                return (mul)*o1.getLogin().compareTo(o2.getLogin());
              }
            }
            return o1.getLogin().compareTo(o2.getLogin());
          }
        });
        return result;
      }

      public int getRowsCount() {
        if(filterByLogin == null || (filterByLogin = filterByLogin.trim()).length() == 0) {
          return users.size();
        }
        int result = 0;
        for(User r : users.values()) {
          if(r.getLogin().startsWith(filterByLogin)) {
            result++;
          }
        }
        return result;
      }
    };
  }

  public boolean isIndex_initialized() {
    return index_initialized;
  }

  public void setIndex_initialized(boolean index_initialized) {
    this.index_initialized = index_initialized;
  }

  public Map<String, User> getUsers() {
    return users;
  }

  public void setUsers(Map<String, User> users) {
    this.users = users;
  }

  public String getFilterByLogin() {
    return filterByLogin;
  }

  public void setFilterByLogin(String filterByLogin) {
    this.filterByLogin = filterByLogin;
  }

  public void viewChanges() {
    addLocalizedMessage(FacesMessage.SEVERITY_INFO, "smsc.users.submit.hint");
  }
}
