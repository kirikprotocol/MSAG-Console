package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.config.ConfigChanges;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskFilter;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskVisitor;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TasksTableHelper;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataItem;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Iterator;
import java.util.List;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:44:07 PM
 */
public class Tasks extends TasksListBean
{
  private static final String DATE_FORMAT = "dd.MM.yyyy HH:mm:ss";

  public static final int RESULT_EDIT = InfoSmeBean.PRIVATE_RESULT;
  public static final int RESULT_ADD = InfoSmeBean.PRIVATE_RESULT + 1;
  protected static final int PRIVATE_RESULT = InfoSmeBean.PRIVATE_RESULT + 3;

  private String mbAdd = null;
  private String mbEdit = null;
  private String mbDelete = null;
  private String mbClear = null;
  private String mbApply = null;
  private String mbReset = null;
  private String mbEnable = null;
  private String mbDisable = null;
  private String mbEnableAll = null;
  private String mbDisableAll = null;

  private String edit;

  public Tasks() {
    tasksFilter = new TaskFilter();
  }

  public int process(HttpServletRequest request)
  {
    if (mbClear != null) {
      mbClear = null;
      tasksFilter.clear();
    }

    String owner = isUserAdmin(request) ? null : request.getRemoteUser();

    setTableHelperMode(isUserAdmin(request) ? TasksTableHelper.MODE_ADMIN : TasksTableHelper.MODE_MARKET, true);
    
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (!isSmeRunning())
      message("infosme.prompt.no_task_edit");

    if (mbAdd != null) {mbAdd = null; return Tasks.RESULT_ADD;}
    if (mbDelete != null) {mbDelete = null; return delete();}
    if (mbApply != null) {mbApply = null; return apply(request.getRemoteUser(), owner);}
    if (mbReset != null) {mbReset = null; return reset(request.getRemoteUser(), owner);}
    if (mbEnable != null) {mbEnable = null; result = setSelectedTasksEnabled(true);}
    if (mbDisable != null) {mbDisable = null; result = setSelectedTasksEnabled(false);}
    if (mbEnableAll != null) {mbEnableAll = null; result = setAllTasksEnabled(request.getRemoteUser(), isUserAdmin(request), true);}
    if (mbDisableAll != null) {mbDisableAll = null;  result = setAllTasksEnabled(request.getRemoteUser(), isUserAdmin(request), false);}

    if (getSelectedTaskId() != null) {
      edit = getSelectedTaskId();
      return Tasks.RESULT_EDIT;
    }

    if (!isUserAdmin(request) && getInfoSmeConfig().isTasksChanged(owner))
      message("Tasks configuration has been changed.");
    
    return result;
  }

  private int apply(String user, String owner) {
    try {
      ConfigChanges changes = getInfoSmeConfig().apply(user, owner, false, true, false, false, false, false);
      // Notify InfoSme about new tasks
      for (Iterator iter = changes.getTasksChanges().getAdded().iterator(); iter.hasNext();)
        getInfoSme().addTask((String)iter.next());
      // Notify InfoSme about deleted tasks
      for (Iterator iter = changes.getTasksChanges().getDeleted().iterator(); iter.hasNext();)
        getInfoSme().removeTask((String)iter.next());
      // Notify InfoSme about changed tasks
      for (Iterator iter = changes.getTasksChanges().getModified().iterator(); iter.hasNext();)
        getInfoSme().changeTask((String)iter.next());

      return RESULT_OK;
    } catch (AdminException e) {
      return error(e.getMessage(), e);
    }
  }

  private int reset(String user, String owner) {
    try {
      getInfoSmeConfig().reset(user, owner, false, true, false, false, false, false);
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

  private int setSelectedTasksEnabled(boolean enabled) {
    List checked = getChecked();
    for (int i = 0; i < checked.size(); i++) {
      Task t = getInfoSmeConfig().getTask((String)checked.get(i));
      t.setEnabled(enabled);
    }
    return RESULT_DONE;
  }

  private int setAllTasksEnabled(final String user, final boolean admin, final boolean enabled) {
    visitAll(new TaskVisitor() {
      public boolean visit(TaskDataItem t) {
        if (admin || t.getOwner() == null || t.getOwner().equals(user)) {
          Task task = getInfoSmeConfig().getTask(t.getId());
          task.setEnabled(enabled);
        }
        return true;
      }
    });
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

  private static Date convertStringToDate(String date) {
    Date converted = new Date();
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      converted = formatter.parse(date);
    } catch (ParseException e) {
      e.printStackTrace();
    }
    return converted;
  }

  public static String convertDateToString(Date date) {
    if (date == null)
      return "";
    SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
    return formatter.format(date);
  }

  public void setFromDate(String startDate) {
    tasksFilter.setStartDate(convertStringToDate(startDate));
  }

  public String getFromDate() {
    return convertDateToString(tasksFilter.getStartDate());
  }

  public void setTillDate(String endDate) {
    tasksFilter.setEndDate(convertStringToDate(endDate));
  }

  public String getTillDate() {
    return convertDateToString(tasksFilter.getEndDate());
  }

  public void setName(String name) {
    tasksFilter.setName(name.trim().length() == 0 ? null : name);
  }

  public String getName() {
    String  name = tasksFilter.getName();
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
      tasksFilter.setActive(null);
    else if (status == 1)
      tasksFilter.setActive(Boolean.TRUE);
    else
      tasksFilter.setActive(Boolean.FALSE);
  }

  public int getStatus() {
    if (tasksFilter.getActive() == null)
      return 0;
    else if (tasksFilter.getActive().booleanValue())
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

  public String getMbEnable() {
    return mbEnable;
  }

  public void setMbEnable(String mbEnable) {
    this.mbEnable = mbEnable;
  }

  public String getMbDisable() {
    return mbDisable;
  }

  public void setMbDisable(String mbDisable) {
    this.mbDisable = mbDisable;
  }

  public String getMbEnableAll() {
    return mbEnableAll;
  }

  public void setMbEnableAll(String mbEnableAll) {
    this.mbEnableAll = mbEnableAll;
  }

  public String getMbDisableAll() {
    return mbDisableAll;
  }

  public void setMbDisableAll(String mbDisableAll) {
    this.mbDisableAll = mbDisableAll;
  }    
}
