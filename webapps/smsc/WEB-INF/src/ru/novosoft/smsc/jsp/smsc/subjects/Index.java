package ru.novosoft.smsc.jsp.smsc.subjects;

/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.subject.SubjectQuery;
import ru.novosoft.smsc.util.Functions;

import javax.servlet.http.HttpServletRequest;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.Iterator;

public final class Index extends IndexBean
{
  public static final int RESULT_ADD = IndexBean.PRIVATE_RESULT + 0;
  public static final int RESULT_EDIT = IndexBean.PRIVATE_RESULT + 1;
  public static final int PRIVATE_RESULT = IndexBean.PRIVATE_RESULT + 2;

  private QueryResultSet subjects = null;

  private String editName = null;

  private String[] checkedSubjects = new String[0];
  private Set checkedSubjectsSet = new HashSet();
  private String[] filter_masks = new String[0];
  private boolean initialized = false;

  private String mbAdd = null;
  private String mbDelete = null;
  private String mbEdit = null;
  private String mbSave = null;
  private String mbLoad = null;
  private String mbRestore = null;
  private String mbQuery = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    pageSize = preferences.getSubjectsPageSize();
    if (sort != null)
      preferences.getSubjectsSortOrder().set(0, sort);
    else
      sort = (String) preferences.getSubjectsSortOrder().get(0);

    filter_masks = Functions.trimStrings(filter_masks);
    if (initialized)
      try {
        preferences.getSubjectsFilter().setMasks(filter_masks);
      } catch (AdminException e) {
        logger.error("Incorrect filter mask(s)", e);
        return error(SMSCErrors.error.subjects.incorrectFilterMask, e.getMessage());
      }
    else
      filter_masks = (String[]) preferences.getSubjectsFilter().getMaskStrings().toArray(new String[0]);

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    subjects = new EmptyResultSet();

    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbAdd != null)
      return RESULT_ADD;
    else if (mbEdit != null)
      return RESULT_EDIT;
    else if (mbDelete != null) {
      int dresult = deleteSubject();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }
    else if (mbSave != null) {
      int dresult = saveRoutes();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }
    else if (mbLoad != null) {
      int dresult = loadRoutes();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }
    else if (mbRestore != null) {
      int dresult = restoreRoutes();
      return (dresult != RESULT_OK) ? dresult : RESULT_DONE;
    }

    logger.debug("Subjects.Index - process with sorting [" + (String) preferences.getSubjectsSortOrder().get(0) + "]");
    subjects = routeSubjectManager.getSubjects().query(new SubjectQuery(pageSize, preferences.getSubjectsFilter(), preferences.getSubjectsSortOrder(), startPosition));
    if (request.getSession().getAttribute("SUBJECT_NAME") != null) {
        subjects = getSubjectsByName((String) request.getSession().getAttribute("SUBJECT_NAME"));
        request.getSession().removeAttribute("SUBJECT_NAME");
    }
    totalSize = subjects.getTotalSize();

    checkedSubjectsSet.addAll(Arrays.asList(checkedSubjects));

    return result;
  }

  private QueryResultSet getSubjectsByName(String name) {
        boolean found = false;
        QueryResultSet subjects = null;
        while (!found) {
            SubjectQuery query = new SubjectQuery(pageSize, preferences.getSubjectsFilter(), preferences.getSubjectsSortOrder(), startPosition);
            subjects = routeSubjectManager.getSubjects().query(query);
            for (Iterator i = subjects.iterator(); i.hasNext();) {
                DataItem item = (DataItem) i.next();
                String al = (String) item.getValue("Name");
                if (al.equals(name)) {
                    found = true;
                }
            }
            if (!found) {
                startPosition += pageSize;
            }
        }
        return subjects;
    }

  protected int deleteSubject()
  {
    int result = RESULT_DONE;
    for (int i = 0; i < checkedSubjects.length; i++) {
      String subject = checkedSubjects[i];
      if (!routeSubjectManager.getRoutes().isSubjectUsed(subject)) {
        routeSubjectManager.getSubjects().remove(subject);
        journalAppend(SubjectTypes.TYPE_subject, subject, Actions.ACTION_DEL);
        appContext.getStatuses().setSubjectsChanged(true);
      }
      else
        result = error(SMSCErrors.error.subjects.cantDelete, subject);
    }
    checkedSubjects = new String[0];
    checkedSubjectsSet.clear();
    return result;
  }

  protected int saveRoutes() // saves temporal configuration
  {
    try {
      routeSubjectManager.save();
    } catch (AdminException exc) {
      return error(SMSCErrors.error.routes.cantSave, exc.getMessage());
    }
    appContext.getStatuses().setRoutesSaved(true);
    appContext.getStatuses().setRoutesRestored(true);
    return RESULT_OK;
  }

  protected int restoreRoutes() // loads saved configuration
  {
    try {
      routeSubjectManager.restore();
      journalAppend(SubjectTypes.TYPE_route, null, Actions.ACTION_RESTORE);
      journalAppend(SubjectTypes.TYPE_subject, null, Actions.ACTION_RESTORE);
    } catch (AdminException exc) {
      return error(SMSCErrors.error.routes.cantRestore, exc.getMessage());
    }
    appContext.getStatuses().setSubjectsChanged(true);
    appContext.getStatuses().setRoutesChanged(true);
    appContext.getStatuses().setRoutesSaved(true);
    appContext.getStatuses().setRoutesRestored(true);
    appContext.getStatuses().setRoutesLoaded(false);
    return RESULT_OK;
  }

  protected int loadRoutes() // loads applied configuration
  {
    try {
      routeSubjectManager.load();
    } catch (AdminException exc) {
      return error(SMSCErrors.error.routes.cantLoad, exc.getMessage());
    }
    appContext.getJournal().clear(SubjectTypes.TYPE_route);
    appContext.getJournal().clear(SubjectTypes.TYPE_subject);
    appContext.getStatuses().setSubjectsChanged(false);
    appContext.getStatuses().setRoutesChanged(false);
    appContext.getStatuses().setRoutesRestored(false);
    appContext.getStatuses().setRoutesLoaded(false);
    return RESULT_OK;
  }

  public boolean isSubjectChecked(String alias)
  {
    return checkedSubjectsSet.contains(alias);
  }

  /**
   * ***************** properties ************************
   */

  public String getEditName()
  {
    return editName;
  }

  public void setEditName(String editName)
  {
    this.editName = editName;
  }

  public String[] getCheckedSubjects()
  {
    return checkedSubjects;
  }

  public void setCheckedSubjects(String[] checkedSubjects)
  {
    this.checkedSubjects = checkedSubjects;
  }

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbLoad()
  {
    return mbLoad;
  }

  public void setMbLoad(String mbLoad)
  {
    this.mbLoad = mbLoad;
  }

  public String getMbRestore()
  {
    return mbRestore;
  }

  public void setMbRestore(String mbRestore)
  {
    this.mbRestore = mbRestore;
  }

  public QueryResultSet getSubjects()
  {
    return subjects;
  }

  public void setSubjects(QueryResultSet subjects)
  {
    this.subjects = subjects;
  }

  public String getMbQuery()
  {
    return mbQuery;
  }

  public void setMbQuery(String mbQuery)
  {
    this.mbQuery = mbQuery;
  }

  public String[] getFilter_masks()
  {
    return filter_masks;
  }

  public void setFilter_masks(String[] filter_masks)
  {
    this.filter_masks = filter_masks;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }
}
