package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

/**
 * @author Artem Snopkov
 */
public class FileDeliveriesProvider implements UserDirResolver {

  private static Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private ScheduledExecutorService scheduler;
  static final String NAME = "ContentProviderDaemon";

  private static final long SHUTDOWN_WAIT_TIME = 2000L;
  private FileSystem fileSys;
  private ContentProviderContext context;
  private final File informerBase;
  private final File workDir;
  private final int groupSize;
  private final int maxTimeSec;

  private MainLoopTask mainLoopTask;


  public FileDeliveriesProvider(ContentProviderContext context, File informerBase, File workDir, int groupSize, int maxTimeSec) throws AdminException {
    this.context = context;
    this.informerBase = informerBase;
    this.groupSize = groupSize;
    this.maxTimeSec = maxTimeSec;
    this.workDir = new File(workDir,"contentProvider");

    fileSys=context.getFileSystem();
    if(!fileSys.exists(this.workDir)) {
      fileSys.mkdirs(this.workDir);
    }

    start();
  }

  public String getName() {
    return NAME;
  }

  private synchronized void start() throws AdminException {
    if (log.isDebugEnabled())
      log.debug("Content Provider Daemon starting...");

    if(scheduler!=null) return;

    scheduler = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable runnable) {
        return new Thread(runnable, NAME);
      }
    });

    mainLoopTask = new MainLoopTask(context, this ,workDir, groupSize, maxTimeSec);
    scheduler.scheduleAtFixedRate(mainLoopTask, 60, 60, TimeUnit.SECONDS);

    if (log.isDebugEnabled())
      log.debug("Content Provider Daemon successfully started.");
  }

  public File getUserLocalDir(String login, UserCPsettings ucps) throws AdminException {
    return new File(workDir,login+"_"+ucps.getHashId());
  }

  @Override
  public Collection<File> getAllUserLocalDirs(final String login) throws AdminException {
    File[] files = fileSys.listFiles(workDir, new FileFilter() {
      @Override
      public boolean accept(File pathname) {
        if(!fileSys.isDirectory(pathname)) {
          return false;
        }
        int i = pathname.getName().indexOf(login);
        if(i != 0) {
          return false;
        }
        if(pathname.getName().substring(login.length()).length() != 33) {
          return false;
        }
        return true;
      }
    });
    if(files == null) {
      return Collections.emptyList();
    }
    Collection<File> res = new ArrayList<File>(files.length);
    Collections.addAll(res, files);
    return res;
  }

  public FileResource getConnection(User user, UserCPsettings ucps) throws AdminException {
    String host = ucps.getHost();
    Integer port = ucps.getPort();
    String login = ucps.getLogin();
    String password = ucps.getPassword();
    String remoteDir = ucps.getDirectory();

    File localDir;
    String dir = ucps.getDirectory();
    if (dir == null)
      dir = "";
    if(dir.indexOf(File.separatorChar)==0) {
      localDir = new File(dir);
    } else {
      localDir = new File(informerBase,dir);
    }


    switch (ucps.getProtocol()) {
      case sftp: return FileResource.createSFTP(host , port, login, password, remoteDir);
      case ftp: return FileResource.createFTP(host, port, login, password, remoteDir, ucps.isPassiveMode());
      case smb: return FileResource.createSMB(host, port, login, password, remoteDir, ucps.getWorkGroup());
      case localFtp:
        File homeDir = context.getFtpUserHomeDir(ucps.getLogin());
        return homeDir != null ? FileResource.createLocal(homeDir, fileSys) : FileResource.createEmpty();
      default: return FileResource.createLocal(localDir, fileSys);
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
    scheduler = null;
    mainLoopTask.shutdown();

    if (log.isDebugEnabled())
      log.debug("Content Provider Daemon shutdowned.");
  }
}
