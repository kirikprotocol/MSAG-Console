package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.monitoring.MBean;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import javax.servlet.ServletContext;
import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.jar.Attributes;
import java.util.jar.Manifest;

/**
 * Инициализация web-приложения
 *
 * @author Aleksandr Khalitov
 */
public class InitListener implements ServletContextListener {

  private static final Logger logger = Logger.getLogger(InitListener.class);

  private WebContext context;

  private void initLog4j(String file) {
    PropertyConfigurator.configureAndWatch(file, 60000L);
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  public void contextInitialized(ServletContextEvent servletContextEvent) {
    try {

      String version = readVersion(servletContextEvent.getServletContext());

      MBean.getInstance(MBean.Source.SYSTEM).notifyStartup(version == null ? "" : version);

      File appBaseDir = new File(System.getProperty("informer.base.dir"));
      initLog4j(new File(new File(appBaseDir, "conf"), "log4j.properties").getAbsolutePath());

      if (System.getProperty("java.security.auth.login.config") == null) {
        System.setProperty("java.security.auth.login.config",
            servletContextEvent.getServletContext().getRealPath("WEB-INF/jaas.config"));
      }

      WebContext.init(appBaseDir);

      context = WebContext.getInstance();

      servletContextEvent.getServletContext().setAttribute("informer-version",
          version);


      MBean.getInstance(MBean.Source.SYSTEM).notifyConfigurationOk();
    } catch (Exception e) {
      e.printStackTrace();
      logger.error(e, e);
      throw new RuntimeException(e);
    }
  }

  public void contextDestroyed(ServletContextEvent servletContextEvent) {
    if (context != null) {
      try{
        context.shutdown();
      }catch (Exception e){
        logger.error(e,e);
      }
    }
    MBean.getInstance(MBean.Source.SYSTEM).notifyShutdown();
    MBean.shutdown();
  }

  @SuppressWarnings({"EmptyCatchBlock"})
  private static String readVersion(ServletContext context) {
    Manifest mf = new Manifest();
    InputStream is = null;
    try {
      is = context.getResourceAsStream("META-INF/MANIFEST.MF");
      if (is == null) {
        is = context.getResourceAsStream("META-INF/manifest.mf");
      }
      if (is == null) {
        return null;
      }
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
