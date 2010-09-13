package ru.novosoft.smsc.web;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.smsc.web.auth.Authenticator;
import ru.novosoft.smsc.web.auth.AuthenticatorImpl;

import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;
import java.io.File;

/**
 * author: alkhal
 */
public class InitListener implements ServletContextListener {

  private static final Logger logger = Logger.getLogger(InitListener.class);

  public void contextInitialized(ServletContextEvent servletContextEvent) {
    try {
      System.setProperty("java.security.auth.login.config",
          servletContextEvent.getServletContext().getRealPath("WEB-INF/jaas.config"));

      WebXml webXml = new WebXml(new File(servletContextEvent.getServletContext().getRealPath("WEB-INF/web.xml")));

      AdminContext adminContext;

      File webconfig = new File(System.getProperty("smsc.config.webconfig"));
      File appBaseDir = new File(System.getProperty("smsc.base.dir"));

      if(Mode.testMode) {
        adminContext = (AdminContext)Class.forName("ru.novosoft.smsc.admin.TestAdminContext").
            getConstructor(File.class, File.class).newInstance(appBaseDir, webconfig);
      }else {
        adminContext = new AdminContext(appBaseDir, webconfig);
      }

      Authenticator authenticator = new AuthenticatorImpl(adminContext.getUsersManager());

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
