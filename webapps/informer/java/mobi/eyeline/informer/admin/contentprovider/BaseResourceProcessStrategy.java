package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.Date;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * User: artem
 * Date: 27.01.11
 */
abstract class BaseResourceProcessStrategy implements ResourceProcessStrategy {

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private final Pattern unfinishedFileName = Pattern.compile("\\.csv\\.\\d+$");

  ContentProviderContext context;
  FileSystem fileSys;
  User user;
  FileResource resource;
  File workDir;
  Address sourceAddr;
  String encoding;
  boolean createReports;

  protected BaseResourceProcessStrategy(ContentProviderContext context, FileResource resource, ResourceOptions opts) throws AdminException {
    this.context = context;
    this.user = opts.getUser();
    this.resource = resource;
    this.workDir = opts.getWorkDir();
    this.fileSys = context.getFileSystem();
    this.sourceAddr = opts.getSourceAddress();
    this.encoding = opts.getEncoding();
    this.createReports = opts.isCreateReports();

    if (!fileSys.exists(workDir))
      fileSys.mkdirs(workDir);

    restoreBrokenFiles();
  }

  private void restoreBrokenFiles() {
    File[] files = fileSys.listFiles(workDir);
    if (files == null)
      return;

    for (File f : files) {

      Matcher unfinishedMatcher = unfinishedFileName.matcher(f.getName());
      if (!unfinishedMatcher.matches())
        continue;

      log("restoring file: " + f.getName() + " ...");

      String ext = unfinishedMatcher.group();
      String baseName = f.getName().substring(0, f.getName().length() - ext.length());
      Integer deliveryId;
      try {
        deliveryId = Integer.valueOf(ext.substring(5));
        try {
          context.dropDelivery(user.getLogin(), deliveryId);
        } catch (AdminException e) {
          log.error(e,e);
        }

        //rename .csv.<id> to .csv
        File newFile = new File(workDir, baseName + ".csv");
        if (fileSys.exists(newFile)) {
          fileSys.delete(f);
        } else {
          fileSys.rename(f, newFile);
        }

        File reportFile = new File(workDir, baseName + ".rep." + deliveryId);
        if (fileSys.exists(reportFile)) {
          fileSys.delete(reportFile);
        }

      } catch (Exception e) {
        log.error(e, e);
      }

      log("restore finished: " + f.getName());
    }
  }

  protected abstract void fileDownloaded(FileResource resource, String remoteFile) throws AdminException;

  protected abstract void deliveryCreationError(FileResource resource, Exception e, String remoteFile);

  protected abstract void deliveryFinished(FileResource resource, String remoteFile) throws AdminException;

  @Override
  public final void process(boolean allowDeliveryCreation) throws AdminException {

    if (allowDeliveryCreation)
      downloadFiles();

    File[] files = fileSys.listFiles(workDir);
    if (files == null) {
      log.error("Error reading dir: " + workDir.getAbsolutePath());
      return;
    }

    for (File f : files) {
      if (f.getName().endsWith(".csv"))
        createDeliveryFromFile(f);

      else if (f.getName().endsWith(".wait"))
        uploadDeliveryResults(f);
    }
  }

  private void log(String text) {
    if (log.isDebugEnabled())
      log.debug(resource.toString() + ": " + text);
  }

  private void downloadFiles() throws AdminException {
    log("downloading csv files ...");
    try {
      resource.open();

      List<String> remoteFiles = resource.listCSVFiles();
      for (String remoteFile : remoteFiles) {

        File localTmpFile = new File(workDir, remoteFile + ".tmp");
        if (fileSys.exists(localTmpFile))
          fileSys.delete(localTmpFile);

        downloadFile(resource, remoteFile, localTmpFile);

        fileSys.rename(localTmpFile, new File(workDir, remoteFile));

        fileDownloaded(resource, remoteFile);
      }
    } finally {
      try {
        resource.close();
      } catch (AdminException e) {
      }
    }
    log("downloaded.");
  }

  private void downloadFile(FileResource res, String remoteFile, File toFile) throws AdminException {
    OutputStream os = null;
    try {
      os = fileSys.getOutputStream(toFile, false);

      res.get(remoteFile, os);
    } finally {
      if (os != null)
        try {
          os.close();
        } catch (IOException e) {
        }
    }
  }

