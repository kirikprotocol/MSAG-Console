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
import java.security.Principal;

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
  protected Principal loginedPrincipal = null;
  protected HttpSession session = null;
  protected String sessionId = null;
  protected UserPreferences preferences = null;

  public int process(final HttpServletRequest request)
  {
    this.errors = (List) request.getAttribute(Constants.SMSC_ERROR_MESSAGES_ATTRIBUTE_NAME);
    if (null == errors) {
      this.errors = new ArrayList();
      error(SMSCErrors.error.errorListNotInitialized);
    }

    this.appContext = (SMSCAppContext) request.getAttribute("appContext");
    if (null == this.appContext)
      return error(SMSCErrors.error.appContextNotInitialized);

    this.loginedPrincipal = request.getUserPrincipal();
    if (null != this.appContext)
      this.preferences = this.appContext.getUserPreferences(this.loginedPrincipal);

    session = request.getSession(false);
    sessionId = null != session ? session.getId() : "unknown";

    return init(errors);
  }

  protected int init(final List errors)
  {
    hostsManager = appContext.getHostsManager();

    if (null == hostsManager)
      return error(SMSCErrors.error.serviceManagerNotInitialized);

    return RESULT_OK;
  }

  protected int error(final String errorCode)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_ERROR));
  }

  protected int error(final String errorCode, final String param)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_ERROR, param));
  }

  protected int error(final String errorCode, final Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_ERROR, cause));
  }

  protected int error(final String errorCode, final String param, final Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_ERROR, param, cause));
  }

  protected int warning(final String errorCode)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_WARNING));
  }

  protected int warning(final String errorCode, final String param)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_WARNING, param));
  }

  protected int warning(final String errorCode, final Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_WARNING, cause));
  }

  protected int warning(final String errorCode, final String param, final Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_WARNING, param, cause));
  }

  protected int message(final String errorCode)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_MESSAGE));
  }

  protected int message(final String errorCode, final String param)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_MESSAGE, param));
  }

  protected int message(final String errorCode, final Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_MESSAGE, cause));
  }

  protected int message(final String errorCode, final String param, final Throwable cause)
  {
    return _error(new SMSCJspException(errorCode, SMSCJspException.ERROR_CLASS_MESSAGE, param, cause));
  }

  protected int _error(final SMSCJspException e)
  {
    errors.add(e);
    return RESULT_ERROR;
  }

  public SMSCAppContext getAppContext()
  {
    return appContext;
  }

  public Principal getLoginedPrincipal()
  {
    return loginedPrincipal;
  }

  public void journalAppend(final byte subjectType, final String subjectId, final byte action)
  {
    appContext.getJournal().append(loginedPrincipal.getName(), sessionId, subjectType, subjectId, action);
  }

  public void journalAppend(final byte subjectType, final String subjectId, final byte action, final String additionalKey, final String additionalValue)
  {
    appContext.getJournal().append(loginedPrincipal.getName(), sessionId, subjectType, subjectId, action, additionalKey, additionalValue);
  }

  public void journalAppend(final byte subjectType, final String subjectId, final byte action, final Map additional)
  {
    appContext.getJournal().append(loginedPrincipal.getName(), sessionId, subjectType, subjectId, action, additional);
  }
}
