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
            // todo Если директория не найдена, стоит просто перейти к следующему пользователю.
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
    // todo listFiles может вернуть null (например, если админ не даст прав на чтение/запись директории). Надо это отлавливать и корректно в лог писать.
    for(File f : files) {
      processUserFile(u, userDir, f);
    }
  }

  private void processUserFile(User u, File userDir, File f)  { // todo лишний параметр userDir. Может быть получен из f.getParent(). Надо выкинуть.
    int inx = f.getName().indexOf('.'); //todo нужен более умный алгоритм поиска расширения. Этот не позволяет ставить точку в имени файла.
    if(inx<0) return;
    String baseName = f.getName().substring(0,inx);
    String ext      = f.getName().substring(inx+1);

    try {
      if(ext.startsWith("csv.")) {
        int deliveryId = Integer.valueOf(ext.substring(4));
        context.dropDelivery(u.getLogin(),u.getPassword(),deliveryId);
        //rename .csv.<id> to .csv
        File newFile = new File(userDir,baseName+".csv");
        fileSys.rename(f,newFile);
        File reportFile = new File(userDir,baseName+".rep."+deliveryId);
        fileSys.delete(reportFile);
        f=newFile;
        ext="csv";
      }

      if(ext.equals("csv")) {
        Delivery delivery = new Delivery(Delivery.Type.Common);
        delivery.setName(baseName);
        delivery.setStartDate(new Date(System.currentTimeMillis()));
        context.getDefaultDelivery(u.getLogin(),delivery);
        context.createDelivery(u.getLogin(),u.getPassword(),delivery,null);
        int deliveryId=delivery.getId();
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
      File newFile = new File(userDir,baseName+".err"); //todo кажется, забыл удалить файл с отчетом
      try {
        fileSys.rename(f,newFile);
      }
      catch (AdminException ex) {
        log.error("Error renaming file to bak "+f.getAbsolutePath(),ex);
      }
    }
  }

  // todo Предлагаю провести вот такой рефакторинг:
  // todo 1. Метод processUserDirectory сначала ищет все файлы, чье расширение начинается на 'csv.'.
  // todo    Для каждого такого файла он удаляет рассылку и статистику, после чего переименовывает его в csv.
  // todo 2. Далее метод processUserDirectory ищет все файлы с расширением 'csv' и создает по ним рассылки.



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
              Address ab = new Address(abonent);  //todo Надо аккуратно отловить и записать в отчет, что адрес абонента некорректен
              boolean skip = false;
              if(regions!=null) {
                Region r = context.getRegion(ab);
                if(r==null || !regions.contains(r.getRegionId())) {
                  skip = true;
                }
              }
              if(skip) {
                ContentProviderDaemon.writeReportLine(reportWriter,ab.getSimpleAddress(),new Date(),"NOT ALLOWED REGION");
                continue;
              }
              String  text = line.substring(inx+1).trim();
              return Message.newMessage(ab,text);
          }
          catch(Exception e) { //todo просто переходим к следующей строке...
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
