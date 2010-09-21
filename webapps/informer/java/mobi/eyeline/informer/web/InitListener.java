package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.util.xml.WebXml;
import mobi.eyeline.informer.web.auth.Authenticator;
import mobi.eyeline.informer.web.auth.AuthenticatorImpl;
import org.apache.log4j.Logger;

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

      File webconfig = new File(System.getProperty("informer.config.webconfig"));
      File appBaseDir = new File(System.getProperty("informer.base.dir"));

      if(Mode.testMode) {
        adminContext = (AdminContext)Class.forName("mobi.eyeline.informer.admin.TestAdminContext").
            getConstructor(File.class, File.class).newInstance(appBaseDir, webconfig);
      }else {
        adminContext = new AdminContext(appBaseDir, webconfig);
      }

      Authenticator authenticator = new AuthenticatorImpl(adminContext.getUsersManager());

      WebContext.init(authenticator, webXml, adminContext);

      servletContextEvent.getServletContext().setAttribute("informer-version", readVersion(servletContextEvent.getServletContext().getRealPath("META-INF")));
      
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
