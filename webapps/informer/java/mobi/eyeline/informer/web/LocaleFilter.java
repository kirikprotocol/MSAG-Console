package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.users.User;

import javax.servlet.*;
import javax.servlet.http.HttpServletRequest;
import java.io.IOException;
import java.security.Principal;
import java.util.Locale;

/**
 * Вычисление и установка локали
 * @author Aleksandr Khalitov
 */
public class LocaleFilter implements Filter {

  public static final String LOCALE_PARAMETER = "informer_user_locale";

  public void init(FilterConfig filterConfig) throws ServletException {
  }

  public void doFilter(ServletRequest servletRequest, ServletResponse servletResponse, FilterChain filterChain) throws IOException, ServletException {
   HttpServletRequest request = (HttpServletRequest)servletRequest;
    Locale l;
    Principal p = request.getUserPrincipal();
    User u = null;
    if(p != null) {
      u = WebContext.getInstance().getConfiguration().getUser(p.getName());
    }

    l = u != null && u.getLocale() != null ? u.getLocale() :
        request.getLocale() != null ? request.getLocale() : Locale.ENGLISH;

    request.setAttribute(LOCALE_PARAMETER, l);

    filterChain.doFilter(servletRequest, servletResponse);
  }

  public void destroy() {
  }
}
