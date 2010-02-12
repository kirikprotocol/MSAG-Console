package ru.sibinco.scag.beans;

import org.apache.log4j.Logger;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.Constants;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.List;
import java.util.ArrayList;


/**
 * Created by igork
 * Date: 03.03.2004
 * Time: 19:37:44
 */
public abstract class SCAGBean
{
  protected SCAGAppContext appContext;
  protected Logger logger = Logger.getLogger(this.getClass());
  protected java.security.Principal loginedPrincipal = null;
  protected List errors = null;

  public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException
  {
    this.errors = (List) request.getAttribute(Constants.SCAG_ERROR_MESSAGES_ATTRIBUTE_NAME);
    if (null == errors) {
      this.errors = new ArrayList();
      error("error.errorListNotInitialized");
    }
    appContext = (SCAGAppContext) request.getAttribute(Constants.APP_CONTEXT);
    loginedPrincipal = request.getUserPrincipal();
  }
  protected void error(final String errorCode)
  {
     _error(new MessageException(errorCode, "ERROR_CLASS_ERROR"));
  }
   protected void error(final String errorCode, final String param)
  {
     _error(new MessageException(errorCode,  param));
  }
   protected void _error(final MessageException e)
  {
    errors.add(e);
  }
  public final SCAGAppContext getAppContext() {
    return appContext;
  }

  public java.security.Principal getLoginedPrincipal() {
    return loginedPrincipal;
  }
  }
