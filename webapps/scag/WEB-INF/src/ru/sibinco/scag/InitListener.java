package ru.sibinco.scag;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.scag.web.security.*;
import ru.sibinco.scag.web.WebContext;

import javax.servlet.ServletContext;
import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;
import java.io.File;
import java.io.FileReader;
import java.io.InputStream;

public class InitListener implements ServletContextListener {

  private static final Logger log = Logger.getLogger(InitListener.class);

  @Override
  public void contextInitialized(ServletContextEvent servletContextEvent) {
    try{
      String configDir = System.getProperty("msag.console.config.dir");
      if (configDir == null){
        String message = "Couldn't find system property 'msag.console.config.dir'.";
        log.error(message);
        throw new IllegalArgumentException(message);
      }

      if (configDir.isEmpty()){
        String message = "System property 'msag.console.config.dir' is empty.";
        log.error(message);
        throw new IllegalArgumentException(message);
      }

      String configFileString = configDir+File.separator+"webconfig.xml";

      File configFile = new File(configFileString);
      boolean exists = configFile.exists();
      if (!exists){
        String message = "MSAG Console configuration file '"+configFileString+"' doesn't exists.";
        log.error(message);
        throw new IllegalArgumentException(message);
      }

      log.info("Use MSAG Console configuration file: "+configFileString);
      Config config = new Config(configFile);

      String usersFileName = config.getString("users_config_file");
      if (log.isInfoEnabled()) log.debug("use configuration parameter 'users_config_file': "+usersFileName);
      File usersFile = new File(usersFileName);
      if (log.isInfoEnabled()) log.info("Try to initialize file '" + usersFile.getAbsolutePath() + "' ...");
      Document usersXmlDocument = XMLDocumentParser.parse(new FileReader(usersFile));
      Authenticator authenticator = new XmlAuthenticator(usersXmlDocument);

      ServletContext sc = servletContextEvent.getServletContext();
      InputStream is = sc.getResourceAsStream("WEB-INF/web.xml");
      if (log.isDebugEnabled()) log.debug("Try to parse web.xml from input stream ...");
      Document webXmlDocument = XMLDocumentParser.parse(is);
      if (log.isDebugEnabled()) log.debug("Try to initialize xml role mapper ...");
      RoleMapper roleMapper = new XMLRoleMapper(usersXmlDocument, webXmlDocument);

      WebContext.init(authenticator, roleMapper);

    } catch (Exception e) {
      log.error("Couldn't initialize servlet context: "+e.getMessage(), e);
      throw new RuntimeException(e);
    }
  }

  @Override
  public void contextDestroyed(ServletContextEvent servletContextEvent) {

  }
}
