package mobi.eyeline.informer.web;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.xml.WebXml;
import mobi.eyeline.informer.web.auth.Authenticator;
import mobi.eyeline.informer.web.auth.AuthenticatorImpl;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.journal.Journal;
import mobi.eyeline.informer.web.journal.JournalFileDataSource;
import mobi.eyeline.informer.web.users.UsersManager;

import java.io.File;
import java.util.concurrent.CountDownLatch;

/**
 * author: alkhal
 */
public class WebContext {

  private static WebContext instance;

  protected WebXml webXml;

  protected Authenticator authenticator;

  protected AdminContext adminContext;

  private static final CountDownLatch initLatch = new CountDownLatch(1);

  protected Journal journal;

  protected UsersManager usersManager;

  protected WebConfig webConfig;

  protected Configuration configuration;

  public static void init(WebXml webXml, WebConfig config, File baseDir) throws InitException {
    if (instance == null) {
      if(Mode.testMode) {
        instance = new WebContext();
      }else {
        System.out.println(" -- TEST MODE -- TEST MODE -- TEST MODE -- TEST MODE -- ");
        try{
          instance = (WebContext)Class.forName("mobi.eyeline.informer.web.TestWebContext").getConstructor().newInstance();
        }catch (Exception e) {
          throw new InitException(e);
        }
      }
      instance.webXml = webXml;
      instance.webConfig = config;
      instance.prepare(baseDir);
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

  protected void prepare(File baseDir) throws InitException {
    try{
      this.adminContext = new AdminContext(baseDir, webConfig.getInstallationType(), webConfig.getAppMirrorDirs());
      File usersFile = new File(webConfig.getUsersFile());
      usersManager = new UsersManager(usersFile, new File(usersFile.getParentFile(), "backup"), adminContext.getFileSystem());
      this.authenticator = new AuthenticatorImpl(usersManager);
      journal = new Journal(new JournalFileDataSource(new File(webConfig.getJournalDir()), adminContext.getFileSystem()));       //todo file system
      configuration = new Configuration(journal, adminContext, usersManager);
    }catch (AdminException e) {
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

  public Journal getJournal() {
    return journal;
  }
}
