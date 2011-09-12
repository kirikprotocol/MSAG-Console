package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.PrintStream;
import java.io.UnsupportedEncodingException;
import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
class DetailedSaveStrategy implements ResourceProcessStrategy{

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private final ContentProviderContext context;
  private final FileSystem fileSys;
  private final User user;
  private final FileResource resource;
  private final Address sourceAddr;
  private String encoding;


  private static final String DELIVERY_PROC_ERR = "Delivery's processing";


  private final File localCopy;

  private final SaveStrategyHelper helper;

  private final boolean createReports;

  private final long maxTimeMillis;

  DetailedSaveStrategy(ContentProviderContext context, FileResource resource, ResourceOptions opts) {
    this.maxTimeMillis = 1000 * opts.getMaxTimeSec();
    this.context = context;
    this.user = opts.getUser();
    this.resource = resource;
    this.fileSys = context.getFileSystem();
    this.sourceAddr = opts.getSourceAddress();
    this.encoding = opts.getEncoding();
    if (encoding == null)
      encoding = "UTF-8";
    this.createReports = opts.isCreateReports();

    localCopy = new File(opts.getWorkDir(), "detailedLocalCopy");

    helper = new SaveStrategyHelper(context, fileSys, user, opts);
  }


  private static final String CSV_POSFIX = ".csv";
  private static final String IN_PROCESS = ".active";
  private static final String FINISHED = ".finished";
  private static final String REPORT = ".report";

  private static String buildFinished(String csv) {
    return csv + FINISHED;
  }

  private static String buildInProcess(String csv) {
    return csv + IN_PROCESS;
  }

  private static String buildReportName(String csv) {
    return csv + REPORT;
  }

  private static String csvFileFromWorkFile(String name) {
    return name.substring(0, name.lastIndexOf(CSV_POSFIX)+CSV_POSFIX.length());
  }

  private void downloadFilesFromResource(FileResource resource) throws AdminException {
    List<String> remoteFiles = new ArrayList<String>(resource.listFiles());
    Collections.shuffle(remoteFiles);

    long start = System.currentTimeMillis();
    for (String remoteCsvFile : remoteFiles) {
      if(!remoteCsvFile.endsWith(".csv")) {
        continue;
      }
      File localCsvFile = new File(localCopy, remoteCsvFile);
      File localInProcessFile = new File(localCopy, buildInProcess(remoteCsvFile));
      File localFinishedFile = new File(localCopy, buildFinished(remoteCsvFile));

      if (fileSys.exists(localCsvFile) || fileSys.exists(localInProcessFile) || fileSys.exists(localFinishedFile))
        continue;

      helper.downloadFileFromResource(resource, remoteCsvFile, localCsvFile);
      long time = System.currentTimeMillis() - start;
      if(time >= maxTimeMillis) {
        if(log.isDebugEnabled()) {
          log.debug("Downloading timeout reached. Stop it.");
        }
        break;
      }
    }
  }

  private void synchronizeInProcess(FileResource resource) throws AdminException {
    Collection<String> remoteFiles = new HashSet<String>(resource.listFiles());
    List<File> inProcessFiles = helper.getFiles(localCopy, IN_PROCESS);
    for (File localInProcessFile : inProcessFiles) {
      String csvFile = csvFileFromWorkFile(localInProcessFile.getName());
      if (remoteFiles.contains(csvFile)) {
        resource.rename(csvFile, localInProcessFile.getName());
      } else if (!remoteFiles.contains(localInProcessFile.getName())){
        fileSys.delete(localInProcessFile);
        File localReportFile = new File(localCopy, buildReportName(csvFile));
        if(fileSys.exists(localReportFile)) {
          fileSys.delete(localReportFile);
        }
      }
    }
  }

