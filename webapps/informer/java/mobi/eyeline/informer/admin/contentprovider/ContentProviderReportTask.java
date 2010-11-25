package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.MessageFilter;
import mobi.eyeline.informer.admin.delivery.MessageInfo;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.users.User;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.Date;


class ContentProviderReportTask implements Runnable{
  Logger log = Logger.getLogger(this.getClass());
  private ContentProviderUserDirectoryResolver userDirectoryResolver;
  private ContentProviderContext context;
  File workDir;

  public ContentProviderReportTask(ContentProviderUserDirectoryResolver userDirectoryResolver, ContentProviderContext context, File workDir) {
    this.userDirectoryResolver = userDirectoryResolver;
    this.context = context;
    this.workDir = workDir;
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
            createReport(deliveryId, userName);
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

    private void createReport(int deliveryId, String userName) throws AdminException, UnsupportedEncodingException {
      User user = context.getUser(userName);
      if (user != null && user.isCreateReports() && user.getDirectory() != null) {
        File userDir = userDirectoryResolver.getUserDirectory(user);

        Delivery d = context.getDelivery(user.getLogin(), user.getPassword(), deliveryId);

        //check was imported
        File reportFile = new File(userDir, d.getName() + ".rep." + deliveryId);
        if (!context.getFileSystem().exists(reportFile)) return;

        PrintStream ps = null;

        try {
          ps = new PrintStream(context.getFileSystem().getOutputStream(reportFile, true), true, user.getFileEncoding());
          final PrintStream psFinal = ps;
          MessageFilter filter = new MessageFilter(deliveryId, d.getStartDate(), new Date());
          context.getMessagesStates(user.getLogin(), user.getPassword(), filter, 1000, new Visitor<MessageInfo>() {
            public boolean visit(MessageInfo mi) throws AdminException {
              String result = "";
              result = mi.getState().toString() + ((mi.getErrorCode()) != null ? (" errCode=" + mi.getErrorCode()) : "");
              ContentProviderReportFormatter.writeReportLine(psFinal, mi.getAbonent(), mi.getDate(), result);
              return true;
            }
          });
        }
        finally {
          if (ps != null) try {
            ps.close();
          }
          catch (Exception e) {
          }
          File finReportFile = new File(userDir, d.getName() + ".report");
          context.getFileSystem().rename(reportFile, finReportFile);
        }
      }
    }

}