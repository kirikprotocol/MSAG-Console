package ru.novosoft.smsc.jsp;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.DocumentBuilderFactory;

import ru.novosoft.smsc.AppContext;

public class SMSCJspController extends HttpServlet
{
  AppContext appContext = null;

  public void service(HttpServletRequest request, HttpServletResponse response)
  {
    try {
      long tm = System.currentTimeMillis();
      String s = request.getPathInfo();
      if (s == null || s.length() <= 1) {
        s = "/index.jsp";
      } else {
        s = "/" + s.substring(1);
      }
      System.out.println("Forwarding to: " + s);

      request.setAttribute("appContext", appContext);
      request.setAttribute("requestURI", new String(request.getRequestURI()));


      long tm1 = System.currentTimeMillis();

      getServletContext().getRequestDispatcher(s).forward(request, response);
      tm1 = System.currentTimeMillis() - tm1;
      tm = System.currentTimeMillis() - tm;
      System.out.println("End Forwarding to: " + s + " time spent=" + tm + " jsp time spent=" + tm1 + " redir=" + (tm - tm1));
    } catch (Exception ex) {
      System.out.println("JSP exception");
      ex.printStackTrace();
    }
  }

  public void init() throws ServletException
  {
    try {
      System.out.println("Initing smsc jsp controller");
      appContext = new AppContextImpl(getInitParameter("config"));
    } catch (Exception ex) {
      ex.printStackTrace();
      throw new ServletException("Exception occured during initialization: " + ex.getMessage());
    }
  }
}

