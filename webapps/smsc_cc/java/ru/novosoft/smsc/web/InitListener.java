package ru.novosoft.smsc.web;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.smsc.web.auth.Authenticator;
import ru.novosoft.smsc.web.auth.XmlAuthenticator;

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
      System.out.println(servletContextEvent.getServletContext().getRealPath("WEB-INF/jaas.config"));
      System.setProperty("java.security.auth.login.config",
          servletContextEvent.getServletContext().getRealPath("WEB-INF/jaas.config"));

      String webConfig = System.getProperty("smsc.config.web-config");
      String usersXml = System.getProperty("smsc.config.users");


      Authenticator authenticator = new XmlAuthenticator(new File(usersXml));
      WebXml webXml = new WebXml(new File(servletContextEvent.getServletContext().getRealPath("WEB-INF/web.xml")));

      WebContext.init(authenticator, webXml, null);

    } catch (Exception e) {
      logger.error(e, e);
      throw new RuntimeException(e);
    }
  }

  public void contextDestroyed(ServletContextEvent servletContextEvent) {


  }
}
