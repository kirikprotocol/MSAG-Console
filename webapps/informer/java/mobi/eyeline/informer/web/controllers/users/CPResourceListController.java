package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.util.*;

/**
 * @author Aleksandr Khalitov
 */
public class CPResourceListController extends CPResourceController {

  private String user;

  private UserCPsettings.Protocol protocol;

  private List<String> selectedRows;

  public void clearFilter() {
    protocol = null;
    user = null;
  }

  private String getEditAction(UserCPsettings.Protocol protocol) {
    switch (protocol) {
      case ftp: return "FTP_RESOURCE_EDIT";
      case file: return "FILE_RESOURCE_EDIT";
      case sftp: return "SFTP_RESOURCE_EDIT";
      case localFtp: return "LOCAL_FTP_RESOURCE_EDIT";
      default: return "SMB_RESOURCE_EDIT";
    }
  }

  private List<Row> load() {

    final List<Row> rows = new LinkedList<Row>();

    for(User u : getConfig().getUsers()) {
      if(user != null && user.length()>0 && !user.equals(u.getLogin())) {
        continue;
      }
      if(u.getCpSettings() != null) {
        for(UserCPsettings s : u.getCpSettings()) {
          if(protocol != null && s.getProtocol() != protocol) {
            continue;
          }
          rows.add(new Row(u.getLogin(), s, getEditAction(s.getProtocol())));
        }
      }
    }

    return rows;
  }

  public DataTableModel getModel() {

    final List<Row> rows = load();

    return new DataTableModel() {

      @Override
      public String getId(Object value) {
        Row r = (Row)value;
        try {
          return r.getUser()+'|'+r.getHashId();
        } catch (AdminException e) {
          addError(e);
          return null;
        }
      }

      @Override
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        if(sortOrder == null || sortOrder.getColumnId().equals("user")) {
          Collections.sort(rows, new Comparator<Row>() {
            public int compare(Row o1, Row o2) {
              return o1.getUser().compareTo(o2.getUser()) * (sortOrder == null || sortOrder.isAsc() ? 1 : -1);
            }
          });
        }else if(sortOrder.getColumnId().equals("name")) {
          Collections.sort(rows, new Comparator<Row>() {
            public int compare(Row o1, Row o2) {
              return o1.getName().compareTo(o2.getName()) * (sortOrder.isAsc() ? 1 : -1);
            }
          });
        }else if(sortOrder.getColumnId().equals("protocol")) {
          Collections.sort(rows, new Comparator<Row>() {
            public int compare(Row o1, Row o2) {
              return o1.getProtocol().toString().compareTo(o2.getProtocol().toString()) * (sortOrder.isAsc() ? 1 : -1);
            }
          });
        }
        List<Row> result = new LinkedList<Row>();
        for (Iterator<Row> i = rows.iterator(); i.hasNext() && count > 0;) {
          Row r = i.next();
          if (--startPos < 0) {
            result.add(r);
            count--;
          }
        }
        return result;
      }

      @Override
      public int getRowsCount() {
        return rows.size();
      }
    };
  }


  public String getUser() {
    return user;
  }

  public void setUser(String user) {
    this.user = user;
  }

  public UserCPsettings.Protocol getProtocol() {
    return protocol;
  }

  public void setProtocol(UserCPsettings.Protocol protocol) {
    this.protocol = protocol;
  }

  public void setSelectedRows(List<String> selectedRows) {
    this.selectedRows = selectedRows;
  }

  public List<String> getSelectedRows() {
    return selectedRows;
  }


  public String removeSelected() {
    for (String id : selectedRows) {
      try{
        String[] str = id.split("\\|", 2);
        User u = getConfig().getUser(str[0]);
        if(u != null) {
          List<UserCPsettings> ss = u.getCpSettings();
          if(ss != null) {
            Iterator<UserCPsettings> i = ss.iterator();
            while(i.hasNext()) {
              UserCPsettings s = i.next();
              if(s.getHashId().equals(str[1])) {
                i.remove();
              }
            }
            u.setCpSettings(ss);
            getConfig().updateUser(u, getUserName());
          }
        }
      }catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }

  public static class Row {

    private String user;

    private UserCPsettings settings;

    private String editAction;

    public Row(String user, UserCPsettings settings, String editAction) {
      this.user = user;
      this.settings = settings;
      this.editAction = editAction;
    }

    public String editAction() {
      return editAction;
    }

    public String getName() {
      return settings.getName();
    }

    public String getUser() {
      return user;
    }

    public String getHost() {
      return settings.getHost();
    }

    public Integer getPort() {
      return settings.getPort();
    }

    public String getDirectory() {
      return settings.getDirectory();
    }

    public UserCPsettings.Protocol getProtocol() {
      return settings.getProtocol();
    }

    public String getHashId() throws AdminException {
      return settings.getHashId();
    }
  }
}
