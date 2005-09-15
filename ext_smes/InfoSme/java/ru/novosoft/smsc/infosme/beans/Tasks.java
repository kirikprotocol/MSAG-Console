package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:44:07 PM
 */
public class Tasks extends TasksListBean
{
  public static final int RESULT_EDIT = InfoSmeBean.PRIVATE_RESULT + 0;
  public static final int RESULT_ADD = InfoSmeBean.PRIVATE_RESULT + 1;
  protected static final int PRIVATE_RESULT = InfoSmeBean.PRIVATE_RESULT + 3;
  private String mbAdd = null;
  private String mbEdit = null;
  private String mbDelete = null;

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (!isSmeRunning())
      message("Tasks cannot be changed becouse Info SME is not running.");

    if (mbAdd != null)
      return Tasks.RESULT_ADD;
    if (mbEdit != null)
      return Tasks.RESULT_EDIT;
    if (mbDelete != null)
      return delete();

    return result;
  }

  private int delete()
  {
    for (int i = 0; i < getChecked().length; i++) {
      getConfig().removeSection(TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(getChecked()[i]));
      getCheckedSet().remove(getChecked()[i]);
      getInfoSmeContext().setChangedTasks(true);
    }

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

}
