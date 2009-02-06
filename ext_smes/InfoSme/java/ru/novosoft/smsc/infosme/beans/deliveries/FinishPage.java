package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.infosme.backend.config.tasks.Task;
import ru.novosoft.smsc.util.smsxsender.SmsXSender;
import ru.novosoft.smsc.util.smsxsender.SmsXSenderResponse;

import javax.servlet.http.HttpServletRequest;
import java.io.File;
import java.util.Iterator;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class FinishPage extends DeliveriesPage {

  protected FinishPage(DeliveriesPageData pageData) {
    super(pageData);
  }

  private void removeTask() throws Exception {
    for (Iterator iter = pageData.getTask().tasks().iterator(); iter.hasNext();) {
      Task t = (Task)iter.next();
      if (t != null) {
        if (log.isDebugEnabled())
          log.debug("Remove " + t.getName() + " from config: " + t.getId());
        pageData.getInfoSmeContext().getInfoSmeConfig().removeAndApplyTask(t.getId());
        try {
          pageData.getInfoSmeContext().getInfoSme().removeTask(t.getId());
        } catch (Throwable e) {
         log.error("Unable to remove task:" + pageData.name, e);
        }
      }
    }
  }

  public DeliveriesPage mbNext(HttpServletRequest request) throws AdminException {
    try {
      if (pageData.secret) {
        try {
          SmsXSender smsxSender = pageData.getAppContext().getSmsXSender();
          for (Iterator iter = pageData.getInputFiles().values().iterator(); iter.hasNext();) {
            File f = (File)iter.next();
            SmsXSenderResponse r = smsxSender.batchSecret(pageData.sourceAddress, pageData.secretText, pageData.secretFlash, f);
            if (r.getStatus() != 0)
              throw new AdminException("Batch secret error: code=" + r.getStatus());
          }
        } catch (Throwable e) {
          log.error(e,e);
          removeTask();
          throw new AdminException("Unable to save secret messages. Reason:" + e.getMessage());
        }
      }

      for (Iterator iter = pageData.getTask().tasks().iterator(); iter.hasNext();) {
        Task t = (Task)iter.next();
        t.setMessagesHaveLoaded(true);
        pageData.getInfoSmeContext().getInfoSmeConfig().addAndApplyTask(t);
        pageData.getInfoSmeContext().getInfoSme().endDeliveryMessageGeneration(t.getId());
      }

      if (pageData.getDeliveriesFile() != null && pageData.getDeliveriesFile().isFile() && pageData.getDeliveriesFile().exists())
        pageData.getDeliveriesFile().delete();

      return new LoadFilePage(pageData);
    } catch (Exception e) {
      log.error(e,e);
      throw new AdminException(e.getMessage());
    }
  }

  public DeliveriesPage mbCancel(HttpServletRequest request) throws AdminException {
    try {
      removeTask();
    } catch (Exception e) {
      log.error(e,e);
    }
    return new LoadFilePage(pageData);
  }

  public DeliveriesPage mbUpdate(HttpServletRequest request) throws AdminException {
    return this;
  }

  public int getId() {
    return FINISH_TASK_PAGE;
  }
}
