package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListenerStub;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.PrintStream;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 17.11.2010
 * Time: 16:21:41
 */
class ContentProviderDaemon extends DeliveryChangeListenerStub implements UserDirResolver {

  private static Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private ScheduledExecutorService scheduler;
  private ScheduledExecutorService reportScheduler;
  static final String NAME = "ContentProviderDaemon";
  private static final String NAME_REPORT = "ContentProviderDaemonReports";

  private static final long SHUTDOWN_WAIT_TIME = 2000L;
  private FileSystem fileSys;
  private ContentProviderContext context;
  private File informerBase;
  private File workDir;



  public ContentProviderDaemon(ContentProviderContext context, File informerBase, File workDir) throws AdminException {
    this.context = context;
    this.informerBase = informerBase;
    this.workDir = new File(workDir,"contentProvider");

    fileSys=context.getFileSystem();
    if(!fileSys.exists(this.workDir)) {
      fileSys.mkdirs(this.workDir);
    }
  }

  public String getName() {
    return NAME;
  }

  public synchronized void start() throws AdminException {
    if (log.isDebugEnabled())
      log.debug("Content Provider Daemon starting...");

    if(scheduler!=null) return;

    scheduler = Executors.newSingleThreadScheduledExecutor(new ThreadFactory(){
      public Thread newThread(Runnable runnable) {
        return new Thread(runnable,NAME);
      }
    });
    reportScheduler = Executors.newSingleThreadScheduledExecutor(new ThreadFactory(){
      public Thread newThread(Runnable runnable) {
        return new Thread(runnable,NAME_REPORT);
      }
    });

    scheduler.scheduleAtFixedRate(new MainLoopTask(context,this,workDir),0, 60, TimeUnit.SECONDS);

    reportScheduler.schedule(new ProcessNotificationsTask(context,this,workDir),0,TimeUnit.MILLISECONDS);

    if (log.isDebugEnabled())
      log.debug("Content Provider Daemon successfully started.");
  }

  public synchronized void stop() throws AdminException {
    if (log.isDebugEnabled())
      log.debug("Content Provider Daemon shutdowning...");

    scheduler.shutdown();
    reportScheduler.shutdown();
    try {
      scheduler.awaitTermination(SHUTDOWN_WAIT_TIME, TimeUnit.MILLISECONDS);
      reportScheduler.awaitTermination(SHUTDOWN_WAIT_TIME, TimeUnit.MILLISECONDS);
    }
    catch (InterruptedException e) {
      if(!scheduler.isShutdown() )scheduler.shutdownNow();
      if(!reportScheduler.isShutdown() )reportScheduler.shutdownNow();
    }
    scheduler = null;
    reportScheduler = null;

    if (log.isDebugEnabled())
      log.debug("Content Provider Daemon shutdowned.");
  }

  boolean isStarted() {
    return scheduler!=null;
  }



  @Override
  public void deliveryStateChanged(ChangeDeliveryStatusEvent e) {
    if (e.getStatus() != DeliveryStatus.Finished)
      return;

    PrintStream ps = null;
    try {
      File notificationFile = new File(workDir,e.getDeliveryId()+".notification");
      ps = new PrintStream(fileSys.getOutputStream(notificationFile,false),true,"utf-8");
      ps.println(e.getUserId());
      synchronized (this) {
        if(isStarted()) {
          reportScheduler.schedule(new ProcessNotificationsTask(context,this,workDir),0,TimeUnit.MILLISECONDS);
        }
      }
    }
    catch (Exception ex) {
      log.error("Error processing delivery finished report for delivery "+e.getDeliveryId(),ex);
    }
    finally {
      if(ps!=null) try {ps.close();} catch (Exception ignored){}
    }
  }

  public File getUserLocalDir(String login, UserCPsettings ucps) throws AdminException {
    return new File(workDir,login+"_"+ucps.getHashId());
  }

  public FileResource getConnection(User user, UserCPsettings ucps) throws AdminException {
    switch (ucps.getProtocol()) {
      case sftp: return new SFTPResource(fileSys, ucps);
      case ftp: return new FTPResource(fileSys, ucps);
      case smb: return new SMBResource(fileSys, ucps);
      case localFtp:
        File homeDir = context.getFtpUserHomeDir(ucps.getLogin());
        return homeDir != null ? new LocalResource(homeDir, fileSys) : new NullResource();
      default: return new LocalResource(informerBase, fileSys, ucps);
    }
  }

  public void verifyConnection(User u, UserCPsettings ucps) throws AdminException {
    if(ucps.getProtocol() == UserCPsettings.Protocol.localFtp) {
      return;
    }
    ucps.checkValid();

    FileResource con = null;
    try {
      con = getConnection(u,ucps);
      con.open();
      con.listCSVFiles();
    }
    finally {
      try {if(con!=null) con.close();} catch (Exception ignored){}
    }
  }
}