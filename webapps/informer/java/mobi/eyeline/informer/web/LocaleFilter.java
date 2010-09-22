package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.users.User;
import org.apache.log4j.Logger;

import javax.servlet.*;
import javax.servlet.http.HttpServletRequest;
import java.io.IOException;
import java.security.Principal;
import java.util.Locale;

/**
 * author: alkhal
 */
public class LocaleFilter implements Filter {

  public static String LOCALE_PARAMETER = "informer_user_locale";

  private static final Logger logger = Logger.getLogger(LocaleFilter.class);

  public void init(FilterConfig filterConfig) throws ServletException {
  }

  public void doFilter(ServletRequest servletRequest, ServletResponse servletResponse, FilterChain filterChain) throws IOException, ServletException {
   HttpServletRequest request = (HttpServletRequest)servletRequest;
    Locale l = null;
    Principal p = request.getUserPrincipal();
    User u = null;
    if(p != null) {
      try {
        u = WebContext.getInstance().getConfiguration().getUserSettings().getUser(p.getName());
      } catch (AdminException e) {
        logger.error(e,e);
      }
    }

    l = u != null && u.getPrefs() != null && u.getPrefs().getLocale() != null ? u.getPrefs().getLocale() :
        request.getLocale() != null ? request.getLocale() : Locale.ENGLISH;

    request.setAttribute(LOCALE_PARAMETER, l);

    filterChain.doFilter(servletRequest, servletResponse);
  }

  public void destroy() {
  }
}
