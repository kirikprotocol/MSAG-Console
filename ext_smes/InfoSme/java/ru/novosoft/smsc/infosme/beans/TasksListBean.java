package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskFilter;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TasksTableHelper;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 02.10.2003
 * Time: 17:09:43
 */
public abstract class TasksListBean extends InfoSmeBean
{
  protected TaskFilter tasksFilter;
  protected TasksTableHelper tableHelper;
  private int mode;
  private boolean selectable;
  private List checked;

  protected TasksListBean(TaskFilter tasksFilter) {
    this.tasksFilter = tasksFilter;
  }

  protected TasksListBean() {
    this(new TaskFilter());
  }

  protected void setTableHelperMode(int mode, boolean selectable) {
    this.mode = mode;
    this.selectable = selectable;
  }

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    return result;
  }

  protected String getSelectedTaskId() {
    return tableHelper.getSelectedCellId();
  }

  protected List getChecked() {
    return checked;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    final boolean admin = isUserAdmin(request);
    final String userName = request.getRemoteUser();

    if (!admin)
      tasksFilter.setOwner(userName);

    try {
      tableHelper = new TasksTableHelper("tasksTableHelper", mode, getInfoSmeContext(), tasksFilter, selectable, getUser(request));
      tableHelper.setPageSize(getInfoSmeContext().getTasksPageSize());
      tableHelper.processRequest(request);
      checked = tableHelper.getSelectedTasksList(request);
    } catch (TableHelperException e) {
      return error(e.getMessage());
    }

    try {
      tableHelper.fillTable();
    } catch (TableHelperException e) {
      return error(e.getMessage());
    }

    return result;
  }

  public TasksTableHelper getTableHelper() {
    return tableHelper;
  }
}
