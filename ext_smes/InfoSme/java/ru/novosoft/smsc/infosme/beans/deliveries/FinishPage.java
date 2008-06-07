package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.infosme.backend.tables.tasks.TaskDataSource;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import javax.servlet.http.HttpServletRequest;
import java.util.Iterator;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class FinishPage extends DeliveriesPage {

  protected FinishPage(DeliveriesPageData pageData) {
    super(pageData);
  }

  public DeliveriesPage mbNext(HttpServletRequest request) throws AdminException {
    try {
      for (Iterator iter = pageData.getTask().tasks().iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        pageData.getInfoSmeContext().getInfoSme().endDeliveryMessageGeneration(t.getId());
        pageData.getInfoSmeContext().getConfig().setBool(TaskDataSource.TASKS_PREFIX + '.' + StringEncoderDecoder.encodeDot(t.getId()) + ".messagesHaveLoaded", true);
      }
      pageData.getInfoSmeContext().getConfig().save();

      if (pageData.getDeliveriesFile() != null && pageData.getDeliveriesFile().isFile() && pageData.getDeliveriesFile().exists())
        pageData.getDeliveriesFile().delete();

      return new StartPage(pageData);
    } catch (Exception e) {
      throw new AdminException(e.getMessage());
    }
  }

  public DeliveriesPage mbCancel(HttpServletRequest request) throws AdminException {
    return new StartPage(pageData);
  }

  public DeliveriesPage mbUpdate(HttpServletRequest request) throws AdminException {
    return this;
  }

  public int getId() {
    return FINISH_TASK_PAGE;
  }
}
