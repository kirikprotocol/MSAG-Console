package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.Collection;


class FileDeliveriesHelper {

  private final Logger logger;

  private final File workDir;
  private final File informerBase;
  private final ContentProviderContext context;
  private final FileSystem fileSys;
  private final int groupSize;


  FileDeliveriesHelper(ContentProviderContext context, File informerBase, File workDir, int groupSize, Logger logger) throws AdminException {
    this.workDir = workDir;
    this.informerBase = informerBase;
    this.context = context;
    this.groupSize = groupSize;
    this.fileSys = context.getFileSystem();
    this.logger = logger;
  }


  File getUserLocalDir(String login, UserCPsettings ucps) throws AdminException {
    return new File(workDir,login+"_"+ucps.getHashId());
  }


  String getUserByDir(String dirName) {
    if(dirName == null || dirName.length()<34) {
      return null;
    }
    return dirName.substring(0, dirName.length()-33);
  }


  boolean isUserFromGroup(String name, int groupNumber) {
    return groupNumber == (Math.abs(name.hashCode())%groupSize);
  }


  void cleanUnusedDirs(Integer group, Collection<String> excludeDirs) throws AdminException {
    String[] allDirs = fileSys.list(workDir);
    if(allDirs!=null) {
      for(String dirName : allDirs) {
        if(!excludeDirs.contains(dirName)) {
          String userName = null;
          if(group != null) {
            userName = getUserByDir(dirName);
            if(userName == null || !isUserFromGroup(userName, group)) {
              continue;
            }
          }
          if(logger.isDebugEnabled()) {
            logger.debug("Unused dir: "+dirName+" will be removed, user="+userName+" group="+group);
          }
          fileSys.recursiveDeleteFolder(new File(workDir,dirName));
        }
      }
    }
  }

  FileResource getConnection(UserCPsettings ucps) throws AdminException {
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
      localDir = new File(informerBase, dir);
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

}
