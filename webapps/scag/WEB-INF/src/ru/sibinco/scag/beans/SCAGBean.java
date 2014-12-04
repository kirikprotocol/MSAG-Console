package ru.sibinco.scag.beans;

import org.apache.log4j.Logger;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.web.security.AuthFilter;
import ru.sibinco.scag.web.security.UserLoginData;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.util.List;
import java.util.ArrayList;


public abstract class SCAGBean
{
  protected SCAGAppContext appContext;
  protected Logger logger = Logger.getLogger(this.getClass());
  protected String userName = null;
  protected List errors = null;

  public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException{
    this.errors = (List) request.getAttribute(Constants.SCAG_ERROR_MESSAGES_ATTRIBUTE_NAME);
    if (null == errors) {
      this.errors = new ArrayList();
      error("error.errorListNotInitialized");
    }
    appContext = (SCAGAppContext) request.getAttribute(Constants.APP_CONTEXT);
    HttpSession session = request.getSession();
    if (session != null && session.getAttribute(AuthFilter.USER_LOGIN_DATA) != null){
      UserLoginData userLoginData = (UserLoginData) session.getAttribute(AuthFilter.USER_LOGIN_DATA);
      userName = userLoginData.getName();
    }
  }

  protected void error(final String errorCode){
    _error(new MessageException(errorCode, "ERROR_CLASS_ERROR"));
  }

  protected void error(final String errorCode, final String param){
    _error(new MessageException(errorCode,  param));
  }
  protected void _error(final MessageException e){
    errors.add(e);
  }

  public final SCAGAppContext getAppContext() {
    return appContext;
  }

  public String getUserName() {
    return userName;
  }

}
