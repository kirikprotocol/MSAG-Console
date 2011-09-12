package mobi.eyeline.informer.admin.contentprovider;

import com.eyeline.utils.ThreadFactoryWithCounter;
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
import java.util.concurrent.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;



class MainLoopTask implements Runnable {

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private ContentProviderContext context;
  private FileSystem fileSys;
  private File workDir;

  private static final String USER_PROC_ERR = "User processing error";
  private static final String RESOURCE_PROC_ERR = "Resource processing error";
  private static final String UNKNOWN_ERR = "Unknown error";

  private UserDirResolver userDirResolver;

  private final ScheduledExecutorService executorService;

  private final int groupsSize;

  private final Map<Integer, GroupExecutor> groupExecutors = new HashMap<Integer, GroupExecutor>();

  private final int maxTimeSec;

  public MainLoopTask(ContentProviderContext context, UserDirResolver userDirResolver, File workDir, int groupsSize, int maxTimeSec) throws AdminException {
    this.context = context;
    this.maxTimeSec = maxTimeSec;
    this.fileSys=context.getFileSystem();
    this.workDir=workDir;
    this.userDirResolver=userDirResolver;
    this.executorService = new ScheduledThreadPoolExecutor(2, new ThreadFactoryWithCounter("CP_GROUP_HANDLER-"));
    this.groupsSize = groupsSize;
    cleanUserDirs();
  }

  public void shutdown() {
    executorService.shutdown();
  }


  private int getGroup(String login) {
    return (Math.abs(login.hashCode()))%groupsSize;
  }

  private Map<String, GroupExecutor> getHandledUsers() {
    Map<String, GroupExecutor> res = new HashMap<String, GroupExecutor>();
    for(GroupExecutor g : groupExecutors.values()) {
      for(String u : g.getUsers()) {
        res.put(u, g);
      }
    }
    return res;
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
    try{

      List<User> users = context.getUsers();

      Map<String, GroupExecutor> toRemove = getHandledUsers();

      List<GroupExecutor> addExecutors = new LinkedList<GroupExecutor>();

      for(User u : users) {
        if (u.getCpSettings() != null && !u.getCpSettings().isEmpty()) {
          if(toRemove.containsKey(u.getLogin())) {                          //старый пользователь
            toRemove.remove(u.getLogin());
          }else {                                                           //новый пользователь
            int group = getGroup(u.getLogin());
            GroupExecutor e = groupExecutors.get(group);
            if(e == null) {
              addExecutors.add(e = new GroupExecutor(group));
              groupExecutors.put(e.group, e);
            }
            if(log.isDebugEnabled()) {
              log.debug("Add user '"+u.getLogin()+"' to group: "+group);
            }
            e.addUser(u.getLogin());
          }
        }
      }

      for(Map.Entry<String, GroupExecutor> e : toRemove.entrySet()) {
        GroupExecutor gE = e.getValue();
        if(log.isDebugEnabled()) {
          log.debug("Remove user '"+e.getKey()+"' from group: "+gE.group);
        }
        gE.removeUser(e.getKey());
        if(gE.isEmpty()) {
          if(log.isDebugEnabled()) {
            log.debug("Group is empty. Remove it: "+gE.group);
          }
          groupExecutors.remove(gE.group);
          gE.shutdown();
        }
      }

      for(GroupExecutor gE : addExecutors) {
        if(log.isDebugEnabled()) {
          log.debug("Add new group: "+gE.group);
        }
        ScheduledFuture future = executorService.scheduleWithFixedDelay(gE, 0l, 1l, TimeUnit.MINUTES);
        gE.setTask(future);
      }


    } catch (Exception e) {
      getMBean().notifyInternalError(UNKNOWN_ERR, e.getMessage());
      log.error(e,e);
    }
  }

  private ResourceProcessStrategy getStrategy(User u, File workDir, UserCPsettings ucps, FileResource resource) throws AdminException {
    ResourceOptions opts = new ResourceOptions(u, workDir, ucps, maxTimeSec);

    return ucps.getWorkType() == UserCPsettings.WorkType.simple ? new SimpleSaveStrategy(context, resource, opts)
        : new DetailedSaveStrategy(context, resource, opts);
  }

  private void cleanUserDirs(String login, Collection<String> userDirs) throws AdminException {
    Collection<File> files = userDirResolver.getAllUserLocalDirs(login);
    for(File s : files) {
      if(userDirs == null || !userDirs.contains(s.getName())) {
        fileSys.recursiveDeleteFolder(s);
      }
    }
  }

