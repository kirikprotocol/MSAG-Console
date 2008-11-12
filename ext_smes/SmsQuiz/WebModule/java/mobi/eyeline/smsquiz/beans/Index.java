package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.PageBean;

import javax.servlet.http.HttpServletRequest;

/**
 * author: alkhal
 * Date: 01.11.2008
 */
public class Index extends SmsQuizBean {

  private String mbStart = null;
  private String mbStop = null;


  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (mbStart != null)
      return start();
    if (mbStop != null)
      return stop();

    return result;
  }

  protected int start() {
    int result = RESULT_DONE;
    try {
      getAppContext().getHostsManager().startService(getSmeId());
      try {
        Thread.sleep(5000);
      } catch (InterruptedException e) {
        //do nothing
      }
    } catch (AdminException e) {
      logger.error("Could not start Info SME", e);
      result = error("infosme.error.start", e);
    }
    return result;
  }

  protected int stop() {
    int result = RESULT_DONE;
    try {
      getAppContext().getHostsManager().shutdownService(getSmeId());
    } catch (AdminException e) {
      logger.error("Could not stop Info SME", e);
      result = error("infosme.error.stop", e);
    }
    return result;
  }

  public String getMbStart() {
    return mbStart;
  }

  public void setMbStart(String mbStart) {
    this.mbStart = mbStart;
  }

  public String getMbStop() {
    return mbStop;
  }

  public void setMbStop(String mbStop) {
    this.mbStop = mbStop;
  }
}
