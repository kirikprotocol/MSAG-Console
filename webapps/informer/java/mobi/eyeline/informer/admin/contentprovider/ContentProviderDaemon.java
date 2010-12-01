package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.Daemon;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListenerStub;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import org.apache.log4j.Logger;

import java.io.*;
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
public class ContentProviderDaemon extends DeliveryChangeListenerStub implements Daemon, ContentProviderUserDirResolver {

  Logger log = Logger.getLogger(this.getClass());

  private ScheduledExecutorService scheduler;
  private ScheduledExecutorService reportScheduler;
  static final String NAME = "ContentProviderDaemon";
  static final String NAME_REPORT = "ContentProviderDaemonReports";
  private static final long PERIOD_MSEC = 1000L;
  private static final long SHUTDOWN_WAIT_TIME = 2000L;
  FileSystem fileSys;
  ContentProviderContext context;
  File informerBase;
  File workDir;


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
    return NAME;  //To change body of implemented methods use File | Settings | File Templates.
  }

  public synchronized void start() throws AdminException {

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

    scheduler.scheduleAtFixedRate(new ContentProviderImportTask(context,this,workDir),0,PERIOD_MSEC,TimeUnit.MILLISECONDS);

    reportScheduler.schedule(new ContentProviderReportTask(context,this,workDir),0,TimeUnit.MILLISECONDS);

  }

  public synchronized void stop() throws AdminException {
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
  }

  public boolean isStarted() {
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
          reportScheduler.schedule(new ContentProviderReportTask(context,this,workDir),0,TimeUnit.MILLISECONDS);
        }
      }
    }
    catch (Exception ex) {
      log.error("Error processing delivery finished report for delivery "+e.getDeliveryId(),ex);
    }
    finally {
      if(ps!=null) try {ps.close();} catch (Exception ex){}
    }
  }

  public File getUserLocalDir(String login, UserCPsettings ucps) throws AdminException {
    File dir = new File(workDir,login+"_"+ucps.getHashId());
    return dir;
  }

  public ContentProviderConnection getConnection(User user, UserCPsettings ucps) throws AdminException {
    if(ucps.getProtocol()==UserCPsettings.Protocol.sftp) {
      return new ContentProviderConnectionSFTP(fileSys,ucps);
    }
    else {
      return new ContentProviderConnectionLocalFilesys(informerBase,fileSys,ucps);
    }
  }

  public void verifyConnection(User u, UserCPsettings ucps) throws AdminException {
    ContentProviderConnection con = null;
    try {
      con = getConnection(u,ucps);
      con.connect();
      con.listCSVFiles();
      con.close();
    }
    finally {
      try {if(con!=null) con.close();} catch (Exception e){}
    }
  }
}