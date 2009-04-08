package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.deliveries.DeliveriesGenerationThread;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class GenerateDeliveriesPage extends DeliveriesPage {

  private final DeliveriesGenerationThread thread;

  public GenerateDeliveriesPage(DeliveriesPageData pageData) {
    super(pageData);
    thread = new DeliveriesGenerationThread(pageData.getInfoSmeContext(), pageData.getTask());
    thread.start();
  }

  public DeliveriesPage mbNext(HttpServletRequest request) throws AdminException {
    switch (thread.getStatus()) {
      case DeliveriesGenerationThread.STATUS_CANCELED:
      case DeliveriesGenerationThread.STATUS_ERR:
        return new LoadFilePage(pageData);
      case DeliveriesGenerationThread.STATUS_DONE:
        pageData.deliveriesGenStatus = DeliveriesGenerationThread.STATUS_DONE;
        return new FinishPage(pageData);
      default:
        return this;
    }
  }

  public DeliveriesPage mbCancel(HttpServletRequest request) throws AdminException {
    thread.cancel();
    return this;
  }

  public DeliveriesPage mbUpdate(HttpServletRequest request) throws AdminException {
    pageData.deliveriesGenProgr = thread.getProgress();
    pageData.deliveriesGenStatus = thread.getStatus();
    pageData.errorStr = thread.getErrorText();
    return this;
  }

  public int getId() {
    return GEN_TASK_PAGE;
  }
}
