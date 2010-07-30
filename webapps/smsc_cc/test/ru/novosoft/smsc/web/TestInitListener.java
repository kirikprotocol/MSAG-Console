package ru.novosoft.smsc.web;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.smsc.web.auth.Authenticator;
import ru.novosoft.smsc.web.auth.XmlAuthenticator;

import javax.faces.context.FacesContext;
import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;
import java.io.File;
import java.io.InputStream;
import java.security.Principal;

/**
 * author: alkhal
 */
public class TestInitListener implements ServletContextListener {

  private static final Logger logger = Logger.getLogger(TestInitListener.class);

  //jvm parameters
  //-Dsmsc.config.users=/home/alkhal/projects/smsc/webapps/smsc_cc/test/users.xml -Dsmsc.config.webconfig=/home/alkhal/projects/smsc/webapps/smsc_cc/test/webconfig.xml
  
  public void contextInitialized(ServletContextEvent servletContextEvent) {
    try {
      System.out.println(servletContextEvent.getServletContext().getRealPath("WEB-INF/jaas.config"));
      System.out.println(servletContextEvent.getServletContext().getRealPath("WEB-INF/jaas.config"));
      System.setProperty("java.security.auth.login.config",
          servletContextEvent.getServletContext().getRealPath("WEB-INF/jaas.config"));

      Authenticator authenticator;
      InputStream is = null;
      try {
        is = this.getClass().getClassLoader().getResourceAsStream("users.xml");
        authenticator = new XmlAuthenticator(is);
      } finally {
        if (is != null) {
          try {
            is.close();
          } catch (Exception e) {
          }
        }
      }

      WebXml webXml = new WebXml(new File(servletContextEvent.getServletContext().getRealPath("WEB-INF/web.xml")));

      System.out.println(servletContextEvent.getServletContext().getRealPath("./"));
      AdminContext adminContext = new TestAdminContext(new File(servletContextEvent.getServletContext().getRealPath("./")), new File(System.getProperty("smsc.config.webconfig")));

      WebContext.init(authenticator, webXml, adminContext);
    } catch (Exception e) {
      e.printStackTrace();
      logger.error(e, e);
      throw new RuntimeException(e);
    }
  }

  public void contextDestroyed(ServletContextEvent servletContextEvent) {


  }
}