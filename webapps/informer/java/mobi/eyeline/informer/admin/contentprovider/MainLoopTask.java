package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.monitoring.MBean;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Time;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.*;


class MainLoopTask implements Runnable {

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private Map<String, Long> lastUpdate = new HashMap<String, Long>(100);

  private ContentProviderContext context;
  private FileSystem fileSys;

  private static final String USER_PROC_ERR = "User processing error";
  private static final String RESOURCE_PROC_ERR = "Resource processing error";
  private static final String UNKNOWN_ERR = "Unknown error";


  private FileDeliveriesHelper fileDeliveriesHelper;

  private final int groupNumber;
  private final int maxTimeSec;

  public MainLoopTask(ContentProviderContext context, FileDeliveriesHelper fileDeliveriesHelper, int groupNumber, int maxTimeSec) throws AdminException {
    this.context = context;
    this.fileSys=context.getFileSystem();
    this.fileDeliveriesHelper = fileDeliveriesHelper;
    this.groupNumber = groupNumber;
    this.maxTimeSec = maxTimeSec;
  }

  private boolean isCreationAvailable(User u){
    if(u.getStatus() == User.Status.DISABLED) {
      if(log.isDebugEnabled()) {
        log.debug("User is blocked: "+u.getLogin());
      }
      return false;
    }
    if(log.isDebugEnabled()) {
      log.debug("Check restrictions for user: "+u.getLogin());
    }
    try{
      context.checkNoRestrictions(u.getLogin());
    }catch (AdminException e){
      if(log.isDebugEnabled()) {
        log.debug(e.getLocalizedMessage());
      }
      return false;
    }
    return true;
  }

  private static MBean getMBean() {
    return MBean.getInstance(MBean.Source.CONTENT_PROVIDER);
  }

  public void run() {
    try {
      List<User> users = context.getUsers();
      Set<String> checkedUcps = new HashSet<String>();
      Collection<String> usersDirs = new HashSet<String>();
      for(User u : users ) {
        if(!fileDeliveriesHelper.isUserFromGroup(u.getLogin(), groupNumber)) {
          continue;
        }
        if(log.isDebugEnabled()) {
          log.debug("Process user="+u.getLogin()+" group="+groupNumber);
        }
        List<UserCPsettings> s = u.getCpSettings();
        if(s != null && s.size()>0) {
          try {
            processUser(u, checkedUcps, usersDirs);
          }
          catch (Exception e) {
            getMBean().notifyInternalError(USER_PROC_ERR, e.getMessage());
            log.error("Error processing ",e);
          }
        }
      }

      fileDeliveriesHelper.cleanUnusedDirs(groupNumber, usersDirs);

      Iterator<String> usageIds = lastUpdate.keySet().iterator();
      while(usageIds.hasNext()) {
        String n = usageIds.next();
        if(!checkedUcps.contains(n)) {
          usageIds.remove();
        }
      }
    }
    catch (Exception e) {
      getMBean().notifyInternalError(UNKNOWN_ERR, e.getMessage());
      log.error(e,e);
    }
  }

  private boolean isUpdateIsNeeded(UserCPsettings ucps, String mapKey, long currentMillis) {

    Time now = new Time(currentMillis);

    if(!now.isInInterval(ucps.getActivePeriodStart(), ucps.getActivePeriodEnd())) {
      return false;
    }

    Long lastUs = lastUpdate.get(mapKey);
    return !(lastUs != null && lastUs + (60L * ucps.getPeriodInMin() * 1000) > currentMillis);

  }

  ResourceProcessStrategy getStrategy(User u, File workDir, UserCPsettings ucps, FileResource resource) throws AdminException {
    ResourceOptions opts = new ResourceOptions(u, workDir, ucps, maxTimeSec);
    switch (ucps.getWorkType()) {
      case simple: return new SimpleSaveStrategy(context, resource, opts);
      case detailed: return new DetailedSaveStrategy(context, resource, opts);
      default: throw new IllegalArgumentException("Work type is unknown: "+ucps.getWorkType());
    }
  }

  private void processUser(User u, Collection<String> checkedUcps, Collection<String> userDirs) throws AdminException {

    if(u.getCpSettings()!=null) {
      for(UserCPsettings ucps : u.getCpSettings()) {
        try{

          String mapKey = ucps.toString();
          checkedUcps.add(mapKey);

          long currentMillis = System.currentTimeMillis();

          if (log.isDebugEnabled())
            log.debug("Start: '" + ucps + "'. User: '" + u.getLogin() + "'...");

          File userDir = fileDeliveriesHelper.getUserLocalDir(u.getLogin(), ucps);

          if(!fileSys.exists(userDir)) {
            fileSys.mkdirs(userDir);
          }
          userDirs.add(userDir.getName());

          ResourceProcessStrategy strategy = getStrategy(u, userDir, ucps, fileDeliveriesHelper.getConnection(ucps));

          if(!isUpdateIsNeeded(ucps, mapKey, currentMillis) || !isCreationAvailable(u)) {
            try {
              strategy.process(false);
            } catch (AdminException e) {
              log.error(e,e);
              getMBean().notifyInternalError(RESOURCE_PROC_ERR+": "+ucps,  e.getMessage());
            }
          } else {
            try {
              strategy.process(true);
            } catch (AdminException e) {
              log.error(e,e);
              getMBean().notifyInternalError(RESOURCE_PROC_ERR+": "+ucps, e.getMessage());
            }
            lastUpdate.put(mapKey, currentMillis);
          }

          if (log.isDebugEnabled())
            log.debug("Finish: '" + ucps + "'. User: '" + u.getLogin() + "'...");

        }catch (Exception e) {
          log.error(e, e);
          getMBean().notifyInternalError(RESOURCE_PROC_ERR+": "+ucps,  e.getMessage());
        }
      }
    }
  }

}
