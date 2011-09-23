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

  private void cleanWorkDir(Collection<String> usedDirs) throws AdminException {
    File[] allDirs = fileSys.listFiles(fileDeliveriesHelper.getWorkDir());
    if (allDirs == null)
      return;

    for(File dir : allDirs) {
      String dirName = dir.getName();
      String userName = fileDeliveriesHelper.getUserByDir(dirName);
      if(userName == null || !fileDeliveriesHelper.isUserFromGroup(userName, groupNumber))
        continue;

      if (!usedDirs.contains(dirName)) {
        if(log.isDebugEnabled())
          log.debug("Unused dir: "+dirName+" will be removed, user="+userName+" group="+groupNumber);
        fileSys.recursiveDeleteFolder(dir);
      }
    }
  }

  private void cleanLastUpdateTimers(Collection<String> usedDirs) {
    Iterator<String> usageIds = lastUpdate.keySet().iterator();
    while(usageIds.hasNext()) {
      String n = usageIds.next();
      if(!usedDirs.contains(n)) {
        usageIds.remove();
      }
    }
  }

  public void run() {
    try {
      List<User> users = context.getUsers();
      Set<String> usedDirs = new HashSet<String>();

      for(User u : users) {
        if(fileDeliveriesHelper.isUserFromGroup(u.getLogin(), groupNumber))
          processUser(u, usedDirs);
      }

      cleanWorkDir(usedDirs);
      cleanLastUpdateTimers(usedDirs);

    } catch (Exception e) {
      getMBean().notifyInternalError(UNKNOWN_ERR, e.getMessage());
      log.error(e,e);
    }
  }

  private boolean isUpdateIsNeeded(UserCPsettings ucps, String mapKey, long currentMillis) {
    Time now = new Time(currentMillis);

    if(!now.isInInterval(ucps.getActivePeriodStart(), ucps.getActivePeriodEnd()))
      return false;

    Long lastUs = lastUpdate.get(mapKey);
    return !(lastUs != null && lastUs + (60L * ucps.getPeriodInMin() * 1000) > currentMillis);
  }

  ResourceProcessStrategy getStrategy(User u, File workDir, UserCPsettings ucps, FileResource resource) throws AdminException {
    ResourceOptions opts = new ResourceOptions(u, workDir, ucps, maxTimeSec);
    switch (ucps.getWorkType()) {
      case simple: return new SimpleSaveStrategy(context, resource, opts);
      case detailed: return new DetailedSaveStrategy(context, resource, opts);
      case detailed_period: return new DetailedPeriodSaveStrategy(context, resource, opts);
      default: throw new IllegalArgumentException("Work type is unknown: "+ucps.getWorkType());
    }
  }

  private void processUser(User u, Collection<String> usedDirs) {
    if(u.getCpSettings() ==null)
      return;

    if(log.isDebugEnabled())
      log.debug("Start process user="+u.getLogin()+" group="+groupNumber);

    for(UserCPsettings ucps : u.getCpSettings()) {
      try {
        File userDir = fileDeliveriesHelper.getUserLocalDir(u.getLogin(), ucps);
        String mapKey = userDir.getName();
        usedDirs.add(mapKey);

        long currentMillis = System.currentTimeMillis();

        if (log.isDebugEnabled())
          log.debug("Start: '" + ucps + "'.");

        if(!fileSys.exists(userDir))
          fileSys.mkdirs(userDir);

        ResourceProcessStrategy strategy = getStrategy(u, userDir, ucps, fileDeliveriesHelper.getConnection(ucps));

        boolean allowDeliveryCreation = isUpdateIsNeeded(ucps, mapKey, currentMillis) && isCreationAvailable(u);
        try {
          strategy.process(allowDeliveryCreation);
        } catch (AdminException e) {
          log.error(e,e);
          getMBean().notifyInternalError(RESOURCE_PROC_ERR+": "+ucps,  e.getMessage());
        }
        if (allowDeliveryCreation)
          lastUpdate.put(mapKey, currentMillis);

        if (log.isDebugEnabled())
          log.debug("Finish: '" + ucps + "'.");

      } catch (Exception e) {
        log.error(e, e);
        getMBean().notifyInternalError(RESOURCE_PROC_ERR+": "+ucps,  e.getMessage());
      }
    }

    if(log.isDebugEnabled())
      log.debug("End process user="+u.getLogin()+" group="+groupNumber);
  }
}