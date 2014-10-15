package ru.sibinco.scag;

import org.apache.log4j.Logger;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.scag.jaas.Authenticator;
import ru.sibinco.scag.jaas.XmlAuthenticator;
import ru.sibinco.scag.web.WebContext;
import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;
import java.io.File;

public class InitListener implements ServletContextListener {

  private static final Logger log = Logger.getLogger(InitListener.class);

  @Override
  public void contextInitialized(ServletContextEvent servletContextEvent) {
    try{
      String configFileName = servletContextEvent.getServletContext().getInitParameter("AppConfigFile");
      if (log.isInfoEnabled()) log.debug("use context init parameter 'AppConfigFile': "+configFileName);
      Config config = new Config(new File(configFileName));

      String usersFileName = config.getString("users_config_file");
      if (log.isInfoEnabled()) log.debug("use configuration parameter 'users_config_file': "+usersFileName);
      File usersFile = new File(usersFileName);

      Authenticator authenticator = new XmlAuthenticator(usersFile);

      WebContext.init(authenticator);

    } catch (Exception e) {
      log.error("Couldn't initialize servlet context: "+e.getMessage(), e);
      throw new RuntimeException(e);
    }
  }

  @Override
  public void contextDestroyed(ServletContextEvent servletContextEvent) {

  }
}
