package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DataSource;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryPrototype;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.FileUtils;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;
import java.util.regex.Matcher;
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

  private ContentProviderContext context;
  private FileSystem fileSys;
  private File workDir;


  Pattern unfinishedFileName = Pattern.compile("\\.csv\\.\\d+$");
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
      for(User u : users ) {
        if(u.getStatus()==User.Status.ENABLED && u.isImportDeliveriesFromDir()) {
          try {
            processUser(u);
          }
          catch (Exception e) {
            log.error("Error processing ",e);
          }
        }
      }
    }
    catch (Exception e) {
      log.error("Error in "+ ContentProviderDaemon.NAME+" thread",e);
    }
  }

  private void processUser(User u) throws AdminException {
    List<String> userDirsNames = new ArrayList<String>();
    if(u.isImportDeliveriesFromDir() && u.getCpSettings()!=null) {
      for(UserCPsettings ucps : u.getCpSettings()) {

        if (log.isDebugEnabled())
          log.debug("Processing start: '" + ucps + "'. User: '" + u.getLogin() + "'...");

        File userDir = userDirResolver.getUserLocalDir(u.getLogin(), ucps);
        try {
          if(!fileSys.exists(userDir)) {
            fileSys.mkdirs(userDir);
          }
          userDirsNames.add(userDir.getName());

          downloadUserFilesToLocalDir(u, userDir,ucps);

          processFilesInLocalDir(u, userDir, ucps);
        }
        catch (Exception e) {
          log.error("Error processing u="+u.getLogin()+" ucps="+ucps,e);
        }
        try {
          uploadResults(u, userDir, ucps);
        }
        catch (Exception e) {
          log.error("Result upload error u="+u.getLogin()+" ucps="+ucps.toString(),e);
        }

        if (log.isDebugEnabled())
          log.debug("Processing finished: '" + ucps + "'. User: '" + u.getLogin() + "'...");
      }
    }
    //clean up unused dirs for this user
    String[] allDirs = fileSys.list(workDir);
    if(allDirs!=null) {
      for(String dirName : allDirs) {
        if(dirName.startsWith(u.getLogin()+"_")) {
          if(!userDirsNames.contains(dirName)) {
            FileUtils.recursiveDeleteFolder(new File(workDir,dirName));
          }
        }
      }
    }
  }

  private boolean downloadFile(FileResource connection, String remoteFile, File toDit) throws AdminException {
    File localTmpFile = new File(toDit, remoteFile + ".tmp");
    if (fileSys.exists(localTmpFile))
      fileSys.delete(localTmpFile);

    try {
      if (log.isDebugEnabled())
        log.debug("  Download remote file: '" + remoteFile + "' to local '" + localTmpFile + "'.");
      connection.get(remoteFile,localTmpFile);

      String renameRemoteTo = remoteFile + ".active";
      if (log.isDebugEnabled())
        log.debug("  Rename remote file: '" + remoteFile + "' to '" + renameRemoteTo + "'.");
      connection.rename(remoteFile, renameRemoteTo);

      File renameLocalTmpTo = new File(localTmpFile.getParentFile(), localTmpFile.getName().substring(0,localTmpFile.getName().length()-4));
      if (log.isDebugEnabled())
        log.debug("  Rename local file: '" + localTmpFile + "' to '" + renameLocalTmpTo.getName() + "'.");
      fileSys.rename(localTmpFile,renameLocalTmpTo);

      return true;
    } catch (Exception e) {
      try {fileSys.delete(localTmpFile);} catch (Exception ignored){}
      log.error("Error loading file: " + toDit.getName() + File.separator + remoteFile,e);
      return false;
    }
  }



  void downloadUserFilesToLocalDir(User user, File userDir, UserCPsettings ucps) throws Exception {
    if (log.isDebugEnabled())
      log.debug("Downloading files...");

    int counter = 0;
    FileResource connection = null;
    try {
      connection = userDirResolver.getConnection(user,ucps);
      connection.open();

      List<String> remoteFiles = connection.listCSVFiles();
      for (String remoteFile : remoteFiles) {
        if (downloadFile(connection, remoteFile, userDir))
          counter++;
      }

    } finally {
      if (log.isDebugEnabled())
        log.debug(counter + " files downloaded.");
      if (connection != null) connection.close();
    }
  }


  private void processFilesInLocalDir(User u, File userDir, UserCPsettings ucps) {
    if (log.isDebugEnabled())
      log.debug("Process local files...");

    File[] files = fileSys.listFiles(userDir);
    if(files==null) {
      log.error("Can't get directory listing for user="+u.getLogin()+" Dir="+userDir.getAbsolutePath());
      return;
    }
    for(File f : files) {
      processUnfinished(u,f);
    }

    files = fileSys.listFiles(userDir);
    if(files==null) {
      log.error("Can't get directory listing for user="+u.getLogin()+" Dir="+userDir.getAbsolutePath());
      return;
    }
    for(File f : files) {
      processUserFile(u,f, ucps);
    }


  }

  private void uploadResults(User u, File userDir, UserCPsettings ucps) throws AdminException {
    File[] files = fileSys.listFiles(userDir);
    if(files==null) {
      log.error("Can't get directory listing for user="+u.getLogin()+" Dir="+userDir.getAbsolutePath());
      return;
    }

    List<File> uploadFiles = new ArrayList<File>();
    List<File> files2remove = new ArrayList<File>();
    Set<String> finishedDeliveries = new HashSet<String>();
    for(File f : files) {
      String fileName = f.getName();
      if (fileName.endsWith(".err")) {
        finishedDeliveries.add(fileName.substring(0,fileName.length()-".err".length()));
        uploadFiles.add(f);
      } else if (fileName.endsWith(".errLog")) {
        finishedDeliveries.add(fileName.substring(0,fileName.length()-".errLog".length()));
        uploadFiles.add(f);
      } else if (fileName.endsWith(".report")) {
        finishedDeliveries.add(fileName.substring(0,fileName.length()-".report".length()));
        uploadFiles.add(f);
      } else if (fileName.endsWith(".fin")) {
        finishedDeliveries.add(fileName.substring(0,fileName.length()-".fin".length()));
        files2remove.add(f);
      }
    }

    if(!uploadFiles.isEmpty() || !finishedDeliveries.isEmpty()) {
      FileResource connection = null;
      String baseName=null;
      try {
        connection = userDirResolver.getConnection(u,ucps);
        connection.open();

        for(File f : uploadFiles) {
          try {
            if (log.isDebugEnabled())
              log.debug("Upload file: '" + f.getAbsolutePath() + "' to remote dir: '" + ucps + "'.");
            connection.put(f,f.getName());
            files2remove.add(f);
          }
          catch (Exception e) {
            log.error("Unable to upload file="+f.getAbsolutePath()+" to u="+u.getLogin()+" ucps="+ucps.toString(),e);
          }
        }

        for (String delivery : finishedDeliveries) {
          try {
            if (log.isDebugEnabled())
              log.debug("Rename remote file at " + ucps + ". Old name: '" + delivery + ".csv.active" + "' new name: '" + delivery+".csv.finished" + "'.");
            connection.rename(delivery + ".csv.active", delivery+".csv.finished");
          } catch (Exception e) {
            log.error("Unable rename remote file to finished state="+baseName+".csv.active",e);
          }
        }

        for (File file : files2remove)
          fileSys.delete(file);

      }
      finally {
        if (connection != null) connection.close();
      }
    }
  }


  private void handleErrorProccessingFile(Exception e, File userDir, File f, String baseName, String username, String password, Integer deliveryId) {
    File errLogFile = new File(userDir,baseName+".errLog");
    try {
      PrintWriter pw = null;
      try {
        pw = new PrintWriter(fileSys.getOutputStream(errLogFile,true));
        e.printStackTrace(pw);
      }
      finally {
        if(pw!=null) pw.close();
      }
    }
    catch (AdminException e1) {
      log.error("error creating error report",e1);
      log.error("unsaved exception ",e);
    }

    //rename to err
    File errFile = new File(userDir,baseName+".err");
    try {
      fileSys.rename(f,errFile);
    }
    catch (AdminException ex) {
      log.error("Error renaming file to err "+f.getAbsolutePath(),ex);
    }
    //delete report
    File reportFile = new File(userDir,baseName+".rep");
    try {
      if(fileSys.exists(reportFile)) {
        fileSys.delete(reportFile);
      }
    }
    catch (AdminException ex) {
      log.error("Error deleting report "+f.getAbsolutePath(),ex);
    }

    if(deliveryId!=null) {
      try {
        context.dropDelivery(username,password,deliveryId);
      }
      catch (Exception ex) {
        log.error("Error removing delivery "+deliveryId,ex);
      }
    }
  }

  private void processUnfinished(User u, File f) {

    Matcher unfinishedMatcher = unfinishedFileName.matcher(f.getName());

    if(unfinishedMatcher.matches()) {
      if (log.isDebugEnabled())
        log.debug("  Unfinished file found: '" + f.getAbsolutePath() + "'. According delivery will be deleted.");

      File userDir = f.getParentFile();
      String ext = unfinishedMatcher.group();
      String baseName = f.getName().substring(0,f.getName().length()-ext.length());
      Integer deliveryId =null;
      try {
        deliveryId = Integer.valueOf(ext.substring(5));
        context.dropDelivery(u.getLogin(),u.getPassword(),deliveryId);
        //rename .csv.<id> to .csv
        File newFile = new File(userDir,baseName+".csv");
        if(fileSys.exists(newFile)){
          fileSys.delete(f);
        }
        else {
          fileSys.rename(f,newFile);
        }
        File reportFile = new File(userDir,baseName+".rep."+deliveryId);
        if(fileSys.exists(reportFile)){
          fileSys.delete(reportFile);
        }
      }
      catch (Exception e) {
        handleErrorProccessingFile(e, userDir, f, baseName, u.getLogin(), u.getPassword(), deliveryId);
      }
    }
  }


  private void processUserFile(User u, File f, UserCPsettings ucps)  {

    String fileName = f.getName();
    if(fileName.endsWith(".csv")) {
      File userDir = f.getParentFile();
      String baseName = fileName.substring(0,fileName.length()-4);
      Integer deliveryId=null;
      try {
        if (log.isDebugEnabled())
          log.debug("  Try to create delivery from file: '" + f.getAbsolutePath() + "'. User: '" + u.getLogin() + "'.");

        DeliveryPrototype delivery = new DeliveryPrototype();
        delivery.setName(baseName);
        delivery.setStartDate(new Date());
        context.copyUserSettingsToDeliveryPrototype(u.getLogin(),delivery);
        delivery.setSourceAddress(ucps.getSourceAddress());
        delivery.setEnableStateChangeLogging(true);
        Delivery d = context.createDelivery(u.getLogin(),u.getPassword(),delivery,null);
        deliveryId = d.getId();
        //rename to .csv.<id>
        File newFile = new File(userDir,baseName+".csv."+deliveryId);
        File reportFile = new File(userDir,baseName+".rep."+deliveryId);
        fileSys.rename(f,newFile);
        f=newFile;
        BufferedReader is=null;
        PrintStream reportWriter = null;
        try {
          String encoding = ucps.getEncoding();
          if(encoding==null) encoding="UTF-8";
          is = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f),encoding));
          reportWriter = new PrintStream(fileSys.getOutputStream(reportFile,true),true,encoding);
          context.addMessages(u.getLogin(),u.getPassword(),new CPMessageSource(
              u.isAllRegionsAllowed() ? null : u.getRegions(),
              is,
              reportWriter
          ),deliveryId);
        }
        finally {
          if(is!=null) try {is.close();} catch (Exception e){}
          if(reportWriter!=null) try {reportWriter.close();} catch (Exception e){}
        }
        context.activateDelivery(u.getLogin(),u.getPassword(),deliveryId);
        fileSys.delete(f);

        if (log.isDebugEnabled())
          log.debug("  Delivery successfully created from file: '" + f.getAbsolutePath() + "'. File was removed.");
      }
      catch (Exception e) {
        handleErrorProccessingFile(e, userDir, f, baseName, u.getLogin(), u.getPassword(), deliveryId);
      }
    }
  }




  private class CPMessageSource implements DataSource<Message> {
    private BufferedReader reader;
    List<Integer> regions;
    PrintStream reportWriter;

    public CPMessageSource(List<Integer> regions, BufferedReader reader, PrintStream reportWriter) {
      this.regions = regions;
      this.reader=reader;
      this.reportWriter=reportWriter;
    }



    public Message next() throws AdminException {
      try {
        int inx;
        String line;
        while( (line = reader.readLine())!=null ) {
          line = line.trim();
          String abonent="";
          if(line.length()==0) continue;
          try {
            inx = line.indexOf('|');
            abonent = line.substring(0,inx).trim();
            Address ab;
            try {
              if(abonent.startsWith("7"))      abonent="+"+abonent;
              else if(abonent.startsWith("8")) abonent="+7"+abonent.substring(1);
              ab = new Address(abonent);
            }
            catch (Exception e) {
              ReportFormatter.writeReportLine(reportWriter, abonent, new Date(), "INVALID ABONENT");
              continue;
            }
            boolean skip = false;
            if(regions!=null) {
              Region r = context.getRegion(ab);
              if(r==null || !regions.contains(r.getRegionId())) {
                skip = true;
              }
            }
            if(skip) {
              ReportFormatter.writeReportLine(reportWriter, abonent, new Date(), "NOT ALLOWED REGION");
              continue;
            }
            String  text = line.substring(inx+1).trim();
            return Message.newMessage(ab,text);
          }
          catch(Exception e) {
            ReportFormatter.writeReportLine(reportWriter, abonent, new Date(), "ERROR PARSING LINE :" + line);
          }
        }
      }
      catch (IOException ioe) {
        throw new ContentProviderException("ioerror",ioe);
      }
      return null;
    }
  }


}
