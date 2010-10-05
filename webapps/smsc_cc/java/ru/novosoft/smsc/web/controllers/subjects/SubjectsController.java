package ru.novosoft.smsc.web.controllers.subjects;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.RouteSubjectSettings;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.web.config.SettingsManager;
import ru.novosoft.smsc.web.controllers.SettingsMController;

import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 05.10.2010
 * Time: 16:17:29
 */
public class SubjectsController extends SettingsMController<RouteSubjectSettings>{

  boolean initError = false;
  private DynamicTableModel masksFilter;
  private String namesFilter;
  private List selectedRows;
  private RouteSubjectSettings settings;


  public SubjectsController() {
    super(WebContext.getInstance().getRouteSubjectManager());
    try {
      init();
    } catch (AdminException e) {
      addError(e);
      initError = true;
    }
  }


  @Override
  protected void init() throws AdminException {
    super.init();    //To change body of overridden methods use File | Settings | File Templates.
    settings = getSettings();
    clearFilter();
    selectedRows = new ArrayList();
  }

  public String clearFilter() {
    masksFilter = new DynamicTableModel();
    namesFilter = null;
    return null;
  }

  public boolean isInitError() {
    return initError;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }



  public String reset() {
    //todo
    return "SUBJECTS";
  }

  public String submit() {
    //todo
    return "INDEX";
  }

  public void setMasksFilter(DynamicTableModel masksFilter) {
    this.masksFilter = masksFilter;
  }

  public DynamicTableModel getMasksFilter() {
    return masksFilter;
  }

  public String getNamesFilter() {
    return namesFilter;
  }

  public void setNamesFilter(String namesFilter) {
    this.namesFilter = namesFilter;
  }

  private boolean filterByMask(Subject subj) {
    if(masksFilter==null || masksFilter.getRowCount()==0) return true;
    for(Address m : subj.getMasks()) {
      String simpleMask = m.getSimpleAddress();
      for(DynamicTableRow row : masksFilter.getRows()) {
        String filterVal = ((String) row.getValue("mask")).trim();
        if(simpleMask.equals(filterVal)) {
          return true;
        }
      }
    }
    return false;
  }
  private boolean filterByName(Subject subj) {
    if(namesFilter==null) return true;
    return subj.getName().indexOf(namesFilter)>=0;
  }

  public DataTableModel getSubjectsModel() {
    final List<Subject> filteredSubjects = new ArrayList<Subject>();

    for(Subject subj : settings.getSubjects()) {
      if(filterByMask(subj) && filterByName(subj)) {
        filteredSubjects.add(subj);
      }
    }

    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        if (sortOrder != null) {
          Collections.sort(filteredSubjects, new Comparator<Subject>() {
            public int compare(Subject o1, Subject o2) {
              int mul = sortOrder.isAsc() ? 1 : -1;
              if ("name".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getName().compareTo(o2.getName());
              }
              else if ("defaultSme".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getDefaultSmeId().compareTo(o2.getDefaultSmeId());
              }
              return 0;
            }
          });
        }
        List<Subject> result = new LinkedList<Subject>();
        for (Iterator<Subject> i = filteredSubjects.iterator(); i.hasNext() && count > 0;) {
          Subject subj = i.next();
          if (--startPos < 0) {
            result.add(subj);
            count--;
          }
        }
        return result;
      }
      public int getRowsCount() {
        return filteredSubjects.size();  //To change body of implemented methods use File | Settings | File Templates.
      }
    };
  }

  public void setSelectedRows(List selectedRows) {
    this.selectedRows = selectedRows;
  }

  public List getSelectedRows() {
    return selectedRows;
  }

  public String add() {
    return "SUBJECT_EDIT";
  }

  public String deleteSelected() {


    setSettings(settings);
    checkChanges();
    return null;
  }
}
