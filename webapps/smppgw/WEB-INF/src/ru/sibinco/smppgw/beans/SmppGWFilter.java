/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 25.02.2004
 * Time: 17:56:11
 */
package ru.sibinco.smppgw.beans;

import org.apache.log4j.Logger;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.smppgw.backend.SmppGWAppContext;

import javax.servlet.*;
import javax.servlet.http.HttpServletRequest;
import java.io.IOException;


public class SmppGWFilter implements Filter
{
  private String defaultEncoding = "ISO-8859-1";
  FilterConfig config = null;
  SmppGWAppContext appContext = null;
  private Logger logger;

  public void init(final FilterConfig config) throws ServletException
  {
    logger = Logger.getLogger(this.getClass());

    defaultEncoding = Functions.getLocaleEncoding();
    this.config = config;
    try {
      appContext = SmppGWAppContext.getInstance(config.getServletContext().getInitParameter("AppConfigFile"));
    } catch (Throwable t) {
      logger.fatal("Could not initialize application", t);
      throw new ServletException("Could not initialize application", t);
    }
  }

  public void destroy()
  {
    defaultEncoding = "ISO-8859-1";
    config = null;
    appContext = null;
  }

  public void doFilter(final ServletRequest req, final ServletResponse resp, final FilterChain chain) throws ServletException, IOException
  {
    if (logger == null) logger = Logger.getLogger(this.getClass());

    if (logger.isDebugEnabled())
      logger.debug("Request: " + req.getScheme() + " from " + req.getRemoteHost() + ':' + req.getRemotePort());

    if (config == null) {
      logger.fatal("Not initialized");
      return;
    }

    req.setCharacterEncoding(defaultEncoding);
    req.setAttribute("appContext", appContext);
    if (req instanceof HttpServletRequest) {
      final HttpServletRequest httpServletRequest = (HttpServletRequest) req;
      req.setAttribute("requestURI", new String(httpServletRequest.getRequestURI()));
    }
    chain.doFilter(req, resp);
  }

}
