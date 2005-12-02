package ru.novosoft.smsc.infosme.backend.tables.tasks;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.InfoSme;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import java.util.*;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 12:48:07 PM
 */
public class TaskDataSource extends AbstractDataSourceImpl
{
  public static final String TASKS_PREFIX = "InfoSme.Tasks";
  private Category logger = Category.getInstance(this.getClass());
  private final InfoSme infoSme;

  public TaskDataSource(InfoSme infoSme)
  {
    super(new String[]{"name", "provider", "enabled", "priority", "retryOnFail", "replaceMessage", "svcType", "generating", "processing", "trackIntegrity"});
    this.infoSme = infoSme;
  }

  public QueryResultSet query(Config config, TaskQuery query_to_run)
  {
    clear();
    Set generatingTasks = null;
    Set processingTasks = null;
    try {
      generatingTasks = infoSme.getInfo().isOnline() ? new HashSet(infoSme.getGeneratingTasks()) : new HashSet();
      processingTasks = infoSme.getInfo().isOnline() ? new HashSet(infoSme.getProcessingTasks()) : new HashSet();
    } catch (AdminException e) {
      logger.error("Could not get tasks statuses", e);
    }
    for (Iterator i = config.getSectionChildShortSectionNames(TASKS_PREFIX).iterator(); i.hasNext();) {
      String taskId = (String) i.next();
      final String currentTaskPrefix = TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(taskId);
      try {
        add(new TaskDataItem(taskId,
                             config.getString(currentTaskPrefix + ".name"),
                             config.getString(currentTaskPrefix + ".dsId"),
                             config.getBool(currentTaskPrefix + ".enabled"),
                             config.getInt(currentTaskPrefix + ".priority"),
                             config.getBool(currentTaskPrefix + ".retryOnFail"),
                             config.getBool(currentTaskPrefix + ".replaceMessage"),
                             config.getString(currentTaskPrefix + ".svcType"),
                             generatingTasks != null ? generatingTasks.contains(taskId) : false,
                             processingTasks != null ? processingTasks.contains(taskId) : false,
                             config.getBool(currentTaskPrefix + ".trackIntegrity")));
      } catch (Exception e) {
        logger.error("Couldn't get parameter for task \"" + taskId + "\", task skipped", e);
      }
    }
    return super.query(query_to_run);
  }
}
