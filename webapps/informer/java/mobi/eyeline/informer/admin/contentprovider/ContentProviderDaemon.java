package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.Daemon;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.delivery.DeliveryNotification;
import mobi.eyeline.informer.admin.users.User;
import org.apache.log4j.Logger;

import java.io.*;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 17.11.2010
 * Time: 16:21:41
 */
public class ContentProviderDaemon extends DeliveryNotificationsAdapter implements Daemon  {
  Logger log = Logger.getLogger(this.getClass());
  private ScheduledExecutorService scheduler;
  private ScheduledExecutorService reportScheduler;
  static final String NAME = "ContentProviderDaemon";
  static final String NAME_REPORT = "ContentProviderDaemonReports";
  private static final long PERIOD_MSEC = 1000L;
  private static final long SHUTDOWN_WAIT_TIME = 2000L;
  FileSystem fileSys;
  AdminContext context;
  File informerBase;
  File workDir;


  private static ThreadLocal<DateFormat> dateFormat = new ThreadLocal<DateFormat>(){
    @Override protected DateFormat initialValue() {
      return new SimpleDateFormat("yyyy.MM.dd HH:mm:ss");
    }
  };

  public ContentProviderDaemon(AdminContext context, File informerBase, File workDir, FileSystem fileSys) throws AdminException {  // todo Плохая зависимость от AdminContext. Надо постараться убрать. По аналогии с Siebel.
    this.workDir = new File(workDir,"contentProvider");
    if(!fileSys.exists(this.workDir)) {
      fileSys.mkdirs(this.workDir);
    }
    this.informerBase = informerBase;
    this.context = context;
    this.fileSys=fileSys;
  }

  public String getName() {
    return NAME;  //To change body of implemented methods use File | Settings | File Templates.
  }

  public synchronized void start() throws AdminException {
    if(scheduler!=null) return;
    scheduler = Executors.newSingleThreadScheduledExecutor(new ThreadFactory(){
      public Thread newThread(Runnable runnable) {
        return new Thread(runnable,NAME);
      }
    });

    scheduler.scheduleAtFixedRate(
        new ContentProviderDaemonTask(context,this,fileSys),
        0,
        PERIOD_MSEC,
        TimeUnit.MILLISECONDS
    );


    reportScheduler = Executors.newSingleThreadScheduledExecutor(new ThreadFactory(){
      public Thread newThread(Runnable runnable) {
        return new Thread(runnable,NAME_REPORT);
      }
    });

    reportScheduler.schedule(new Runnable(){
      public void run() {
        processNotifications();
      }
    },0,TimeUnit.MILLISECONDS);

  }

  public synchronized void stop() throws AdminException {
    scheduler.shutdown();
    reportScheduler.shutdown();
    try {
      scheduler.awaitTermination(SHUTDOWN_WAIT_TIME, TimeUnit.MILLISECONDS);
      reportScheduler.awaitTermination(SHUTDOWN_WAIT_TIME, TimeUnit.MILLISECONDS);
    }
    catch (InterruptedException e) {
      if(!scheduler.isShutdown() )scheduler.shutdownNow();
      if(!reportScheduler.isShutdown() )reportScheduler.shutdownNow();
    }
    scheduler = null;
    reportScheduler = null;
  }

  public boolean isStarted() {
    return scheduler!=null;
  }


  public static void writeReportLine(PrintStream reportWriter, String abonent, Date date, String s) {
    reportWriter.print(abonent);
    reportWriter.print(" | ");
    reportWriter.print(dateFormat.get().format(date));
    reportWriter.print(" | ");
    reportWriter.println(s);
  }

  @Override
  public void onDeliveryFinishNotification(DeliveryNotification notification) {

    PrintStream ps = null;
    try {
      File notificationFile = new File(workDir,notification.getDeliveryId()+".notification");
      ps = new PrintStream(fileSys.getOutputStream(notificationFile,false),true,"utf-8");
      ps.println(notification.getUserId());
      synchronized (this) {
        if(isStarted()) {
          reportScheduler.schedule(new Runnable(){  //todo Точно такой же Runnable созается в методе start. Предлагаю оформить всю логику по формированию отчетов в отдельном классе.
            public void run() {
              processNotifications();
            }
          },0,TimeUnit.MILLISECONDS);
        }
      }
    }
    catch (Exception e) {
      log.error("Error processing delivery finished report for delivery "+notification.getDeliveryId(),e);
    }
    finally {
      if(ps!=null) try {ps.close();} catch (Exception e){}
    }


  }


  private void processNotifications() {
    File[] files = fileSys.listFiles(workDir);
    if(files==null) {
      log.error("Error listing of working directory "+workDir.getAbsolutePath());
      return;
    }
    for(File f : files) {
      if(f.getName().endsWith(".notification")) {
        String sId = f.getName().substring(0,f.getName().length()-".notification".length());
        BufferedReader reader=null;
        try {
          int deliveryId = Integer.valueOf(sId);
          reader = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f),"utf-8"));
          String userName = reader.readLine().trim();
          createReport(deliveryId,userName);
        }
        catch (Exception e){
          log.error("error processing file "+f.getAbsolutePath());
          try {
            fileSys.rename(f,new File(workDir,sId+".err"));
          }
          catch (Exception ex){
            log.error("unable to rename file to "+sId+".err",ex);
          }
        }
        finally {
          if(reader!=null) try {reader.close();} catch (Exception e){}
          try {
            if(fileSys.exists(f)) fileSys.delete(f);
          }
          catch (Exception e) {
            log.error("unable to delete file"+f.getAbsolutePath(),e);
          }
        }
      }
    }
  }


  private void createReport(int deliveryId, String userName) throws AdminException, UnsupportedEncodingException {
    User user = context.getUser(userName);

    if(user!=null && user.isCreateReports() && user.getDirectory()!=null) {

      File userDir = getUserDirectory(user);

      Delivery d = context.getDelivery(user.getLogin(),user.getPassword(),deliveryId);

      //check was imported
      File reportFile = new File(userDir,d.getName()+".rep."+deliveryId);
      if(!fileSys.exists(reportFile)) return;

      PrintStream ps = null;

      try {
        ps = new PrintStream(fileSys.getOutputStream(reportFile,true),true,user.getFileEncoding());
        final PrintStream psFinal = ps;
        MessageFilter filter = new MessageFilter(deliveryId,d.getStartDate(),new Date());
        context.getMessagesStates(user.getLogin(),user.getPassword(),filter,1000,new Visitor<MessageInfo>(){
          public boolean visit(MessageInfo mi) throws AdminException {
            String result="";
            result = mi.getState().toString() + ((mi.getErrorCode())!=null ? (" errCode="+mi.getErrorCode()) : "");
            writeReportLine(psFinal,mi.getAbonent(),mi.getDate(),result);
            return true;
          }
        });
      }
      finally {
        if(ps!=null) try {ps.close();} catch (Exception e){}
        File finReportFile = new File(userDir,d.getName()+".report");
        fileSys.rename(reportFile,finReportFile);
      }
    }
  }

  public File getUserDirectory(User user) throws AdminException {
    String sDir = user.getDirectory();
    if(sDir==null || sDir.length()==0) {
      throw new ContentProviderException("userDirNotFound",user.getLogin(),sDir);
    }

    File userDir = new File(informerBase,sDir);
    if(fileSys.exists(userDir)) return userDir;

    userDir=new File(user.getDirectory());
    if(!fileSys.exists(userDir)) {
      throw new ContentProviderException("userDirNotFound",user.getLogin(),sDir);
    }
    return userDir;
  }


}
