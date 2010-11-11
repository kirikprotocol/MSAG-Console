package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.WebConfigManager;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.xml.WebXml;
import mobi.eyeline.informer.web.auth.Authenticator;
import mobi.eyeline.informer.web.auth.impl.AuthenticatorImpl;
import mobi.eyeline.informer.web.auth.impl.Users;
import mobi.eyeline.informer.web.config.Configuration;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.concurrent.CountDownLatch;

/**
 * Класс для управления конфигурациями, авторизацией, демонами и др.
 * @author Aleksandr Khalitov
 */
public class WebContext {

  private static final Logger logger = Logger.getLogger(WebContext.class);

  private static WebContext instance;

  protected WebXml webXml;

  protected Authenticator authenticator;

  protected AdminContext adminContext;

  private static final CountDownLatch initLatch = new CountDownLatch(1);

  protected Configuration configuration;

  public static void init(WebXml webXml, File config, File baseDir) throws InitException {
    if (instance == null) {
      instance = new WebContext(webXml, config, baseDir);
      initLatch.countDown();
    }
  }

  public static WebContext getInstance() {
    try {
      initLatch.await();
      return instance;
    } catch (InterruptedException e) {
      return null;
    }
  }

  private WebContext(WebXml webXml, File webconfigFile, File baseDir) throws InitException {
    this.webXml = webXml;
    try{
      File confDir = new File(webconfigFile.getParent());
      WebConfigManager webConfigManager = new  WebConfigManager(webconfigFile ,new File(confDir, "backup"), FileSystem.getFSForSingleInst()) ;

      if(Mode.testMode) {
        if(logger.isInfoEnabled()) {
          logger.info(" -- TEST MODE -- TEST MODE -- TEST MODE -- TEST MODE -- ");
        }
        this.adminContext = (AdminContext)Class.forName("mobi.eyeline.informer.admin.TestAdminContext").
            getConstructor(File.class, WebConfigManager.class).newInstance(baseDir, webConfigManager);
      }else {
        this.adminContext = new AdminContext(baseDir, webConfigManager);
      }

      this.authenticator = new AuthenticatorImpl(new Users() {
        public User getUser(String login) {
          try{
            return adminContext.getUser(login);
          }catch (Exception e){
            logger.error(e,e);
            return null;
          }
        }
      });
      configuration = new Configuration(adminContext);
    }catch (InitException e) {
      throw e;
    }catch (Exception e) {
      throw new InitException(e);
    }
  }

  public Authenticator getAuthenticator() {
    return authenticator;
  }

  public WebXml getWebXml() {
    return webXml;
  }

  public Configuration getConfiguration() {
    return configuration;
  }

  void shutdown() {
    if(adminContext != null) {
      adminContext.shutdown();
    }
  }

}
