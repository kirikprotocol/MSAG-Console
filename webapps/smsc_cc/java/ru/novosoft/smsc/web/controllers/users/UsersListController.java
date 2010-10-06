package ru.novosoft.smsc.web.controllers.users;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;

import javax.faces.application.FacesMessage;
import javax.faces.event.ActionEvent;
import java.util.*;

/**
 * author: alkhal
 */
@SuppressWarnings({"unchecked", "UnusedDeclaration"})
public class UsersListController extends UsersController {

  private static final Logger logger = Logger.getLogger(UsersListController.class);

  private String filterByLogin;

  public UsersListController() {
    super(true);
  }

  public void clearFilter() {
    filterByLogin = null;
  }

  private List selectedRows;

  public void setSelectedRows(List rows) {
    selectedRows = rows;
  }

  public void removeSelected(ActionEvent e) {
    UsersSettings settings = getSettings();
    Map<String, User> users = settings.getUsersMap();
    if (selectedRows != null && !selectedRows.isEmpty()) {
      for (String s : (List<String>) selectedRows)
        users.remove(s);

      try {
        settings.setUsers(users.values());
        setSettings(settings);
        checkChanges();
      } catch (AdminException e1) {
        addError(e1);
      }
    }
  }

  public String submit() {

    try {

      submitSettings();

      return "INDEX";

    } catch (AdminException e) {
      logger.warn(e, e);
      addMessage(FacesMessage.SEVERITY_ERROR, e.getMessage(getLocale()));

      return null;
    }
  }

  public void reset(ActionEvent ev) {
    try {
      resetSettings();
    } catch (AdminException e) {
      addError(e);
    }
  }

  public String edit() {
    return "USER_EDIT";
  }

  public DataTableModel getUsersModel() {
    final List<User> users = new ArrayList<User>(getSettings().getUsers());

    if (filterByLogin != null && (filterByLogin = filterByLogin.trim()).length() > 0) {
      for (Iterator<User> iter = users.iterator(); iter.hasNext();) {
        if (!iter.next().getLogin().startsWith(filterByLogin))
          iter.remove();
      }
    }

    return new DataTableModel() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        List<User> tmp = new LinkedList<User>(users);

        Collections.sort(tmp, new Comparator<User>() {
          public int compare(User o1, User o2) {
            if (sortOrder != null) {
              int mul = sortOrder.isAsc() ? 1 : -1;
              if ("firstName".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getFirstName().compareTo(o2.getFirstName());
              } else if ("lastName".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getLastName().compareTo(o2.getLastName());
              } else if ("dept".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getDept().compareTo(o2.getDept());
              } else if ("login".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getLogin().compareTo(o2.getLogin());
              }
            }
            return o1.getLogin().compareTo(o2.getLogin());
          }
        });

        List<User> result = new LinkedList<User>();
        for (Iterator<User> i = tmp.iterator(); i.hasNext() && count > 0;) {
          User r = i.next();
          if (--startPos < 0) {
            result.add(r);
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

  public String getFilterByLogin() {
    return filterByLogin;
  }

  public void setFilterByLogin(String filterByLogin) {
    this.filterByLogin = filterByLogin;
  }

}