  private void cleanUserDirs() throws AdminException {
    Set<String> usedDirs = new HashSet<String>();
    for(User u : context.getUsers()) {
      if(u.getCpSettings() != null && !u.getCpSettings().isEmpty()) {
        for(File f : userDirResolver.getAllUserLocalDirs(u.getLogin())) {
          usedDirs.add(f.getName());
        }
      }
    }
    File[] files = fileSys.listFiles(workDir);
    if(files != null) {
      for(File f : files) {
        if(!usedDirs.contains(f.getName())) {
          fileSys.recursiveDeleteFolder(f);
        }
      }
    }
  }


  private class GroupExecutor implements Runnable{

    private Map<String, Long> lastUpdate = new HashMap<String, Long>(100);

    private final int group;

    private final Lock lock = new ReentrantLock();

    private Future task;

    private List<String> removedUsers = new LinkedList<String>();

    private boolean started = true;

    private GroupExecutor(int group) {
      this.group = group;
    }

    private Collection<String> users = new LinkedList<String>();


    public Collection<String> getUsers() {
      return users;
    }

    public boolean isEmpty() {
      return users.isEmpty();
    }

    public void addUser(String login) {
      try{
        lock.lock();
        users.add(login);
        removedUsers.remove(login);
      }finally {
        lock.unlock();
      }
    }

    public void removeUser(String login) {
      try{
        lock.lock();
        users.remove(login);
        removedUsers.add(login);
      }finally {
        lock.unlock();
      }
    }

    public void setTask(Future task) {
      this.task = task;
    }

    public void shutdown() {
      started = false;
      if(task != null) {
        task.cancel(false);
      }
      cleanRemovedUsersDirs();
    }

    private List<String> copyUsers() {
      try{
        lock.lock();
        return new ArrayList<String>(this.users);
      }finally {
        lock.unlock();
      }

    }

    public void run() {

      if(log.isDebugEnabled()) {
        log.debug("Group Executor started: "+group);
      }

      List<String> users = copyUsers();
      Collections.shuffle(users);

      Set<String> checkedUcps = new HashSet<String>();
      try{
        for(String uS : users ) {
          if(!started) {
            return;
          }
          User u = context.getUser(uS);
          List<UserCPsettings> s = u.getCpSettings();
          if(s != null && !s.isEmpty()) {
            try {
              processUser(u, checkedUcps);
            } catch (Exception e) {
              getMBean().notifyInternalError(USER_PROC_ERR, e.getMessage());
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
      } catch (Exception e) {
        getMBean().notifyInternalError(UNKNOWN_ERR, e.getMessage());
        log.error(e,e);
      }finally {
        cleanRemovedUsersDirs();
        if(log.isDebugEnabled()) {
          log.debug("Group Executor finished: "+group);
        }
      }
    }

    private void cleanRemovedUsersDirs() {
      try{
        lock.lock();
        if(!removedUsers.isEmpty()) {
          for(String u : removedUsers) {
            try{
              cleanUserDirs(u, null);
            }catch (Exception ignored){}
          }
          removedUsers.clear();
        }
      }finally {
        lock.unlock();
      }
    }

    private void processUser(User u, Collection<String> checkedUcps) throws AdminException {

      Set<String> userDirs = new HashSet<String>();     //used directories

      if(u.getCpSettings()!=null) {
        for(UserCPsettings ucps : u.getCpSettings()) {
          try{

            String mapKey = ucps.toString();
            checkedUcps.add(mapKey);

            long currentMillis = System.currentTimeMillis();

            if (log.isDebugEnabled())
              log.debug("Start: '" + ucps + "'. User: '" + u.getLogin() + "'...");

            File userDir = userDirResolver.getUserLocalDir(u.getLogin(), ucps);

            if(!fileSys.exists(userDir)) {
              fileSys.mkdirs(userDir);
            }
            userDirs.add(userDir.getName());

            ResourceProcessStrategy strategy = getStrategy(u, userDir, ucps, userDirResolver.getConnection(u, ucps));

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
      cleanUserDirs(u.getLogin(), userDirs);
    }


    private boolean isUpdateIsNeeded(UserCPsettings ucps, String mapKey, long currentMillis) {

      Time now = new Time(currentMillis);

      if(!now.isInInterval(ucps.getActivePeriodStart(), ucps.getActivePeriodEnd())) {
        return false;
      }

      Long lastUs = lastUpdate.get(mapKey);

      return !(lastUs != null && lastUs + (60L * ucps.getPeriodInMin() * 1000) > currentMillis);

    }

  }
}
