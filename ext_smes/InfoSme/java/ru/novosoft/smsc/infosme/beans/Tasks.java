package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskQuery;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.security.Principal;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:44:07 PM
 */
public class Tasks extends InfoSmeBean {
  public static final int RESULT_EDIT = InfoSmeBean.PRIVATE_RESULT + 0;
  public static final int RESULT_ADD = InfoSmeBean.PRIVATE_RESULT + 1;
  public static final int RESULT_REFRESH = InfoSmeBean.PRIVATE_RESULT + 2;
  protected static final int PRIVATE_RESULT = InfoSmeBean.PRIVATE_RESULT + 3;

  private String sort = null;
  private String edit = "";
  private String[] checked = new String[0];
  private Collection checkedSet = null;

  private String mbAdd = null;
  private String mbEdit = null;
  private String mbDelete = null;

  protected int startPosition = 0;
  protected int totalSize = 0;
  protected int pageSize = 0;
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

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbAdd != null)
      return RESULT_ADD;
    if (mbEdit != null)
      return RESULT_EDIT;
    if (mbDelete != null)
      return delete();

    tasks = new TaskDataSource().query(getConfig(), new TaskQuery(pageSize, sort, startPosition));
    totalSize = tasks.getTotalSize();

    return result;
  }

  private int delete()
  {
    for (int i = 0; i < checked.length; i++) {
      getConfig().removeSection(TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(checked[i]));
      checkedSet.remove(checked[i]);
    }

    return RESULT_DONE;
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

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
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

}
