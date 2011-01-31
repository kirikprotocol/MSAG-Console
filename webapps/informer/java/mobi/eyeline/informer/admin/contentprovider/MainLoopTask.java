package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.FileUtils;
import mobi.eyeline.informer.util.Time;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.*;
import java.util.regex.Pattern;


/**
 *
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 17.11.2010
 * Time: 18:29:48
 */
class MainLoopTask implements Runnable {

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private Map<String, Long> lastUpdate = new HashMap<String, Long>(100);

  private ContentProviderContext context;
  private FileSystem fileSys;
  private File workDir;


  private Pattern unfinishedFileName = Pattern.compile("\\.csv\\.\\d+$");
  private UserDirResolver userDirResolver;

  public MainLoopTask(ContentProviderContext context, UserDirResolver userDirResolver, File workDir) throws AdminException {
    this.context = context;
    this.fileSys=context.getFileSystem();
    this.workDir=workDir;
    this.userDirResolver=userDirResolver;
  }


  public void run() {
    try {
      List<User> users = context.getUsers();
      Set<String> checkedUcps = new HashSet<String>();
      for(User u : users ) {
        List<UserCPsettings> s = u.getCpSettings();
        if(u.getStatus()==User.Status.ENABLED && s != null && s.size()>0) {
          try {
            processUser(u, checkedUcps);
          }
          catch (Exception e) {
            log.error("Error processing ",e);
          }
        }
      }
      Iterator<String> usageIds = lastUpdate.keySet().iterator();
      while(usageIds.hasNext()) {
        String n = usageIds.next();
        if(!checkedUcps.contains(n)) {
          usageIds.remove();
        }
      }
    }
    catch (Exception e) {
      log.error(e,e);
    }
  }

  private boolean isUpdateIsNeeded(UserCPsettings ucps, String mapKey, long currentMillis) {

    Time now = new Time(currentMillis);

    if(!now.isInInterval(ucps.getActivePeriodStart(), ucps.getActivePeriodEnd())) {
      if (log.isDebugEnabled())
        log.debug("Active period doesn't become: '" + ucps); //todo remove
      return false;
    }

    Long lastUs = lastUpdate.get(mapKey);
    if(lastUs != null && lastUs + (60L*ucps.getPeriodInMin()*1000) > currentMillis) {
      if (log.isDebugEnabled())
        log.debug("Timeout for updating: '" + ucps);         //todo remove
      return false;
    }

    return true;
  }

  //clean up unused dirs for this user
  private void cleanUpDirs(String login, List<String> userDirsNames) {
    String[] allDirs = fileSys.list(workDir);
    if(allDirs!=null) {
      for(String dirName : allDirs) {
        if(dirName.startsWith(login+"_")) {
          if(!userDirsNames.contains(dirName)) {
            FileUtils.recursiveDeleteFolder(new File(workDir,dirName));
          }
        }
      }
    }
  }

  ResourceProcessStrategy getStrategy(User u, File workDir, UserCPsettings ucps, FileResource resource) throws AdminException {
    ResourceOptions opts = new ResourceOptions(u, workDir, ucps);
    return ucps.getWorkType() == UserCPsettings.WorkType.simple
            ? new SimpleResourceProcessStrategy(context, resource, opts) : new DetailedResourceProcessStrategy(context, resource, opts);
  }

  private void processUser(User u, Collection<String> checkedUcps) throws AdminException {
    List<String> userDirsNames = new ArrayList<String>();
    if(u.getCpSettings()!=null) {
      for(UserCPsettings ucps : u.getCpSettings()) {

        String mapKey = ucps.toString();

        checkedUcps.add(mapKey);

        long currentMillis = System.currentTimeMillis();

        if (log.isDebugEnabled())
          log.debug("Start: '" + ucps + "'. User: '" + u.getLogin() + "'...");

        File userDir = userDirResolver.getUserLocalDir(u.getLogin(), ucps);

        if(!fileSys.exists(userDir)) {
          fileSys.mkdirs(userDir);
        }
        userDirsNames.add(userDir.getName());

        ResourceProcessStrategy strategy = getStrategy(u, userDir, ucps, userDirResolver.getConnection(u, ucps));

        if(!isUpdateIsNeeded(ucps, mapKey, currentMillis)) {
          try {
            strategy.process(false);
          } catch (AdminException e) {
            log.error(e,e);
          }
        } else {
          try {
            strategy.process(true);
          } catch (AdminException e) {
            log.error(e,e);
          }
          lastUpdate.put(mapKey, currentMillis);
        }

        if (log.isDebugEnabled())
          log.debug("Finish: '" + ucps + "'. User: '" + u.getLogin() + "'...");
      }
    }
    cleanUpDirs(u.getLogin(), userDirsNames);
  }

}
