package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskFilter;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TasksTableHelper;
import ru.novosoft.smsc.infosme.backend.config.ConfigChanges;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.admin.AdminException;

import javax.servlet.http.HttpServletRequest;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Iterator;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:44:07 PM
 */
public class Tasks extends TasksListBean
{
  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

  public static final int RESULT_EDIT = InfoSmeBean.PRIVATE_RESULT + 0;
  public static final int RESULT_ADD = InfoSmeBean.PRIVATE_RESULT + 1;
  protected static final int PRIVATE_RESULT = InfoSmeBean.PRIVATE_RESULT + 3;

  private String mbAdd = null;
  private String mbEdit = null;
  private String mbDelete = null;
  private String mbClear = null;
  private String mbApply = null;
  private String mbReset = null;

  private String edit;

  public Tasks() {
    tasksFilter = new TaskFilter();
  }

  public int process(HttpServletRequest request)
  {
    if (mbClear != null) {
      mbClear = null;
      ((TaskFilter)tasksFilter).clear();
    }

    String owner = isUserAdmin(request) ? null : request.getRemoteUser();

    setTableHelperMode(isUserAdmin(request) ? TasksTableHelper.MODE_ADMIN : TasksTableHelper.MODE_MARKET, true);
    
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (!isSmeRunning())
      message("infosme.prompt.no_task_edit");

    if (mbAdd != null)
      return Tasks.RESULT_ADD;
    if (mbDelete != null)
      return delete();
    if (mbApply != null)
      return apply(owner);
    if (mbReset != null)
      return reset(owner);

    if (getSelectedTaskId() != null) {
      edit = getSelectedTaskId();
      return Tasks.RESULT_EDIT;
    }

    return result;
  }

  private int apply(String owner) {
    try {
      ConfigChanges changes = getInfoSmeConfig().apply(owner, false, true, false, false, false, false);
      // Notify InfoSme about new tasks
      for (Iterator iter = changes.getTasksChanges().getAdded().iterator(); iter.hasNext();)
        getInfoSme().addTask((String)iter.next());
      // Notify InfoSme about deleted tasks
      for (Iterator iter = changes.getTasksChanges().getDeleted().iterator(); iter.hasNext();)
        getInfoSme().removeTask((String)iter.next());
      // Notify InfoSme about changed tasks
      for (Iterator iter = changes.getTasksChanges().getModified().iterator(); iter.hasNext();)
        getInfoSme().changeTask((String)iter.next());

      return RESULT_DONE;
    } catch (AdminException e) {
      return error(e.getMessage(), e);
    }
  }

  private int reset(String owner) {
    try {
      getInfoSmeConfig().reset(owner, false, true, false, false, false, false);
      return RESULT_DONE;
    } catch (AdminException e) {
      return error(e.getMessage(), e);
    }
  }

  private int delete()
  {
    List checked = getChecked();
    for (int i = 0; i < getChecked().size(); i++)
      getInfoSmeConfig().removeTask((String)checked.get(i));
    return RESULT_DONE;
  }

  public String getMbAdd() {
    return mbAdd;
  }
  public void setMbAdd(String mbAdd) {
    this.mbAdd = mbAdd;
  }
  public String getMbEdit() {
    return mbEdit;
  }
  public void setMbEdit(String mbEdit) {
    this.mbEdit = mbEdit;
  }
  public String getMbDelete() {
    return mbDelete;
  }
  public void setMbDelete(String mbDelete) {
    this.mbDelete = mbDelete;
  }

  private Date convertStringToDate(String date) {
    Date converted = new Date();
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      converted = formatter.parse(date);
    } catch (ParseException e) {
      e.printStackTrace();
    }
    return converted;
  }
  public String convertDateToString(Date date) {
    if (date == null)
      return "";
    SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
    return formatter.format(date);
  }

  public void setFromDate(String startDate) {
    ((TaskFilter)tasksFilter).setStartDate(convertStringToDate(startDate));
  }

  public String getFromDate() {
    return convertDateToString(((TaskFilter)tasksFilter).getStartDate());
  }

  public void setTillDate(String endDate) {
    ((TaskFilter)tasksFilter).setEndDate(convertStringToDate(endDate));
  }

  public String getTillDate() {
    return convertDateToString(((TaskFilter)tasksFilter).getEndDate());
  }

  public void setName(String name) {
    ((TaskFilter)tasksFilter).setName(name.trim().length() == 0 ? null : name);
  }

  public String getName() {
    String  name = ((TaskFilter)tasksFilter).getName();
    return name == null ? "" : name;
  }

  public String getMbClear() {
    return mbClear;
  }

  public void setMbClear(String mbClear) {
    this.mbClear = mbClear;
  }

  public void setStatus(int status) {
    if (status == 0)
      ((TaskFilter)tasksFilter).setActive(null);
    else if (status == 1)
      ((TaskFilter)tasksFilter).setActive(Boolean.TRUE);
    else
      ((TaskFilter)tasksFilter).setActive(Boolean.FALSE);
  }

  public int getStatus() {
    if (((TaskFilter)tasksFilter).getActive() == null)
      return 0;
    else if (((TaskFilter)tasksFilter).getActive().booleanValue())
      return 1;
    return 2;
  }

  public String getEdit() {
    return edit;
  }

  public String getMbApply() {
    return mbApply;
  }

  public void setMbApply(String mbApply) {
    this.mbApply = mbApply;
  }

  public String getMbReset() {
    return mbReset;
  }

  public void setMbReset(String mbReset) {
    this.mbReset = mbReset;
  }
}
