package ru.novosoft.smsc.infosme.backend.tables.tasks;

import org.apache.log4j.Category;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.config.Config;

import java.util.Iterator;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 12:48:07 PM
 */
public class TaskDataSource extends AbstractDataSourceImpl {
  public static final String TASKS_PREFIX = "InfoSme.Tasks";
  private Category logger = Category.getInstance(this.getClass());

  public TaskDataSource()
  {
    super(new String[]{"name", "provider", "enabled", "priority", "retryOnFail", "replaceMessage", "svcType"});
  }

  public QueryResultSet query(Config config, TaskQuery query_to_run)
  {
    clear();
    for (Iterator i = config.getSectionChildShortSectionNames(TASKS_PREFIX).iterator(); i.hasNext();) {
      String taskName = (String) i.next();
      final String currentTaskPrefix = TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(taskName);
      try {
        add(new TaskDataItem(taskName,
                             config.getString(currentTaskPrefix + ".dsId"),
                             config.getBool(currentTaskPrefix + ".enabled"),
                             config.getInt(currentTaskPrefix + ".priority"),
                             config.getBool(currentTaskPrefix + ".retryOnFail"),
                             config.getBool(currentTaskPrefix + ".replaceMessage"),
                             config.getString(currentTaskPrefix + ".svcType")
        ));
      } catch (Exception e) {
        logger.error("Couldn't get parameter for task \"" + taskName + "\", task skipped", e);
      }
    }
    return super.query(query_to_run);
  }
}