  private void synchronizeFinished(FileResource resource) throws AdminException {
    Collection<String> remoteFiles = new HashSet<String>(resource.listFiles());
    List<File> finishedFiles = helper.getFiles(localCopy, FINISHED);
    Collections.shuffle(finishedFiles);
    long start = System.currentTimeMillis();
    for(File localFinishedFile : finishedFiles) {
      if (remoteFiles.contains(localFinishedFile.getName()))
        continue;

      String localCsvFile = csvFileFromWorkFile(localFinishedFile.getName());
      File localReportFile = new File(localCopy, buildReportName(localCsvFile));
      String localInProcessFile = buildInProcess(localCsvFile);

      if(remoteFiles.contains(localInProcessFile)) {
        if(fileSys.exists(localReportFile) && !remoteFiles.contains(localReportFile.getName()))
          helper.uploadFileToResource(resource, localReportFile, remoteFiles);
        resource.rename(localInProcessFile, localFinishedFile.getName());

        long time = System.currentTimeMillis() - start;
        if(time >= maxTimeMillis) {
          if(log.isDebugEnabled()) {
            log.debug("Uploading results timeout reached. Stop it.");
          }
          break;
        }

      } else {
        fileSys.delete(localReportFile);
        fileSys.delete(localFinishedFile);
      }
    }
  }

  void synchronize(boolean downloadNewFiles) throws AdminException{
    if(!fileSys.exists(localCopy))
      fileSys.mkdirs(localCopy);

    try{
      resource.open();

      synchronizeFinished(resource);

      if(downloadNewFiles)
        downloadFilesFromResource(resource);

      synchronizeInProcess(resource);

      helper.notifyInteractionOk("type","synchronization");

    }catch (AdminException e){
      helper.notifyInteractionError("Can't synchronize", "type","synchronization");
      throw e;
    }finally {
      try{
        resource.close();
      }catch (Exception ignored){}
    }
  }

  private FileFormatStrategy getFormatStrategy() {
    if(context.getCpFileFormat() == CpFileFormat.MTS) {
      return new MtsFileFormatStrategy();
    }else {
      return new EyelineFileFormatStrategy();
    }
  }


  public void process(boolean allowCreate) throws AdminException {
    try{
      helper.logStartProcess(allowCreate);

      synchronize(allowCreate);

      File[] list = fileSys.listFiles(localCopy);
      if (list == null)
        return;

      List<File> fs = Arrays.asList(list);

      Collections.shuffle(fs);

      FileFormatStrategy formatStrategy = getFormatStrategy();

      long start = System.currentTimeMillis();
      for(File f : fs) {
        try{
          long time = System.currentTimeMillis() - start;
          if(allowCreate && f.getName().endsWith(CSV_POSFIX)) {
            helper.logProcessFile(f.getName());
            processCsvFile(formatStrategy, f);
            if(time >= maxTimeMillis) {
              if(log.isDebugEnabled()) {
                log.debug("Processing timeout reached. Stop it.");
              }
              break;
            }
          }else if(f.getName().endsWith(IN_PROCESS)) {
            helper.logProcessFile(f.getName());
            processInProcessFile(formatStrategy, f);
            if(time >= maxTimeMillis) {
              if(log.isDebugEnabled()) {
                log.debug("Processing timeout reached. Stop it.");
              }
              break;
            }
          }
        } catch (Exception e){
          helper.notifyInternalError(DELIVERY_PROC_ERR+" file="+f.getName(), "Can't process delivery for user="+user.getLogin());
          log.error(e,e);
        }
      }

      synchronize(false);
    }finally {
      helper.logEndProcess();
    }
  }

  private void processInProcessFile(FileFormatStrategy formatStrategy, File localInProcessFile) throws Exception {
    final String md5 = helper.getMD5Checksum(localInProcessFile);
    String deliveryName = SaveStrategyHelper.getDeliveryName(localInProcessFile);
    Delivery d = helper.lookupDelivery(deliveryName, md5);

    if(d == null || d.getStatus() != DeliveryStatus.Finished)
      return;

    helper.logFinishDelivery(deliveryName);

    String localCsvFile = csvFileFromWorkFile(localInProcessFile.getName());
    if(createReports)
      buildReport(formatStrategy, new File(localCopy, buildReportName(localCsvFile)), d);

    File localFinishedFile = new File(localCopy, buildFinished(localCsvFile));
    fileSys.rename(localInProcessFile, localFinishedFile);
  }


