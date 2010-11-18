package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DataSource;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.Date;
import java.util.List;


/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 17.11.2010
 * Time: 18:29:48
 */
public class ContentProviderDaemonTask implements Runnable {
  Logger log = Logger.getLogger(this.getClass());
  private AdminContext context;
  private File informerBase;
  private FileSystem fileSys;

  public ContentProviderDaemonTask(AdminContext context, File informerBase, FileSystem fileSys) {
    this.informerBase = informerBase;
    this.context = context;
    this.fileSys=fileSys;
  }


  public void run() {
    try {
      List<User> users = context.getUsers();
      for(User u : users ) {
        if(u.getStatus()==User.Status.ENABLED && u.isImportDeliveriesFromDir()) {
          String dir = u.getDirectory();
          if(dir!=null && dir.length()>0) {
            File userDir = new File(informerBase,dir);
            if(!fileSys.exists(userDir)) throw new IllegalArgumentException("Not found source directory for user="+u.getLogin()+" Dir="+userDir.getAbsolutePath());
            processUserDirectory(u,userDir);
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

    for(File f : files) {
      processUserFile(u, userDir, f);
    }
  }

  private void processUserFile(User u, File userDir, File f)  {
    int inx = f.getName().indexOf('.');
    if(inx<0) return;
    String baseName = f.getName().substring(0,inx);
    String ext      = f.getName().substring(inx+1);

    try {
      if(ext.startsWith("csv.")) {
        int deliveryId = Integer.valueOf(ext.substring(5));
        context.dropDelivery(u.getLogin(),u.getPassword(),deliveryId);
        //rename .csv.<id> to .csv
        File newFile = new File(userDir,baseName+".csv");
        fileSys.rename(f,newFile);
        f=newFile;
        ext=".csv";
      }

      if(ext.equals("csv")) {
        Delivery delivery = new Delivery(Delivery.Type.Common);
        delivery.setName(baseName);
        delivery.setStartDate(new Date(System.currentTimeMillis()+1000));
        context.getDefaultDelivery(u.getLogin(),delivery);
        context.createDelivery(u.getLogin(),u.getPassword(),delivery,null);
        int deliveryId=delivery.getId();
        //rename to .csv.<id>
        File newFile = new File(userDir,baseName+".csv."+deliveryId);
        fileSys.rename(f,newFile);
        f=newFile;
        BufferedReader is=null;
        try {
          String encoding = u.getFileEncoding();
          if(encoding==null) encoding="UTF-8";
          is = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f),encoding));
          context.addMessages(u.getLogin(),u.getPassword(),new CPMessageSource(is),deliveryId);
        }
        finally {
          if(is!=null) try {is.close();} catch (Exception e){}
        }
        //rename to .bak.<id>
        newFile = new File(userDir,baseName+".bak."+deliveryId);
        fileSys.rename(f,newFile);
        f=newFile;
      }
    }
    catch (Exception e) {
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
        log.error("error creating error report",e);
      }

      //rename to err
      File newFile = new File(userDir,baseName+".err");
      try {
          fileSys.rename(f,newFile);
      }
      catch (AdminException ex) {
          log.error("Error renaming file to bak "+f.getAbsolutePath(),ex);
      }
    }
  }



  private class CPMessageSource implements DataSource<Message> {
    private BufferedReader reader;

    public CPMessageSource(BufferedReader reader) {
      this.reader=reader;
    }

    public Message next() throws AdminException {
      try {
        int inx;
        String line;
        while( (line = reader.readLine())!=null ) {
          line = line.trim();
          if(line.length()>0) {
            inx = line.indexOf('|');
            if(inx>0) {
              Address ab   = new Address(line.substring(0,inx).trim());
              String  text = line.substring(inx+1).trim();
              return Message.newMessage(ab,text);
            }
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
