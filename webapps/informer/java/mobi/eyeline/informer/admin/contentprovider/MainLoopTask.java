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
import mobi.eyeline.informer.util.Time;
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
      log.error("Error in "+ ContentProviderDaemon.NAME+" thread",e);
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

  ResourceProcessStrategy getStrategy(UserCPsettings ucps) {
    return ucps.getWorkType() == UserCPsettings.WorkType.simple
            ? new SimpleResourceProcessStrategy() : new DetailedResourceProcessStrategy();
  }

  private void processUser(User u, Collection<String> checkedUcps) throws AdminException {
    List<String> userDirsNames = new ArrayList<String>();
    if(u.getCpSettings()!=null) {
      for(UserCPsettings ucps : u.getCpSettings()) {

        String mapKey = ucps.toString();

        checkedUcps.add(mapKey);

        long currentMillis = System.currentTimeMillis();

        if(!isUpdateIsNeeded(ucps, mapKey, currentMillis)) {
          return;
        }

        if (log.isDebugEnabled())
          log.debug("Processing start: '" + ucps + "'. User: '" + u.getLogin() + "'...");

        File userDir = userDirResolver.getUserLocalDir(u.getLogin(), ucps);

        if(!fileSys.exists(userDir)) {
          fileSys.mkdirs(userDir);
        }
        userDirsNames.add(userDir.getName());

        ResourceProcessStrategy strategy = getStrategy(ucps);

        strategy.process(u, userDir, ucps);

        lastUpdate.put(mapKey, currentMillis);

        if (log.isDebugEnabled())
          log.debug("Processing finished: '" + ucps + "'. User: '" + u.getLogin() + "'...");
      }
    }
    cleanUpDirs(u.getLogin(), userDirsNames);
  }


  private void downloadUserFilesToLocalDir(User user, File userDir, UserCPsettings ucps,  Downloader downloader) throws Exception {
    if (log.isDebugEnabled())
      log.debug("Downloading files...");

    int counter = 0;
    FileResource connection = null;
    try {
      connection = userDirResolver.getConnection(user,ucps);
      connection.open();

      List<String> remoteFiles = connection.listCSVFiles();
      for (String remoteFile : remoteFiles) {
        if (downloader.download(connection, remoteFile, userDir, ucps))
          counter++;
      }

    } finally {
      if (log.isDebugEnabled())
        log.debug(counter + " files downloaded.");
      if (connection != null) connection.close();
    }
  }


  private void processFilesInLocalDir(User u, File userDir, String encoding, DeliveryPrototype prototype) {
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
      processUserFile(u, f, encoding, prototype);
    }

  }

  private void handleErrorProccessingFile(Exception e, File userDir, File f, String baseName, String username,  Integer deliveryId) {
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
        context.dropDelivery(username,deliveryId);
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
        context.dropDelivery(u.getLogin(),deliveryId);
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
        handleErrorProccessingFile(e, userDir, f, baseName, u.getLogin(), deliveryId);
      }
    }
  }


  private void processUserFile(User u, File f, String encoding, DeliveryPrototype prototype)  {

    String fileName = f.getName();
    if(fileName.endsWith(".csv")) {
      File userDir = f.getParentFile();
      String baseName = fileName.substring(0,fileName.length()-4);
      Integer deliveryId=null;
      try {
        if (log.isDebugEnabled())
          log.debug("  Try to create delivery from file: '" + f.getAbsolutePath() + "'. User: '" + u.getLogin() + "'.");

        prototype.setName(baseName);

        Delivery d = context.createDeliveryWithIndividualTexts(u.getLogin(), prototype, null);
        deliveryId = d.getId();
        //rename to .csv.<id>
        File newFile = new File(userDir,baseName+".csv."+deliveryId);
        File reportFile = new File(userDir,baseName+".rep."+deliveryId);
        fileSys.rename(f,newFile);
        f=newFile;
        BufferedReader is=null;
        PrintStream reportWriter = null;
        try {
          if(encoding==null) encoding="UTF-8";
          is = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f),encoding));
          reportWriter = new PrintStream(fileSys.getOutputStream(reportFile,true),true,encoding);
          context.addMessages(u.getLogin(),new CPMessageSource(
              u.isAllRegionsAllowed() ? null : u.getRegions(),
              is,
              reportWriter
          ),deliveryId);
        }
        finally {
          if(is!=null) try {is.close();} catch (Exception ignored){}
          if(reportWriter!=null) try {reportWriter.close();} catch (Exception ignored){}
        }
        context.activateDelivery(u.getLogin(),deliveryId);
        fileSys.delete(f);

        if (log.isDebugEnabled())
          log.debug("  Delivery successfully created from file: '" + f.getAbsolutePath() + "'. File was removed.");
      }
      catch (Exception e) {
        handleErrorProccessingFile(e, userDir, f, baseName, u.getLogin(), deliveryId);
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

    /**
     * Десериализует сообщение из строки
     * @param line строка
     * @return сообщение или null, если строка не содержит сообщения
     * @throws IllegalArgumentException если строка имеет некорректный формат
     */
    private Message parseLine(String line) {
      int inx = line.indexOf('|');
      if (inx < 0)
        return null;

      String  abonent = line.substring(0,inx).trim();
      Address ab;
      try {
        ab = new Address(abonent);
      } catch (Exception e) {
        throw new IllegalArgumentException("INVALID ABONENT");
      }

      if(regions!=null) {
        Region r = context.getRegion(ab);
        if(r==null || !regions.contains(r.getRegionId()))
          throw new IllegalArgumentException("NOT ALLOWED REGION");
      }

      String text, userData = null;
      int nextInx = line.indexOf('|', inx+1);
      if (nextInx < 0)
        text = line.substring(inx+1).trim();
      else {
        userData = line.substring(inx+1, nextInx);
        text = line.substring(nextInx+1).trim();
      }

      Message m = Message.newMessage(ab,text);
      if (userData != null)
        m.setProperty("userData", userData);
      return m;
    }



    public Message next() throws AdminException {
      try {
        String line;
        while( (line = reader.readLine())!=null ) {
          line = line.trim();
          if(line.length()==0) continue;
          try {
            Message m = parseLine(line);
            if (m == null)
              continue;
            return m;
          } catch (IllegalArgumentException e) {
            ReportFormatter.writeReportLine(reportWriter, line, new Date(), e.getMessage());
          } catch(Exception e) {
            ReportFormatter.writeReportLine(reportWriter, line, new Date(), "ERROR PARSING LINE :" + line);
          }
        }
      }
      catch (IOException ioe) {
        throw new ContentProviderException("ioerror",ioe);
      }
      return null;
    }
  }



  private class DetailedResourceProcessStrategy implements ResourceProcessStrategy {

    public void process(User u, File userDir, UserCPsettings ucps) throws AdminException {

      try {
        downloadUserFilesToLocalDir(u, userDir, ucps, new Downloader() {
          public boolean download(FileResource resource, String remoteFile, File toDir, UserCPsettings ucps) throws AdminException {
            File localTmpFile = new File(toDir, remoteFile + ".tmp");
            if (fileSys.exists(localTmpFile))
              fileSys.delete(localTmpFile);
            try {
              resource.get(remoteFile, localTmpFile);

              String renameRemoteTo = remoteFile + ".active";

              resource.rename(remoteFile, renameRemoteTo);

              File renameLocalTmpTo = new File(localTmpFile.getParentFile(), localTmpFile.getName().substring(0, localTmpFile.getName().length()-4));

              fileSys.rename(localTmpFile,renameLocalTmpTo);

              return true;
            } catch (Exception e) {
              try {fileSys.delete(localTmpFile);} catch (Exception ignored){}
              log.error("Error loading file: " + remoteFile,e);
              return false;
            }
          }
        });


        DeliveryPrototype delivery = new DeliveryPrototype();
        delivery.setStartDate(new Date());
        context.copyUserSettingsToDeliveryPrototype(u.getLogin(), delivery);
        delivery.setSourceAddress(ucps.getSourceAddress());
        delivery.setEnableStateChangeLogging(true);

        processFilesInLocalDir(u, userDir, ucps.getEncoding(), delivery);

      } catch (Exception e) {
        log.error("Error processing u="+u.getLogin()+" ucps="+ucps,e);
      }

      try {
        processResults(u, userDir, ucps);
      } catch (Exception e) {
        log.error("Result upload error u="+u.getLogin()+" ucps="+ucps.toString(),e);
      }

    }

    private void processResults(User u, File userDir, UserCPsettings ucps) throws AdminException {
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
  }


  class SimpleResourceProcessStrategy implements ResourceProcessStrategy {

    public void process(User u, File userDir, UserCPsettings ucps) throws AdminException {

      try {
        downloadUserFilesToLocalDir(u, userDir, ucps, new Downloader() {

          public boolean download(FileResource resource, String remoteFile, File toDit, UserCPsettings ucps) throws AdminException {
            File localTmpFile = new File(toDit, remoteFile + ".tmp");
            if (fileSys.exists(localTmpFile))
              fileSys.delete(localTmpFile);
            try {
              resource.get(remoteFile,localTmpFile);

              resource.remove(remoteFile);

              File renameLocalTmpTo = new File(localTmpFile.getParentFile(), localTmpFile.getName().substring(0, localTmpFile.getName().length()-4));

              fileSys.rename(localTmpFile,renameLocalTmpTo);

              return true;
            } catch (Exception e) {
              try {fileSys.delete(localTmpFile);} catch (Exception ignored){}
              log.error("Error loading file: " + remoteFile,e);
              return false;
            }
          }
        });

        DeliveryPrototype delivery = new DeliveryPrototype();
        delivery.setStartDate(new Date());
        context.copyUserSettingsToDeliveryPrototype(u.getLogin(), delivery);
        delivery.setSourceAddress(ucps.getSourceAddress());
        delivery.setEnableStateChangeLogging(false);

        processFilesInLocalDir(u, userDir, ucps.getEncoding(), delivery);

      } catch (Exception e) {
        log.error("Error processing u="+u.getLogin()+" ucps="+ucps,e);
      }
    }
  }

  private static interface Downloader {

    public boolean download(FileResource connection, String remoteFile, File toDir, UserCPsettings ucps) throws AdminException;

  }

}
