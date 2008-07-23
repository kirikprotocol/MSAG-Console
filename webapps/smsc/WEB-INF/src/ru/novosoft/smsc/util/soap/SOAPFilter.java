package ru.novosoft.smsc.util.soap;

import javax.servlet.*;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletRequestWrapper;
import java.io.IOException;

/**
 * User: artem
 * Date: 17.06.2008
 */

public class SOAPFilter implements Filter {
  public void init(FilterConfig filterConfig) throws ServletException {
  }

  public void doFilter(ServletRequest servletRequest, ServletResponse servletResponse, FilterChain filterChain) throws IOException, ServletException {
    filterChain.doFilter(new MyServletRequest((HttpServletRequest)servletRequest), servletResponse);
//    filterChain.doFilter(servletRequest, servletResponse);
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
      if (header == null && name.equalsIgnoreCase("SOAPAction")) {
        System.out.println("SOAPAction is null! Add empty SOAPAction");
        header = "";
      }
      return header;
    }
  }
}

