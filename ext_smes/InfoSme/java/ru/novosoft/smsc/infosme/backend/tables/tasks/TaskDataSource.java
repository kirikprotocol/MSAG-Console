package ru.novosoft.smsc.infosme.backend.tables.tasks;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.InfoSme;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSource;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

/**
 * Created by igork
 * Date: Sep 2, 2003
 * Time: 12:48:07 PM
 */
public class TaskDataSource extends AbstractDataSource
{
  public static final String TASKS_PREFIX = "InfoSme.Tasks";
  private Category logger = Category.getInstance(this.getClass());

  private final InfoSme infoSme;
  private final InfoSmeConfig config;

  public TaskDataSource(InfoSme infoSme, InfoSmeConfig config)
  {
    super(new String[]{"id", "name", "provider", "enabled", "priority", "retryOnFail", "replaceMessage", "svcType", "generating", "processing", "trackIntegrity", "startDate", "endDate", "owner", "delivery"});
    this.infoSme = infoSme;
    this.config = config;
  }
  

  public QueryResultSet query(Query query_to_run)
  {
    init(query_to_run);
    Set generatingTasks = new HashSet();
    Set processingTasks = new HashSet();
    try {
      if (infoSme.getInfo().isOnline()) {
        generatingTasks.addAll(infoSme.getGeneratingTasks());
        processingTasks.addAll(infoSme.getProcessingTasks());
      }
      for (Iterator iter = config.getTasks(null).iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        TaskDataItem item = new TaskDataItem(t.getId(), t.getName(), t.getProvider(), t.isEnabled(), t.getPriority(), t.getRetryPolicy(), t.isReplaceMessage(),
            t.getSvcType(), generatingTasks.contains(t.getId()), processingTasks.contains(t.getId()), t.isTrackIntegrity(), t.getStartDate(), t.getEndDate(), t.getOwner(), t.isDelivery());
        add(item);
      }
    } catch (AdminException e) {
      logger.error("Could not get tasks statuses", e);
    }
            
    return getResults();
  }

}
