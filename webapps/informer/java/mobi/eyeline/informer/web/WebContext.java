package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.auth.Authenticator;
import mobi.eyeline.informer.web.auth.impl.AuthenticatorImpl;
import mobi.eyeline.informer.web.auth.impl.Users;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.config.InformerTimezones;
import mobi.eyeline.informer.web.config.TimezonesConfig;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.concurrent.CountDownLatch;

/**
 * Класс для управления конфигурациями, авторизацией, демонами и др.
 *
 * @author Aleksandr Khalitov
 */
public class WebContext {

  private static final Logger logger = Logger.getLogger(WebContext.class);

  private static WebContext instance;

  private Authenticator authenticator;

  private AdminContext adminContext;

  private static final CountDownLatch initLatch = new CountDownLatch(1);

  private Configuration configuration;

  private InformerTimezones webTimezones;

  public static void init(File baseDir) throws InitException {
    if (instance == null) {

      if (Mode.testMode) {
        if (logger.isInfoEnabled()) {
          logger.info(" -- TEST MODE -- TEST MODE -- TEST MODE -- TEST MODE -- ");
        }
        try {
          instance = (WebContext) Class.forName("mobi.eyeline.informer.web.TestWebContext").getConstructor(File.class).newInstance(baseDir);
        } catch (Exception e) {
          throw new InitException(e);
        }
      } else {
        instance = new WebContext(baseDir);
      }

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

  protected void init(File baseDir, final AdminContext adminContext) throws InitException {
    this.adminContext = adminContext;

    try {
      this.authenticator = new AuthenticatorImpl(new Users() {
        public User getUser(String login) {
          try {
            return adminContext.getUser(login);
          } catch (Exception e) {
            logger.error(e, e);
            return null;
          }
        }
      });
      configuration = new Configuration(adminContext);

      webTimezones = new TimezonesConfig(new File(adminContext.getInformerConfDir(), "timezones.xml")).getTimezones();

    } catch (InitException e) {
      throw e;
    } catch (Exception e) {
      throw new InitException(e);
    }
  }

  protected WebContext() throws InitException {
  }

  private WebContext(File baseDir) throws InitException {
    init(baseDir, new AdminContext(baseDir));
  }

  public InformerTimezones getWebTimezones() {
    return webTimezones;
  }

  public Authenticator getAuthenticator() {
    return authenticator;
  }

  public Configuration getConfiguration() {
    return configuration;
  }

  void shutdown() {
    if (adminContext != null) {
      adminContext.shutdown();
    }
  }

}
