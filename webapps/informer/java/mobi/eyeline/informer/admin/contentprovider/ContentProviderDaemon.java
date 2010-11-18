package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.Daemon;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.apache.log4j.Logger;

import java.io.File;
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
public class ContentProviderDaemon implements Daemon {
  Logger log = Logger.getLogger(this.getClass());
  private ScheduledExecutorService scheduler;
  static final String NAME = "ContentProviderDaemon";
  private static final long PERIOD_MSEC = 1000L;
  private static final long SHUTDOWN_WAIT_TIME = 2000L;
  FileSystem fileSys;
  AdminContext context;
  File informerBase;

  public ContentProviderDaemon(AdminContext context, File informerBase, FileSystem fileSys) {
    this.informerBase = informerBase;
    this.context = context;
    this.fileSys=fileSys;
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
    scheduler.scheduleAtFixedRate(
        new ContentProviderDaemonTask(context,informerBase,fileSys),
        0,
        PERIOD_MSEC,
        TimeUnit.MILLISECONDS
    );
  }

  public synchronized void stop() throws AdminException {
    scheduler.shutdown();
    try {
      scheduler.awaitTermination(SHUTDOWN_WAIT_TIME, TimeUnit.MILLISECONDS);
    }
    catch (InterruptedException e) {
      scheduler.shutdownNow();
    }
    scheduler = null;
  }

  public boolean isStarted() {
    return scheduler!=null;
  }

}
