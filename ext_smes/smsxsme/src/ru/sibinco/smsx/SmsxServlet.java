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
      boolean testMode = System.getProperty("test.mode") != null;

      int jmxPort;

      if (System.getProperty("portJMX") == null) {
        System.out.println("portJMX = null");
        jmxPort = -1;
      } else {
        try {
          jmxPort = Integer.parseInt(System.getProperty("portJMX"));
        } catch (Throwable e) {
          e.printStackTrace();
          jmxPort = -1;
        }
      }

      sme = new Sme(getServletConfig().getInitParameter("configDir"), testMode, jmxPort);

    } catch (Throwable e) {
      e.printStackTrace();;
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
