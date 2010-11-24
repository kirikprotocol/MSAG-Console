package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DataSource;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.Date;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 17.11.2010
 * Time: 18:29:48
 */
class ContentProviderImportTask implements Runnable {
  Logger log = Logger.getLogger(this.getClass());
  private AdminContext context;
  ContentProviderDaemon daemon;
  private FileSystem fileSys;

  Pattern unfinishedFileName = Pattern.compile("\\.csv\\.\\d+$");

  public ContentProviderImportTask(AdminContext context,ContentProviderDaemon daemon, FileSystem fileSys) {
    this.daemon = daemon;
    this.context = context;
    this.fileSys=fileSys;
  }


  public void run() {
    try {
      List<User> users = context.getUsers();
      for(User u : users ) {
        if(u.getStatus()==User.Status.ENABLED && u.isImportDeliveriesFromDir()) {
          try {
            File userDir = daemon.getUserDirectory(u);
            processUserDirectory(u,userDir);
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

  private void processUserDirectory(User u, File userDir) {
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
      processUserFile(u,f);
    }
  }


  private void handleErrorProccessingFile(Exception e, File userDir, File f, String baseName, String username, String password, Integer deliveryId) {
    try {
      PrintWriter pw = null;
      try {
        pw = new PrintWriter(fileSys.getOutputStream(new File(userDir,baseName+".errLog"),true));
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
    File newFile = new File(userDir,baseName+".err");
    try {
      fileSys.rename(f,newFile);
    }
    catch (AdminException ex) {
      log.error("Error renaming file to bak "+f.getAbsolutePath(),ex);
    }
    //delete report
    File reportFile = new File(userDir,baseName+".rep");
    try {
      if(fileSys.exists(reportFile)) {
        fileSys.delete(reportFile);
      }
    }
    catch (AdminException ex) {
      log.error("Error renaming file to bak "+f.getAbsolutePath(),ex);
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
      File userDir = f.getParentFile();
      String ext = unfinishedMatcher.group();
      String baseName = f.getName().substring(0,f.getName().length()-ext.length());
      Integer deliveryId =null;
      try {
          deliveryId = Integer.valueOf(ext.substring(5));
          context.dropDelivery(u.getLogin(),u.getPassword(),deliveryId);
          //rename .csv.<id> to .csv
          File newFile = new File(userDir,baseName+".csv");
          fileSys.rename(f,newFile);
          File reportFile = new File(userDir,baseName+".rep."+deliveryId);
          fileSys.delete(reportFile);
      }
      catch (Exception e) {
        handleErrorProccessingFile(e, userDir, f, baseName, u.getLogin(), u.getPassword(), deliveryId);
      }
    }
  }


  private void processUserFile(User u, File f)  {

    String fileName = f.getName();
    if(fileName.endsWith(".csv")) {
      File userDir = f.getParentFile();
      String baseName = fileName.substring(0,fileName.length()-4);
      Integer deliveryId=null;
      try {
        Delivery delivery = new Delivery(Delivery.Type.Common);
        delivery.setName(baseName);
        delivery.setStartDate(new Date(System.currentTimeMillis()));
        context.getDefaultDelivery(u.getLogin(),delivery);
        context.createDelivery(u.getLogin(),u.getPassword(),delivery,null);
        deliveryId = delivery.getId();
        //rename to .csv.<id>
        File newFile = new File(userDir,baseName+".csv."+deliveryId);
        File reportFile = new File(userDir,baseName+".rep."+deliveryId);
        fileSys.rename(f,newFile);
        f=newFile;
        BufferedReader is=null;
        PrintStream reportWriter = null;
        try {
          String encoding = u.getFileEncoding();
          if(encoding==null) encoding="UTF-8";
          is = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f),encoding));
          reportWriter = new PrintStream(fileSys.getOutputStream(reportFile,true),true,encoding);
          context.addMessages(u.getLogin(),u.getPassword(),new CPMessageSource(
              u.isAllRegionsAllowed() ? null : u.getRegions(),
              is,
              reportWriter
          ),deliveryId);
          context.activateDelivery(u.getLogin(),u.getPassword(),deliveryId);
        }
        finally {
          if(is!=null) try {is.close();} catch (Exception e){}
          if(reportWriter!=null) try {reportWriter.close();} catch (Exception e){}
        }
        //rename to .bak.<id>
        newFile = new File(userDir,baseName+".bak."+deliveryId);
        fileSys.rename(f,newFile);
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
              Address ab = null;
              try {
                  ab = new Address(abonent);
              }
              catch (Exception e) {
                ContentProviderDaemon.writeReportLine(reportWriter,abonent,new Date(),"INVALID ABONENT");
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
                ContentProviderDaemon.writeReportLine(reportWriter,abonent,new Date(),"NOT ALLOWED REGION");
                continue;
              }
              String  text = line.substring(inx+1).trim();
              return Message.newMessage(ab,text);
          }
          catch(Exception e) {
            ContentProviderDaemon.writeReportLine(reportWriter,abonent,new Date(),"ERROR PARSING LINE :"+line);
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
