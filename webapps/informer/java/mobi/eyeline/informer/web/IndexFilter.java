package mobi.eyeline.informer.web;

import javax.servlet.*;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;

/**
 * @author Aleksandr Khalitov
 */
public class IndexFilter implements Filter{


  public void init(FilterConfig filterConfig) throws ServletException {

  }

  public void doFilter(ServletRequest servletRequest, ServletResponse servletResponse, FilterChain filterChain) throws IOException, ServletException {
    if(!((HttpServletRequest)servletRequest).isUserInRole("informer-admin")){
      ((HttpServletResponse)servletResponse).sendRedirect(((HttpServletRequest) servletRequest).getContextPath()+"/deliveries/index.faces");
      return;
    }
    filterChain.doFilter(servletRequest, servletResponse);
  }

  public void destroy() {

  }
}
