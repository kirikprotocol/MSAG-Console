package ru.novosoft.smsc.web.controllers;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.config.AppliableConfiguration;
import ru.novosoft.smsc.web.config.UpdateInfo;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import javax.servlet.http.HttpSession;
import java.security.Principal;
import java.util.*;

/**
 * author: alkhal
 */
public class UsersController extends SmscController{

  private Map<String, User> users;

  private AppliableConfiguration conf;

  private static final Logger logger = Logger.getLogger(UsersController.class);

  private boolean index_initialized;

  private HttpSession session;

  private String prefix;

  public UsersController() {
    session = getSession(false);
    conf = WebContext.getInstance().getAppliableConfiguration();
    if(getRequestParameter("index_initialized") == null) {
      initUsers();
      index_initialized = true;
    }
  }

  private void initUsers() {
    if( session.getAttribute("users.users") == null) {
      System.out.println("Init users");
      session.setAttribute("users.last.update", conf.getUsersSettingsUpdateInfo());
      UsersSettings usersSettings = conf.getUsersSettings();
      users = new LinkedHashMap<String, User>();
      for(User u : usersSettings.getUsers()) {
        users.put(u.getLogin(), u);
      }
    }else {
      users = (Map<String, User>)session.getAttribute("users.users");
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
    }
    session.setAttribute("users.users", users);
  }

  public String submit() {

    UpdateInfo lastChange = (UpdateInfo)session.getAttribute("users.last.update");
    if(lastChange != conf.getUsersSettingsUpdateInfo()) {
      addLocalizedMessage(FacesMessage.SEVERITY_ERROR, "smsc.config.not.actual");
      return null;
    }

    try{
      UsersSettings settings = conf.getUsersSettings();

      Collection<User> newUsers = new ArrayList<User>(users.values());
      settings.setUsers(newUsers);

      Principal p = getUserPrincipal();

      conf.setUsersSettings(settings, p.getName());

      session.removeAttribute("users.users");
      session.removeAttribute("users.last.update");
      return "INDEX";

    } catch (AdminException e) {
      logger.warn(e,e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));

      return null;
    }
  }

  public void reset(ActionEvent ev) {
    session.removeAttribute("users.users");
    session.removeAttribute("users.last.update");
    initUsers();
  }

  public String edit() {
    session.setAttribute("users.users", users);
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
          if(prefix != null && (prefix = prefix.trim()).length()>0 && !r.getLogin().startsWith(prefix)) {
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
              if("firstName".equals(sortOrder.getColumnId())) {
                return (sortOrder.isAsc() ? -1 : 1)*o1.getFirstName().compareTo(o2.getFirstName());
              }else if("lastName".equals(sortOrder.getColumnId())) {
                return (sortOrder.isAsc() ? -1 : 1)*o1.getLastName().compareTo(o2.getLastName());
              }else if("dept".equals(sortOrder.getColumnId())) {
                return (sortOrder.isAsc() ? -1 : 1)*o1.getDept().compareTo(o2.getDept());
              }else if("login".equals(sortOrder.getColumnId())) {
                return (sortOrder.isAsc() ? -1 : 1)*o1.getLogin().compareTo(o2.getLogin());
              }
            }
            return o1.getLogin().compareTo(o2.getLogin());
          }
        });
        return result;
      }

      public int getRowsCount() {
        if(prefix == null || (prefix = prefix.trim()).length() == 0) {
          return users.size();
        }
        int result = 0;
        for(User r : users.values()) {
          if(r.getLogin().startsWith(prefix)) {
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

  public String getPrefix() {
    return prefix;
  }

  public void setPrefix(String prefix) {
    this.prefix = prefix;
  }
}