package ru.sibinco.smppgw.beans;

import org.apache.log4j.Logger;
import ru.sibinco.smppgw.backend.SmppGWAppContext;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;


/**
 * Created by igork
 * Date: 03.03.2004
 * Time: 19:37:44
 */
public abstract class SmppgwBean
{
  protected SmppGWAppContext appContext;
  protected Logger logger = Logger.getLogger(this.getClass());

  public void process(HttpServletRequest request, HttpServletResponse response) throws SmppgwJspException
  {
    appContext = (SmppGWAppContext) request.getAttribute("appContext");
  }

  public final SmppGWAppContext getAppContext()
  {
    return appContext;
  }
}
