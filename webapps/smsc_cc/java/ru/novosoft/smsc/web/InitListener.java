package ru.novosoft.smsc.web;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminContext;
import ru.novosoft.smsc.util.xml.WebXml;
import ru.novosoft.smsc.web.auth.Authenticator;
import ru.novosoft.smsc.web.auth.AuthenticatorImpl;

import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.jar.Attributes;
import java.util.jar.Manifest;

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

      servletContextEvent.getServletContext().setAttribute("smsc-version", readVersion(servletContextEvent.getServletContext().getRealPath("META-INF")));

    } catch (Exception e) {
      e.printStackTrace();
      logger.error(e, e);
      throw new RuntimeException(e);
    }
  }

  public void contextDestroyed(ServletContextEvent servletContextEvent) {


  }

  @SuppressWarnings({"EmptyCatchBlock"})
  private static String readVersion(String manifestDir) {
    File f = new File(manifestDir + File.separator + "MANIFEST.MF");
    if (!f.exists()) {
      f = new File(manifestDir + File.separator + "manifest.mf");
    }
    if (!f.exists()) {
      return null;
    }
    Manifest mf = new Manifest();
    InputStream is = null;
    try {
      is = new FileInputStream(f);
      mf.read(is);
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (IOException e) {
        }
      }
    }
    Attributes a = mf.getMainAttributes();
    return a != null ? a.getValue("Implementation-Version") : null;
  }
}
