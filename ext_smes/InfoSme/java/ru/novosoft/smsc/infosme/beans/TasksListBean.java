package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskQuery;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 02.10.2003
 * Time: 17:09:43
 */
public abstract class TasksListBean extends InfoSmeBean
{
  private String sort = null;
  private String edit = "";
  private String[] checked = new String[0];
  private Collection checkedSet = null;
  private int startPosition = 0;
  private int totalSize = 0;
  private int pageSize = 0;
  private QueryResultSet tasks = new EmptyResultSet();

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    checkedSet = new HashSet(Arrays.asList(checked));

    if (sort != null)
      getInfoSmeContext().setTasksSort(sort);
    else
      sort = getInfoSmeContext().getTasksSort();
    if (pageSize != 0)
      getInfoSmeContext().setTasksPageSize(pageSize);
    else
      pageSize = getInfoSmeContext().getTasksPageSize();

    return result;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    tasks = new TaskDataSource(getInfoSmeContext().getInfoSme()).query(getConfig(), new TaskQuery(pageSize, sort, startPosition));
    totalSize = tasks.getTotalSize();

    return result;
  }

  public QueryResultSet getTasks()
  {
    return tasks;
  }

  public boolean isTaskChecked(String taskName)
  {
    return checkedSet.contains(taskName);
  }

  public String getEdit()
  {
    return edit;
  }

  public void setEdit(String edit)
  {
    this.edit = edit;
  }

  public String[] getChecked()
  {
    return checked;
  }

  public void setChecked(String[] checked)
  {
    this.checked = checked;
  }

  /****************** IndexBean delegates *****************/
  public String getStartPosition()
  {
    return String.valueOf(startPosition);
  }

  public int getStartPositionInt()
  {
    return startPosition;
  }

  public void setStartPosition(String startPosition)
  {
    try {
      this.startPosition = Integer.decode(startPosition).intValue();
    } catch (NumberFormatException e) {
      this.startPosition = 0;
    }
  }

  public String getTotalSize()
  {
    return String.valueOf(totalSize);
  }

  public int getTotalSizeInt()
  {
    return totalSize;
  }

  public void setTotalSize(String totalSize)
  {
    try {
      this.totalSize = Integer.decode(totalSize).intValue();
    } catch (NumberFormatException e) {
      this.totalSize = 0;
    }
  }

  public void setPageSize(String pageSize)
  {
    try {
      this.pageSize = Integer.decode(pageSize).intValue();
    } catch (NumberFormatException e) {
      this.pageSize = 0;
    }
  }

  public String getSort()
  {
    return sort;
  }

  public void setSort(String sort)
  {
    this.sort = sort;
  }

  public String getPageSize()
  {
    return String.valueOf(pageSize);
  }

  public int getPageSizeInt()
  {
    return pageSize;
  }

  protected Collection getCheckedSet()
  {
    return checkedSet;
  }

}
