package ru.novosoft.smsc.web.controllers.closed_groups;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import mobi.eyeline.util.jsf.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.util.jsf.components.data_table.model.ModelException;
import mobi.eyeline.util.jsf.components.data_table.model.ModelWithObjectIds;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.util.Address;

import javax.faces.event.ActionEvent;
import java.util.*;

/**
 * author: alkhal
 */
public class ClosedGroupsListController extends ClosedGroupsController{

  private List selectedRows;

  private Integer filterById;

  private String filterByName;

  private String filterByAddress;

  public ClosedGroupsListController() { 
    super();
  }

  public void setSelectedRows(List rows) {
    selectedRows = rows;
  }

  public void applyFilter() {

  }

  public void clearFilter() {
    filterByAddress = null;
    filterById = null;
    filterByName = null;
  }

  @SuppressWarnings({"unchecked"})
  public void removeSelected(ActionEvent e) {
    if (selectedRows != null && !selectedRows.isEmpty()) {
      for (String s : (List<String>) selectedRows) {
        int id = Integer.parseInt(s);
        try {
          manager.removeGroup(id);
        } catch (AdminException ex) {
          addError(ex);
        }
      }
    }
  }

  public DataTableModel getClosedGroupsModel() {

    return new ModelWithObjectIds() {

      private List<ClosedGroup> groups;

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        List<ClosedGroup> result = new ArrayList<ClosedGroup>(count);

        if (count <= 0) {
          return result;
        }
        List<ClosedGroup> groups = getGroups();
        if(sortOrder == null || sortOrder.getColumnId() == null || sortOrder.getColumnId().equals("id")) {
          Collections.sort(groups, new Comparator<ClosedGroup>() {
            public int compare(ClosedGroup o1, ClosedGroup o2) {
              try{
                return (o1.getId() < o2.getId() ? -1 : o1.getId() == o2.getId() ? 0 : -1) *( sortOrder == null || sortOrder.isAsc() ? 1 : -1);
              }catch (AdminException e){
                addError(e);
                return 0;
              }
            }
          });
        }else if(sortOrder.getColumnId().equals("name")) {
          Collections.sort(groups, new Comparator<ClosedGroup>() {
            public int compare(ClosedGroup o1, ClosedGroup o2) {
              try{
                return (o1.getName().compareTo(o2.getName())) *( sortOrder.isAsc() ? 1 : -1);
              }catch (AdminException e){
                addError(e);
                return 0;
              }
            }
          });
        }

        for (Iterator<ClosedGroup> i = groups.iterator(); i.hasNext() && count > 0;) {
          ClosedGroup r = i.next();
          if (--startPos < 0) {
            result.add(r);
            count--;
          }
        }

        return result;
      }

      public int getRowsCount() {
        return getGroups().size();
      }

      private List<ClosedGroup> getGroups() {
        if(groups == null) {
          groups = new LinkedList<ClosedGroup>();
          try{
            for(ClosedGroup g : manager.groups()) {
              if(filterById != null && g.getId() != filterById) {
                continue;
              }
              if(filterByName != null && (filterByName = filterByName.trim()).length()>0 && !g.getName().startsWith(filterByName)) {
                continue;
              }
              if(filterByAddress != null && (filterByAddress = filterByAddress.trim()).length() > 0) {
                boolean accept = false;
                for(Address a : g.getMasks()) {
                  if(a.getSimpleAddress().startsWith(filterByAddress) || a.getNormalizedAddress().startsWith(filterByAddress)) {
                    accept = true;
                    break;
                  }
                }
                if(!accept) {
                  continue;
                }
              }
              groups.add(g);
            }
          }catch (AdminException e) {
            addError(e);
            return new ArrayList<ClosedGroup>(0);
          }
        }
        return groups;
      }

      public String getId(Object o) throws ModelException {
        try {
          return String.valueOf(((ClosedGroup) o).getId());
        } catch (AdminException e) {
          return null;
        }
      }
    };


  }

  public Integer getFilterById() {
    return filterById;
  }

  public void setFilterById(Integer filterById) {
    this.filterById = filterById;
  }

  public String getFilterByName() {
    return filterByName;
  }

  public void setFilterByName(String filterByName) {
    this.filterByName = filterByName;
  }

  public String getFilterByAddress() {
    return filterByAddress;
  }

  public void setFilterByAddress(String filterByAddress) {
    this.filterByAddress = filterByAddress;
  }

}
