/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 25.02.2004
 * Time: 17:56:11
 */
package ru.sibinco.scag.beans;

import org.apache.log4j.Logger;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.Constants;

import javax.servlet.*;
import javax.servlet.jsp.jstl.core.Config;
import javax.servlet.http.HttpServletRequest;
import java.io.IOException;
import java.util.ArrayList;


public class SCAGFilter implements Filter {
    private String defaultEncoding = "ISO-8859-1";
    FilterConfig config = null;
    SCAGAppContext appContext = null;
    private Logger logger;

    public void init(final FilterConfig config) throws ServletException {
        logger = Logger.getLogger(this.getClass());

        defaultEncoding = Functions.getLocaleEncoding();
        Functions.setLocaleEncoding("UTF-8");
        this.config = config;
        try {
            appContext = SCAGAppContext.getInstance(config.getServletContext().getInitParameter("AppConfigFile"));
            setdefaultLocale();
        } catch (Throwable t) {
            logger.fatal("Could not initialize application", t);
            throw new ServletException("Could not initialize application", t);
        }
    }

    public void destroy() {
        appContext.destroy();
        defaultEncoding = "ISO-8859-1";
        config.getServletContext().removeAttribute("localesList");
        config = null;
        appContext = null;
    }

    public void doFilter(final ServletRequest req, final ServletResponse resp, final FilterChain chain) throws ServletException, IOException
    {
        if (logger == null) logger = Logger.getLogger(this.getClass());
        if (config == null) {
            logger.fatal("Not initialized");
            return;
        }
        req.setCharacterEncoding(defaultEncoding);
        req.setAttribute(Constants.APP_CONTEXT, appContext);
        if (req instanceof HttpServletRequest) {
            final HttpServletRequest request = (HttpServletRequest) req;
            req.setAttribute("requestURI", request.getRequestURI());
            /*if (logger.isDebugEnabled()){
                logger.debug("Requested URL:" + request.getRequestURL());
            }*/
        } else if (logger.isDebugEnabled()){
            //logger.debug("Request: " + req.getScheme() + " from " + req.getRemoteHost() + "| "+ req.toString() );
        }
        chain.doFilter(req, resp);
    }

    private void setdefaultLocale() throws Throwable {
      String language_country = config.getServletContext().getInitParameter("javax.servlet.jsp.jstl.fmt.locale");
      ArrayList localesList;
      try {
        localesList = appContext.getLocaleManager().validate(language_country);
      } catch (Throwable t) {
        t.printStackTrace();
        throw t;
      }
      config.getServletContext().setAttribute("localesList", localesList);
      Config.set(config.getServletContext(), Config.FMT_LOCALIZATION_CONTEXT, config.getServletContext().getInitParameter("javax.servlet.jsp.jstl.fmt.localizationContext"));
      Config.set(config.getServletContext(), Config.FMT_LOCALE, language_country);
    }
}
