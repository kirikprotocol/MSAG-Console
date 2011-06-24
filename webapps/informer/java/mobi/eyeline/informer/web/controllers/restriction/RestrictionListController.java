package mobi.eyeline.informer.web.controllers.restriction;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.restriction.RestrictionsFilter;
import mobi.eyeline.informer.web.components.data_table.model.ModelWithObjectIds;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 08.11.2010
 * Time: 14:04:40
 */
public class RestrictionListController extends RestrictionController {

  RestrictionsFilter filter;
  private List<String> selectedRows;

  public RestrictionListController() throws AdminException {
    filter = new RestrictionsFilter();
  }

  public void clearFilter() {
    filter.setStartDate(null);
    filter.setEndDate(null);
    filter.setNameFilter(null);
    filter.setUserId(null);
  }

  public RestrictionsFilter getFilter() {
    return filter;
  }

  public void setFilter(RestrictionsFilter filter) {
    this.filter = filter;
  }


  public void setSelectedRows(List<String> selectedRows) {
    this.selectedRows = selectedRows;
  }

  public List<String> getSelectedRows() {
    return selectedRows;
  }

  public DataTableModel getRestrictionsModel() {
    final List<Restriction> restrictions = getConfig().getRestrictions(filter);


    return new ModelWithObjectIds() {
      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        if (sortOrder != null) {
          Collections.sort(restrictions, new Comparator<Restriction>() {
            public int compare(Restriction o1, Restriction o2) {
              int mul = sortOrder.isAsc() ? 1 : -1;
              if ("startDate".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getStartDate().compareTo(o2.getStartDate());
              }
              if ("endDate".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getEndDate().compareTo(o2.getEndDate());
              }
              if ("name".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getName().compareTo(o2.getName());
              }
              if ("allUsers".equals(sortOrder.getColumnId())) {
                return (mul) * (Boolean.valueOf(o1.isAllUsers())).compareTo(o2.isAllUsers());
              }
              return 0;
            }
          });
        }
        List<Restriction> result = new LinkedList<Restriction>();
        for (Iterator<Restriction> i = restrictions.iterator(); i.hasNext() && count > 0;) {
          Restriction r = i.next();
          if (--startPos < 0) {
            result.add(r);
            count--;
          }
        }
        return result;
      }

      @Override
      public String getId(Object value) {
        return ((Restriction)value).getId().toString();
      }

      public int getRowsCount() {
        return restrictions.size();
      }
    };
  }


  public String removeSelected() {
    for (String id : selectedRows) {
      try {
        getConfig().deleteRestriction(Integer.valueOf(id), getUserName());
      }
      catch (AdminException e) {
        addError(e);
      }
    }
    return null;
  }


}
