/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 21:14:48
 */
package ru.novosoft.smsc.jsp;

import org.apache.log4j.Category;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.preferences.UserPreferences;
import ru.novosoft.smsc.admin.service.HostsManager;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpSession;
import java.util.*;

public abstract class PageBean
{
  public static final int RESULT_OK = 0;
  public static final int RESULT_DONE = 1;
  public static final int RESULT_ERROR = 2;
  protected static final int PRIVATE_RESULT = 3;

  protected Category logger = Category.getInstance(this.getClass());

  protected List errors = null;
  protected SMSCAppContext appContext = null;
  protected HostsManager hostsManager = null;
  protected java.security.Principal loginedPrincipal = null;
  protected HttpSession session = null;
  protected String sessionId = null;
  protected UserPreferences preferences = null;

  public int process(HttpServletRequest request)
  {
    this.errors = (List) request.getAttribute(Constants.SMSC_ERROR_MESSAGES_ATTRIBUTE_NAME);
    if (errors == null) {
      this.errors = new ArrayList();
      error(SMSCErrors.error.errorListNotInitialized);
    }

    this.appContext = (ru.novosoft.smsc.jsp.SMSCAppContext) request.getAttribute("appContext");
    if (this.appContext == null)
      return error(SMSCErrors.error.appContextNotInitialized);

    this.loginedPrincipal = request.getUserPrincipal();
    if (this.appContext != null)
      this.preferences = this.appContext.getUserPreferences(this.loginedPrincipal);

    session = request.getSession(false);
    sessionId = session != null ? session.getId() : "unknown";

    return init(errors);
  }

  protected int init(List errors)
  {
    hostsManager = appContext.getHostsManager();

    if (hostsManager == null)
      return error(SMSCErrors.error.serviceManagerNotInitialized);

    return RESULT_OK;
  }

  protected int error(String errorCode)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_ERROR));
  }

  protected int error(String errorCode, String param)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_ERROR, param));
  }

  protected int error(String errorCode, Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_ERROR, cause));
  }

  protected int error(String errorCode, String param, Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_ERROR, param, cause));
  }

  protected int warning(String errorCode)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_WARNING));
  }

  protected int warning(String errorCode, String param)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_WARNING, param));
  }

  protected int warning(String errorCode, Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_WARNING, cause));
  }

  protected int warning(String errorCode, String param, Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_WARNING, param, cause));
  }

  protected int message(String errorCode)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_MESSAGE));
  }

  protected int message(String errorCode, String param)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_MESSAGE, param));
  }

  protected int message(String errorCode, Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_MESSAGE, cause));
  }

  protected int message(String errorCode, String param, Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_MESSAGE, param, cause));
  }

  protected int _error(SMSCJspException e)
  {
    errors.add(e);
    return RESULT_ERROR;
  }

  public SMSCAppContext getAppContext()
  {
    return appContext;
  }

  public java.security.Principal getLoginedPrincipal()
  {
    return loginedPrincipal;
  }

  public void journalAppend(byte subjectType, String subjectId, byte action)
  {
    appContext.getJournal().append(loginedPrincipal.getName(), sessionId, subjectType, subjectId, action);
  }

  public void journalAppend(byte subjectType, String subjectId, byte action, Date timestamp, String additionalKey, String additionalValue)
  {
    appContext.getJournal().append(loginedPrincipal.getName(), sessionId, subjectType, subjectId, action, timestamp, additionalKey, additionalValue);
  }

  public void journalAppend(byte subjectType, String subjectId, byte action, Date timestamp, Map additional)
  {
    appContext.getJournal().append(loginedPrincipal.getName(), sessionId, subjectType, subjectId, action, timestamp, additional);
  }
}
