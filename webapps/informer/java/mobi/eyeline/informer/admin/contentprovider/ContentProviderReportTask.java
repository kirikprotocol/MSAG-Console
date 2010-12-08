package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.admin.delivery.MessageFilter;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;


class ContentProviderReportTask implements Runnable{
  Logger log = Logger.getLogger(this.getClass());

  private ContentProviderUserDirResolver userDirResolver;
  private ContentProviderContext context;
  File workDir;

  public ContentProviderReportTask(ContentProviderContext context,ContentProviderUserDirResolver userDirResolver, File workDir) {
    this.context = context;
    this.workDir = workDir;
    this.userDirResolver = userDirResolver;
  }


  public void run() {
    File[] files = context.getFileSystem().listFiles(workDir);
    if (files == null) {
      log.error("Error listing of working directory " + workDir.getAbsolutePath());
      return;
    }
    for (File f : files) {
      if (f.getName().endsWith(".notification")) {
        String sId = f.getName().substring(0, f.getName().length() - ".notification".length());
        BufferedReader reader = null;
        try {
          int deliveryId = Integer.valueOf(sId);
          reader = new BufferedReader(new InputStreamReader(context.getFileSystem().getInputStream(f), "utf-8"));
          String userName = reader.readLine().trim();
          handleDeliveryFinalization(deliveryId, userName);
        }
        catch (Exception e) {
          log.error("error processing file " + f.getAbsolutePath());
          try {
            context.getFileSystem().rename(f, new File(workDir, sId + ".err"));
          }
          catch (Exception ex) {
            log.error("unable to rename file to " + sId + ".err", ex);
          }
        }
        finally {
          if (reader != null) try {
            reader.close();
          }
          catch (Exception e) {
          }
          try {
            if (context.getFileSystem().exists(f)) context.getFileSystem().delete(f);
          }
          catch (Exception e) {
            log.error("unable to delete file" + f.getAbsolutePath(), e);
          }
        }
      }
    }
  }

  private void handleDeliveryFinalization(int deliveryId, String userName) throws AdminException, UnsupportedEncodingException {
    User user = context.getUser(userName);
    if (user != null && user.getCpSettings()!=null) {

      Delivery d = context.getDelivery(user.getLogin(), user.getPassword(), deliveryId);

      for(UserCPsettings ucps : user.getCpSettings()) {

        File userDir = userDirResolver.getUserLocalDir(userName,ucps);
        if(userDir.exists()) {

          //check was imported
          File reportFile = new File(userDir, d.getName() + ".rep." + deliveryId);

          if (context.getFileSystem().exists(reportFile)) {
            if (user.isCreateReports()) {
              createReport(deliveryId, reportFile, user, d, ucps);
              File finReportFile = new File(userDir, d.getName() + ".report");
              context.getFileSystem().rename(reportFile, finReportFile);

            } else {
              createFinalizationFile(new File(userDir, d.getName() + ".fin"));
              context.getFileSystem().delete(reportFile);
            }
            return;
          }
        }
      }
    }
  }

  private void createFinalizationFile(File file) throws AdminException {
    OutputStream os = null;
    try {
      os = context.getFileSystem().getOutputStream(file, false);
    } finally {
      if (os != null)
        try {
          os.close();
        } catch (IOException ignored) {}
    }
  }

  private void createReport(int deliveryId, File reportFile, User user, Delivery d, UserCPsettings ucps) throws AdminException, UnsupportedEncodingException {

    PrintStream ps = null;
    try {
      String encoding = ucps.getEncoding();
      if(encoding==null) encoding="UTF-8";

      ps = new PrintStream(context.getFileSystem().getOutputStream(reportFile, true), true, encoding);
      final PrintStream psFinal = ps;
      MessageFilter filter = new MessageFilter(deliveryId, d.getStartDate(), new Date());
      context.getMessagesStates(user.getLogin(), user.getPassword(), filter, 1000, new Visitor<Message>() {
        public boolean visit(Message mi) throws AdminException {
          String result = "";
          result = mi.getState().toString() + ((mi.getErrorCode()) != null ? (" errCode=" + mi.getErrorCode()) : "");
          ContentProviderReportFormatter.writeReportLine(psFinal, mi.getAbonent(), mi.getDate(), result);
          return true;
        }
      });

    } finally {
      if (ps != null) try {
        ps.close();
      } catch (Exception ignored) {}


    }
  }

}