  private Delivery createEmptyDelivery(String fileName) throws AdminException {
    log("create empty delivery: " + fileName);
    DeliveryPrototype delivery = new DeliveryPrototype();
    delivery.setStartDate(new Date(System.currentTimeMillis() + 60000));
    context.copyUserSettingsToDeliveryPrototype(user.getLogin(), delivery);
    delivery.setSourceAddress(sourceAddr);
    String deliveryName = fileName.substring(0, fileName.length() - 4);
    delivery.setName(deliveryName);
    Delivery d = context.createDeliveryWithIndividualTexts(user.getLogin(), delivery, null);
    context.pauseDelivery(user.getLogin(), d.getId());
    log("delivery created. id=" + d.getId());
    return d;
  }

  private void addMessagesToDelivery(Delivery d, File f, File reportFile) throws AdminException, UnsupportedEncodingException {
    log("add messages to delivery: id=" + d.getId() + "; file=" + f.getName() + " ...");
    BufferedReader is = null;
    PrintStream reportWriter = null;
    try {
      if (encoding == null) encoding = "UTF-8";
      is = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f), encoding));
      reportWriter = new PrintStream(fileSys.getOutputStream(reportFile, true), true, encoding);
      context.addMessages(user.getLogin(), new CPMessageSource(
          user.isAllRegionsAllowed() ? null : user.getRegions(),
          is,
          reportWriter
      ), d.getId());
    } finally {
      if (is != null) try {
        is.close();
      } catch (Exception ignored) {
      }
      if (reportWriter != null) try {
        reportWriter.close();
      } catch (Exception ignored) {
      }
    }
    log("all messages added.");
  }

  private void deliveryCreationError(Exception e, String remoteFile) {
    try {
      try {
        resource.open();

        deliveryCreationError(resource, e, remoteFile);

      } finally {
        resource.close();
      }
    } catch (Exception ex) {
      log.error(ex,ex);
    }
  }

  private void createDeliveryFromFile(File f) {
    String fileName = f.getName();
    Delivery d;
    try {
      d = createEmptyDelivery(fileName);
    } catch (Exception e) {
      log.error(e,e);
      deliveryCreationError(e, f.getName());
      try {
        fileSys.delete(f);
      } catch (AdminException ignored) {
      }
      return;
    }

    String deliveryName = d.getName();
    Integer deliveryId = d.getId();

    File newFile = new File(f.getParentFile(), deliveryName + ".csv." + deliveryId);
    File reportFile = new File(f.getParentFile(), deliveryName + ".rep." + deliveryId);
    try {
      //rename to .csv.<id>
      fileSys.rename(f, newFile);

      addMessagesToDelivery(d, newFile, reportFile);

      context.activateDelivery(user.getLogin(), deliveryId);

      fileSys.rename(newFile, new File(newFile.getParentFile(), newFile.getName() + ".wait"));

      log("delivery created from: " + f.getName());

      if (!createReports)
        try {
          fileSys.delete(reportFile);
        } catch (AdminException ignored) {
        }

    } catch (Exception e) {
      log.error(e,e);

      try {
        context.dropDelivery(user.getLogin(), deliveryId);
      } catch (AdminException ignored) {
      }
      try {
        fileSys.delete(newFile);
      } catch (AdminException ignored) {
      }
      try {
        fileSys.delete(f);
      } catch (AdminException ignored) {
      }
      try {
        fileSys.delete(reportFile);
      } catch (AdminException ignored) {
      }

      deliveryCreationError(e, f.getName());
    }
  }

  protected void uploadFile(FileResource resource, File file, String toFile) throws AdminException {
    InputStream is = null;
    try {
      is = fileSys.getInputStream(file);
      resource.put(is, toFile);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException e) {
        }
    }

  }

  private void uploadDeliveryResults(File f) {
    log("wait file: " + f.getName());

    String name = f.getName().substring(0, f.getName().length() - ".wait".length());
    Integer deliveryId = Integer.parseInt(name.substring(name.lastIndexOf('.') + 1));
    String deliveryName = name.substring(0, name.indexOf('.'));

    try {
      Delivery d = context.getDelivery(user.getLogin(), deliveryId);
      if (d == null) {
        log("Delivery not found for: " + f.getName() + ". id=" + deliveryId);
        return;
      }

      if (d.getStatus() == DeliveryStatus.Finished) {
        log("delivery finished for: " + f.getName());

        resource.open();

        if (createReports) {
          log("create report for: " + f.getName());
          File reportTmpFile = new File(f.getParent(), deliveryName + ".rep." + deliveryId);
          createReport(reportTmpFile, d);
          uploadFile(resource, reportTmpFile, deliveryName + ".csv.report");
          fileSys.delete(reportTmpFile);
        } else {
          log("no report needed for: " + f.getName());
        }

        deliveryFinished(resource, d.getName() + ".csv");

        fileSys.delete(f);
      } else {
        log("delivery in status " + d.getStatus() + " for: " + f.getName());
      }
    } catch (Exception e) {
      log.error(e, e);
    } finally {
        try {
          resource.close();
        } catch (AdminException ignored) {
        }
    }
  }

  private void createReport(File reportFile, Delivery d) throws AdminException, UnsupportedEncodingException {

    PrintStream ps = null;
    try {
      if (encoding == null) encoding = "UTF-8";

      ps = new PrintStream(context.getFileSystem().getOutputStream(reportFile, true), true, encoding);
      final PrintStream psFinal = ps;
      MessageFilter filter = new MessageFilter(d.getId(), d.getStartDate(), new Date());
      context.getMessagesStates(user.getLogin(), filter, 1000, new Visitor<Message>() {
        public boolean visit(Message mi) throws AdminException {
          String result = "";
          String userData = mi.getProperty("userData");
          if (userData != null)
            result = userData + '|';
          result += mi.getState().toString() + '|' + ((mi.getErrorCode()) != null ? (mi.getErrorCode()) : "0");
          ReportFormatter.writeReportLine(psFinal, mi.getAbonent(), mi.getDate(), result);
          return true;
        }
      });

    } finally {
      if (ps != null) try {
        ps.close();
      } catch (Exception ignored) {
      }
    }
  }

  private class CPMessageSource implements DataSource<Message> {
    private BufferedReader reader;
    List<Integer> regions;
    PrintStream reportWriter;

    public CPMessageSource(List<Integer> regions, BufferedReader reader, PrintStream reportWriter) {
      this.regions = regions;
      this.reader = reader;
      this.reportWriter = reportWriter;
    }

    /**
     * Десериализует сообщение из строки
     *
     * @param line строка
     * @return сообщение или null, если строка не содержит сообщения
     * @throws IllegalArgumentException если строка имеет некорректный формат
     */
    private Message parseLine(String line) {
      int inx = line.indexOf('|');
      if (inx < 0)
        return null;

      String abonent = line.substring(0, inx).trim();
      Address ab;
      try {
        ab = new Address(abonent);
      } catch (Exception e) {
        throw new IllegalArgumentException("INVALID ABONENT");
      }

      if (regions != null) {
        Region r = context.getRegion(ab);
        if (r == null || !regions.contains(r.getRegionId()))
          throw new IllegalArgumentException("NOT ALLOWED REGION");
      }

      String text, userData = null;
      int nextInx = line.indexOf('|', inx + 1);
      if (nextInx < 0)
        text = line.substring(inx + 1).trim();
      else {
        userData = line.substring(inx + 1, nextInx);
        text = line.substring(nextInx + 1).trim();
      }

      Message m = Message.newMessage(ab, text);
      if (userData != null)
        m.setProperty("userData", userData);
      return m;
    }


    public Message next() throws AdminException {
      try {
        String line;
        while ((line = reader.readLine()) != null) {
          line = line.trim();
          if (line.length() == 0) continue;
          try {
            Message m = parseLine(line);
            if (m == null)
              continue;
            return m;
          } catch (IllegalArgumentException e) {
            ReportFormatter.writeReportLine(reportWriter, line, new Date(), e.getMessage());
          } catch (Exception e) {
            ReportFormatter.writeReportLine(reportWriter, line, new Date(), "ERROR PARSING LINE :" + line);
          }
        }
      } catch (IOException ioe) {
        throw new ContentProviderException("ioerror", ioe);
      }
      return null;
    }
  }
}
