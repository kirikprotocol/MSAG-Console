package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.monitoring.MBean;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;
import java.io.File;

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

      MBean.getInstance(MBean.Source.SYSTEM).notifyStartup(Version.version == null ? "" : Version.version);

      File appBaseDir = new File(System.getProperty("informer.base.dir"));
      initLog4j(new File(new File(appBaseDir, "conf"), "log4j.properties").getAbsolutePath());

      if (System.getProperty("java.security.auth.login.config") == null) {
        System.setProperty("java.security.auth.login.config",
            servletContextEvent.getServletContext().getRealPath("WEB-INF/jaas.config"));
      }

      WebContext.init(appBaseDir);

      context = WebContext.getInstance();


      MBean.getInstance(MBean.Source.SYSTEM).notifyConfigurationOk();
    } catch (Throwable e) {
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

}
