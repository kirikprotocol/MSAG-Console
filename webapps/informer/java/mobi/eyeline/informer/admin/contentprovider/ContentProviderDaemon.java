package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminContext;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.Daemon;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.PrintStream;
import java.io.UnsupportedEncodingException;
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
public class ContentProviderDaemon implements Daemon, DeliveryNotificationsListener {
  Logger log = Logger.getLogger(this.getClass());
  private ScheduledExecutorService scheduler;
  static final String NAME = "ContentProviderDaemon";
  private static final long PERIOD_MSEC = 1000L;
  private static final long SHUTDOWN_WAIT_TIME = 2000L;
  FileSystem fileSys;
  AdminContext context;
  File informerBase;

  public ContentProviderDaemon(AdminContext context, File informerBase, FileSystem fileSys) {
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
        new ContentProviderDaemonTask(context,informerBase,fileSys),
        0,
        PERIOD_MSEC,
        TimeUnit.MILLISECONDS
    );
  }

  public synchronized void stop() throws AdminException {
    scheduler.shutdown();
    try {
      scheduler.awaitTermination(SHUTDOWN_WAIT_TIME, TimeUnit.MILLISECONDS);
    }
    catch (InterruptedException e) {
      scheduler.shutdownNow();
    }
    scheduler = null;
  }

  public boolean isStarted() {
    return scheduler!=null;
  }


  public static void writeReportLine(PrintStream reportWriter, String abonent, Date date, String s) {
    reportWriter.print(abonent);
    reportWriter.print(" | ");
    reportWriter.print(new SimpleDateFormat("yyyy.MM.dd HH:mm:ss").format(date)); // todo вынести SimpleDateFormat в отдельное поле
    reportWriter.print(" | ");
    reportWriter.println(s);
  }

  public void onDeliveryNotification(DeliveryNotification notification) {
    switch (notification.getType()) { // todo Упростить до if
      case DELIVERY_FINISHED:
        try {
          String userId = notification.getUserId();
          User user = context.getUser(userId);
          createReport(notification,user); // todo генерация отчетов может существенно затормозить основной поток. Надо использовать собственный сторадж и поток.
        }
        catch (Exception e) {
          log.error("Error processing delivery finished report for delivery "+notification.getDeliveryId(),e);
        }
        break;
      default:
    }
  }

  private void createReport(DeliveryNotification notification, User user) throws AdminException, UnsupportedEncodingException {
    if(user!=null && user.isCreateReports() && user.getDirectory()!=null) {

      File userDir = new File(informerBase,user.getDirectory()); // todo А если в user.getDirectory() абсолютный путь?
      if(!fileSys.exists(userDir)) throw new ContentProviderException("userDirNotFound",userDir.toString());

      int deliveryId = notification.getDeliveryId();
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
            result = mi.getState().toString() + (mi.getErrorCode())!=null ? (" errCode="+mi.getErrorCode()) : "";
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
}
