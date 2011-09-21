package mobi.eyeline.informer.admin.contentprovider;

import com.eyeline.utils.ThreadFactoryWithCounter;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/**
 * @author Artem Snopkov
 */
public class FileDeliveriesProvider {

  private static Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private ScheduledExecutorService scheduler;
  private static final String NAME = "ContentProviderDaemon-";

  private static final long SHUTDOWN_WAIT_TIME = 2000L;
  private ContentProviderContext context;
  private final int groupSize;
  private final int maxTimeSec;

  private final FileDeliveriesHelper helper;


  public FileDeliveriesProvider(ContentProviderContext context, File informerBase, File workDir, int groupSize, int maxTimeSec) throws AdminException {
    this.context = context;
    this.groupSize = groupSize;
    this.maxTimeSec = maxTimeSec;
    File cpWorkDir = new File(workDir, "contentProvider");

    FileSystem fileSys = context.getFileSystem();
    if(!fileSys.exists(cpWorkDir)) {
      fileSys.mkdirs(cpWorkDir);
    }

    helper = new FileDeliveriesHelper(context, informerBase, cpWorkDir, groupSize, log);

    start();
  }


  private void cleanupDirs() throws AdminException {
    Collection<String> usersDirs = new HashSet<String>(10);
    for(User u : context.getUsers()) {
      List<UserCPsettings> ucpss = u.getCpSettings();
      if(u.getCpSettings() != null) {
        for(UserCPsettings s : ucpss) {
          usersDirs.add(helper.getUserLocalDir(u.getLogin(), s).getName());
        }
      }
    }
    helper.cleanUnusedDirs(null, usersDirs);

  }

  private synchronized void start() throws AdminException {
    if (log.isDebugEnabled())
      log.debug("Content Provider Daemon starting...");

    if(scheduler!=null) return;

    cleanupDirs();

    scheduler = Executors.newScheduledThreadPool(2, new ThreadFactoryWithCounter(NAME));

    MainLoopTask[] mainLoopTasks = new MainLoopTask[groupSize];
    for(int groupNumber=0; groupNumber<groupSize; groupNumber++) {
      mainLoopTasks[groupNumber] = new MainLoopTask(context,helper, groupNumber, maxTimeSec);
      scheduler.scheduleWithFixedDelay(mainLoopTasks[groupNumber], 60, 60, TimeUnit.SECONDS);
    }
    if (log.isDebugEnabled())
      log.debug("Content Provider Daemon successfully started.");
  }

  public void verifyConnection(User u, UserCPsettings ucps) throws AdminException {
    if(ucps.getProtocol() == UserCPsettings.Protocol.localFtp) {
      return;
    }
    ucps.checkValid();

    FileResource con = null;
    try {
      con = helper.getConnection(ucps);
      con.open();
      con.listFiles();
    }
    finally {
      try {if(con!=null) con.close();} catch (Exception ignored){}
    }
  }

  public void shutdown() {
    if (log.isDebugEnabled())
      log.debug("Content Provider Daemon shutdowning...");


    scheduler.shutdown();
    try {
      scheduler.awaitTermination(SHUTDOWN_WAIT_TIME, TimeUnit.MILLISECONDS);
    }
    catch (Exception e) {
      try{
        if(!scheduler.isShutdown() )scheduler.shutdownNow();
      }catch (Exception ignored) {}
    }

    if (log.isDebugEnabled())
      log.debug("Content Provider Daemon shutdowned.");
  }
}
