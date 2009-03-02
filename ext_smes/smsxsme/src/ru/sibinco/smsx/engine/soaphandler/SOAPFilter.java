package ru.sibinco.smsx.engine.soaphandler;

import javax.servlet.*;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletRequestWrapper;
import java.io.IOException;

/**
 * User: artem
 * Date: 07.12.2007
 */

public class SOAPFilter implements Filter {
  public void init(FilterConfig filterConfig) throws ServletException {
  }

  public void doFilter(ServletRequest servletRequest, ServletResponse servletResponse, FilterChain filterChain) throws IOException, ServletException {
    filterChain.doFilter(new MyServletRequest((HttpServletRequest)servletRequest), servletResponse);
  }

  public void destroy() {
  }

  public static class MyServletRequest extends HttpServletRequestWrapper {

    public MyServletRequest(HttpServletRequest httpServletRequest) {
      super(httpServletRequest);
    }

    public String getContentType() {
      return super.getContentType() + " action  ";
    }

    public String getHeader(String name) {
      String header = super.getHeader(name);
      if (header == null && name.equalsIgnoreCase("SOAPAction"))
        header = "";
      return header;
    }
  }
}