  private void processCsvFile(FileFormatStrategy formatStrategy, File localCsvFile) throws Exception {
    final String md5 = helper.getMD5Checksum(localCsvFile);
    String deliveryName = SaveStrategyHelper.getDeliveryName(localCsvFile);
    Delivery d = helper.lookupDelivery(deliveryName, md5);

    if (d != null && d.getStatus() == DeliveryStatus.Paused) {
      helper.logDropBrokenDelivery(deliveryName);
      context.dropDelivery(user.getLogin(), d.getId());
      d = null;
    }

    if(d == null) {
      File reportFile = createReports ? new File(localCopy, buildReportName(localCsvFile.getName())) : null;
      try{
        createDelivery(formatStrategy, deliveryName, md5, localCsvFile, reportFile);
      }catch (AdminException e) {
        if(reportFile != null) {
          try{
            if(fileSys.exists(reportFile)) {
              fileSys.delete(reportFile);
            }
          }catch (Exception ignored) {}
        }
        throw e;
      }
    }

    fileSys.rename(localCsvFile, new File(localCopy, buildInProcess(localCsvFile.getName())));
  }

  private void createDelivery(final FileFormatStrategy formatStrategy, String deliveryName, final String md5, File localCsvFile, File reportFile) throws AdminException {
    helper.logCreateDelivery(deliveryName);

    if (reportFile != null) {
      final PrintStream[] ps = new PrintStream[1];
      try{
        ps[0] = new PrintStream(fileSys.getOutputStream(reportFile, false), true, encoding);
        helper.createDelivery(formatStrategy, localCsvFile, deliveryName, sourceAddr, encoding, md5, new SaveStrategyHelper.RejectListener() {
          @Override
          public void reject(String abonent, String userData) {
            formatStrategy.writeReportLine(ps[0], abonent, userData, new Date(), MessageState.Failed, 9999);
          }
        });
      } catch (UnsupportedEncodingException e) {
        log.error(e,e);
      } finally {
        if(ps[0]!=null) {
          ps[0].close();
        }
      }
    } else {
      helper.createDelivery(formatStrategy, localCsvFile, deliveryName, sourceAddr, encoding, md5, null);
    }
  }

  private void buildReport(final FileFormatStrategy formatStrategy, File reportFile, Delivery d) throws AdminException, UnsupportedEncodingException {
    File reportTmp = new File(reportFile.getAbsolutePath()+".tmp."+System.currentTimeMillis());
    try{
      fileSys.copy(reportFile, reportTmp);

      PrintStream ps = null;
      try {
        if (encoding == null) encoding = "UTF-8";
        ps = new PrintStream(context.getFileSystem().getOutputStream(reportTmp, true), true, encoding);
        final PrintStream psFinal = ps;
        MessageFilter filter = new MessageFilter(d.getId(), d.getCreateDate(), new Date());
        context.getMessagesStates(user.getLogin(), filter, 1000, new Visitor<Message>() {
          public boolean visit(Message mi) throws AdminException {
            formatStrategy.writeReportLine(psFinal, SaveStrategyHelper.getMessageRecipient(mi),
                mi.getProperty("udata"), new Date(), mi.getState(), mi.getErrorCode());
            return true;
          }
        });
      } catch (UnsupportedEncodingException ignored) {
      } finally {
        if (ps != null) try {
          ps.close();
        } catch (Exception ignored) {
        }
      }

      fileSys.delete(reportFile);
      fileSys.rename(reportTmp, reportFile);

    } catch (AdminException e){
      try{
        fileSys.delete(reportTmp);
      }catch (AdminException ignored){}
      throw e;
    }
  }


}