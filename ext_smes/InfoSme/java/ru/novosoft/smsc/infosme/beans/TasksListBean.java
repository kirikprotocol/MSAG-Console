package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.tables.tasks.*;
import ru.novosoft.smsc.jsp.util.helper.statictable.TableHelperException;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;
import ru.novosoft.smsc.admin.AdminException;

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
  public static final int VIEW_LIST = 0;
  public static final int VIEW_WEEKLY = 1;

  protected TaskFilter tasksFilter;
  protected PagedStaticTableHelper tableHelper;
  private int mode;
  private boolean selectable;
  private List checked;
  private int view = VIEW_LIST;

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

  public int getView() {
    return view;
  }

  public void setView(int view) {
    this.view = view;
  }

  protected String getSelectedTaskId() {
    return tableHelper.getSelectedCellId();
  }

  protected List getChecked() {
    return checked;
  }

  protected void visitAll(TaskVisitor visitor) {
    TaskDataSource tds = new TaskDataSource(getInfoSme(), getInfoSmeConfig());
    try {
      tds.visit(visitor, tasksFilter);
    } catch (AdminException e) {
      logger.error(e,e);
    }
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
      if (view == VIEW_LIST) {
        tableHelper = new TasksTableHelper("tasksTableHelper", mode, getInfoSmeContext(), tasksFilter, selectable, getUser(request));
        tableHelper.setPageSize(getInfoSmeContext().getTasksPageSize());
      } else {
        tableHelper = new TaskWeeklyTableHelper("tasksWeeklyTableHelper", getInfoSmeContext(), tasksFilter);
      }
      tableHelper.processRequest(request);
      checked = view == VIEW_LIST ? ((TasksTableHelper)tableHelper).getSelectedTasksList(request) : null;
    } catch (Exception e) {
      logger.error(e,e);
      return error(e.getMessage());
    }

    return result;
  }

  public PagedStaticTableHelper getTableHelper() {
    return tableHelper;
  }
}
