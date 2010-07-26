package ru.novosoft.smsc.web;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.admin.TestAdminContext;
import ru.novosoft.smsc.changelog.ChangeLog;
import ru.novosoft.smsc.changelog.ChangeLogListener;
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

      ChangeLog log = ChangeLog.getInstance(adminContext);
      log.addListener(new ChangeLogListener() {

        public void applyCalled(ChangeLog.Subject subject) {
          System.out.println("Apply called "+subject);
        }

        public void resetCalled(ChangeLog.Subject subject) {
          System.out.println("Reset called "+subject);
        }

        public void propertyChanged(ChangeLog.Subject source, String object, Class objectClass, String propertyName, Object oldValue, Object newValue) {
          Principal principal = FacesContext.getCurrentInstance().getExternalContext().getUserPrincipal();
          String user = principal == null ? null : principal.getName();

          System.out.println("User: " + user);
          System.out.println(source + " " + object + " " + propertyName + " " + oldValue + " " + newValue);
        }

        public void objectAdded(ChangeLog.Subject source, Object object) {
          //To change body of implemented methods use File | Settings | File Templates.
        }

        public void objectRemoved(ChangeLog.Subject source, Object object) {
          //To change body of implemented methods use File | Settings | File Templates.
        }
      });

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