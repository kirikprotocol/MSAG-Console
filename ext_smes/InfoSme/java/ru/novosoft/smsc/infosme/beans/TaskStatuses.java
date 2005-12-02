package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.InfoSme;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.SortedList;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 03.10.2003
 * Time: 20:31:44
 */
public class TaskStatuses extends InfoSmeBean
{
  private List taskIds = new ArrayList();
  private Set generatingTasks = new HashSet();
  private Set processingTasks = new HashSet();

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    taskIds = new SortedList(getConfig().getSectionChildShortSectionNames(TaskDataSource.TASKS_PREFIX));
    try {
      final InfoSme infoSme = getInfoSmeContext().getInfoSme();
      generatingTasks = infoSme.getInfo().isOnline() ? new HashSet(infoSme.getGeneratingTasks()) : new HashSet();
      processingTasks = infoSme.getInfo().isOnline() ? new HashSet(infoSme.getProcessingTasks()) : new HashSet();
    } catch (AdminException e) {
      logger.error("Could not get tasks statuses", e);
      result = error("infosme.error.get_tasks_statuses", e);
    }

    return result;
  }

  public List getTaskIds()
  {
    return taskIds;
  }

  public boolean isTaskGenerating(String taskId)
  {
    return generatingTasks.contains(taskId);
  }

  public boolean isTaskProcessing(String taskId)
  {
    return processingTasks.contains(taskId);
  }
}
