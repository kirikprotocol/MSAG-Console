package ru.sibinco.smsx;

import org.apache.axis.transport.http.AxisServlet;

import javax.servlet.ServletException;

/**
 * User: artem
 * Date: 04.07.2007
 */

public class SmsxServlet extends AxisServlet {

  private Sme sme;

  public void init() throws ServletException {
    super.init();
    try {
      sme = new Sme(getServletConfig().getInitParameter("configDir"), System.getProperty("test.mode") != null);
    } catch (Throwable e) {
      throw new ServletException(e);
    }
  }

  public void destroy() {
    super.destroy();
    try {
      System.out.println("Stopping sme...");
      sme.release();
      System.out.println("Sme has been stopped");
    } catch (SmeException e) {
      e.printStackTrace();
    }
  }
}
