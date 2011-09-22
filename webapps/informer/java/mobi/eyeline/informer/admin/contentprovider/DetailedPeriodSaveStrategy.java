package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileFilter;
import java.io.PrintStream;
import java.io.UnsupportedEncodingException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
public class DetailedPeriodSaveStrategy implements ResourceProcessStrategy{

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private final ContentProviderContext context;
  private final User user;
  private final FileResource resource;
  private final Address sourceAddr;
  private String encoding;


  private static final String DELIVERY_PROC_ERR = "Delivery's processing";


  private final File localCopy;

  private final SaveStrategyHelper helper;

  private final long maxTimeMillis;

  private final long reportTimeoutMillis;

  DetailedPeriodSaveStrategy(ContentProviderContext context, FileResource resource, ResourceOptions opts) throws AdminException {
    this.maxTimeMillis = 1000 * opts.getMaxTimeSec();
    this.reportTimeoutMillis = 60 * 1000 * (opts.getReportTimeoutMin() == null ? 10 : opts.getReportTimeoutMin());
    this.context = context;
    this.user = opts.getUser();
    this.resource = resource;
    this.sourceAddr = opts.getSourceAddress();
    this.encoding = opts.getEncoding();
    if (encoding == null)
      encoding = "UTF-8";
    localCopy = new File(opts.getWorkDir(), "detailedPeriodLocalCopy");

    helper = new SaveStrategyHelper(context, user, opts);

    deleteAllTmpReports();
  }


  private static final String CSV_POSFIX = ".csv";
  private static final String IN_PROCESS = ".active";
  private static final String FINISHED = ".finished";

  private static String buildFinished(String csv) {
    return csv + FINISHED;
  }

  private static String buildInProcess(String csv) {
    return csv + IN_PROCESS;
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

      if (helper.exists(localCsvFile) || helper.exists(localInProcessFile) || helper.exists(localFinishedFile))
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
    List<File> inProcessFiles = helper.listFiles(localCopy, IN_PROCESS);
    for (File localInProcessFile : inProcessFiles) {
      String csvFile = csvFileFromWorkFile(localInProcessFile.getName());
      if (remoteFiles.contains(csvFile)) {
        resource.rename(csvFile, localInProcessFile.getName());
        uploadReports(remoteFiles, csvFile);
      }else if (remoteFiles.contains(localInProcessFile.getName())){
        uploadReports(remoteFiles, csvFile);
      }else {
        helper.delete(localInProcessFile);
        deleteAllReports(csvFile);
      }
    }
  }

  private void deleteAllReports(String localCsvFile) throws AdminException{
    for(File f : getReportInfoFiles(localCsvFile)) {
      helper.delete(f);
    }
    for(File f : getReportFiles(localCsvFile)) {
      helper.delete(f);
    }
  }

  private void deleteAllTmpReports() throws AdminException{
    final String suffix = ".tmp";
    File[] fs = helper.listFiles(localCopy, new FileFilter() {
      @Override
      public boolean accept(File pathname) {
        return !helper.isDirectory(pathname) && pathname.getName().endsWith(suffix);
      }
    });
    if(fs != null) {
      for(File f : fs) {
        helper.delete(f);
      }
    }
  }

  private void uploadReports(Collection<String> remoteFiles, String localCsvFile) throws AdminException {
    List<File> fs = getReportFiles(localCsvFile);
    Collections.sort(fs); //todo check order
    for(File f : fs) {
      if(!remoteFiles.contains(f.getName())) {
        helper.uploadFileToResource(resource, f, remoteFiles);
        //todo remove
      }
    }
  }


  private void synchronizeFinished(FileResource resource) throws AdminException {
    Collection<String> remoteFiles = new HashSet<String>(resource.listFiles());
    List<File> finishedFiles = helper.listFiles(localCopy, FINISHED);
    Collections.shuffle(finishedFiles);
    long start = System.currentTimeMillis();
    for(File localFinishedFile : finishedFiles) {
      if (remoteFiles.contains(localFinishedFile.getName()))
        continue;

      String localCsvFile = csvFileFromWorkFile(localFinishedFile.getName());
      String localInProcessFile = buildInProcess(localCsvFile);

      if(remoteFiles.contains(localInProcessFile)) {
        uploadReports(remoteFiles, localCsvFile);
        resource.rename(localInProcessFile, localFinishedFile.getName());
        long time = System.currentTimeMillis() - start;
        if(time >= maxTimeMillis) {
          if(log.isDebugEnabled()) {
            log.debug("Uploading results timeout reached. Stop it.");
          }
          break;
        }
      } else {
        deleteAllReports(localCsvFile);
        helper.delete(localFinishedFile);
      }
    }
  }

  void synchronize(boolean downloadNewFiles) throws AdminException{
    if(!helper.exists(localCopy))
      helper.mkdirs(localCopy);

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

      File[] list = helper.listFiles(localCopy);
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

  void processInProcessFile(FileFormatStrategy formatStrategy, File localInProcessFile) throws Exception {
    final String md5 = helper.getMD5Checksum(localInProcessFile);
    String deliveryName = SaveStrategyHelper.getDeliveryName(localInProcessFile);
    Delivery d = helper.lookupDelivery(deliveryName, md5);

    if(d == null)
      return;

    Date now = new Date();
    String localCsvFile = csvFileFromWorkFile(localInProcessFile.getName());

    buildReport(formatStrategy, localCsvFile, d, now);

    if(d.getStatus() == DeliveryStatus.Finished) {
      helper.logFinishDelivery(deliveryName);
      File localFinishedFile = new File(localCopy, buildFinished(localCsvFile));
      helper.rename(localInProcessFile, localFinishedFile);
    }
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
      File reportFile = new File(localCopy, localCsvFile.getName()+'.'+reportFileFormat.format(new Date())+".report");
      try{
        createDelivery(formatStrategy, deliveryName, md5, localCsvFile, reportFile);
      }catch (AdminException e) {
        try{
          if(helper.exists(reportFile)) {
            helper.delete(reportFile);
          }
        }catch (Exception ignored) {}
        throw e;
      }
    }

    helper.rename(localCsvFile, new File(localCopy, buildInProcess(localCsvFile.getName())));
  }

  private void createDelivery(final FileFormatStrategy formatStrategy, String deliveryName, final String md5, File localCsvFile, File reportFile) throws AdminException {
    helper.logCreateDelivery(deliveryName);

    Date now = new Date();
    final PrintStream[] ps = new PrintStream[1];
    try{
      ps[0] = new PrintStream(helper.getOutputStream(reportFile, false), true, encoding);
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
    ReportInfo info = new ReportInfo();
    info.date = now;
    buildReportInfo(localCsvFile.getName(), info);
  }

  private List<File> getReportInfoFiles(String localCsvFile) {
    final String reportPrefix = localCsvFile+".period.";
    File[] fs = helper.listFiles(localCopy, new FileFilter() {
      @Override
      public boolean accept(File pathname) {
        return !helper.isDirectory(pathname) && pathname.getName().startsWith(reportPrefix);
      }
    });
    if(fs == null || fs.length == 0) {
      return Collections.emptyList();
    }else {
      return Arrays.asList(fs);
    }
  }

  private List<File> getReportFiles(final String localCsvFile) {
    final String suffix1 = ".report";
    final String suffix2 = ".report.final";
    int reportDateLenght = reportFileFormat.toPattern().length();
    final int lenght1 = localCsvFile.length()+ 1 + reportDateLenght + suffix1.length();
    final int lenght2 = localCsvFile.length()+ 1 + reportDateLenght + suffix2.length();
    File[] fs = helper.listFiles(localCopy, new FileFilter() {
      @Override
      public boolean accept(File pathname) {
        if(helper.isDirectory(pathname)) {
          return false;
        }
        String name = pathname.getName();
        return name.startsWith(localCsvFile) && (name.endsWith(suffix1) || name.endsWith(suffix2)) && (name.length() == lenght1 || name.length() == lenght2);
      }
    });
    if(fs == null || fs.length == 0) {
      return Collections.emptyList();
    }else {
      return Arrays.asList(fs);
    }
  }

  private ReportInfo getLastReportInfo(String localCsvFile) {
    final String reportPrefix = localCsvFile+".period.";
    Comparator<ReportInfo> comparator = new Comparator<ReportInfo>() {
      @Override
      public int compare(ReportInfo o1, ReportInfo o2) {
        return o1.date.compareTo(o2.date);
      }
    };
    ReportInfo res = null;
    int dotPos = reportPrefix.length()-1;
    for(File f : getReportInfoFiles(localCsvFile)) {
      try{
        int i = f.getName().lastIndexOf(".");
        String s = f.getName().substring(reportPrefix.length(), i > dotPos ? i : f.getName().length()) ;

        ReportInfo reportInfo = new ReportInfo();
        reportInfo.date = reportInfoFormat.parse(s);
        if(i>dotPos) {
          s = f.getName().substring(i+1);
          reportInfo.messageId = Long.parseLong(s);
        }

        if(res == null || comparator.compare(res, reportInfo) >= 0) {
          res = reportInfo;
        }
      }catch (ParseException e){
        log.error(e,e);
      }
    }
    return res;
  }

  private static class ReportInfo {
    private Date date;
    private Long messageId;
  }


  private void buildReportInfo(String localCsvFile, ReportInfo info) throws AdminException{
    StringBuilder name = new StringBuilder().append(localCsvFile).append(".period.").append(reportInfoFormat.format(info.date));
    if(info.messageId != null) {
      name.append('.').append(info.messageId);
    }
    helper.createNewFile(new File(localCopy, name.toString()));
  }

  private final SimpleDateFormat reportInfoFormat = new SimpleDateFormat("yyyyMMddHHmmss");
  private final SimpleDateFormat reportFileFormat = new SimpleDateFormat("yyyyMMddHHmm");

  void buildReport(final FileFormatStrategy formatStrategy, String localCsvFile, Delivery d, Date now) throws Exception{

    ReportInfo lastInfo = getLastReportInfo(localCsvFile);

    if(d.getStatus() != DeliveryStatus.Finished) {
      if(System.currentTimeMillis() - lastInfo.date.getTime() < reportTimeoutMillis) {
        return;
      }
    }

    String fName = localCsvFile+'.'+ reportFileFormat.format(now) + ".report";

    if(d.getStatus() == DeliveryStatus.Finished) {
      fName+=".final";
    }

    File report = new File(localCopy, fName);

    File reportTmp = new File(localCopy, report.getName()+".tmp");
    Message lst;
    try{
      lst = writeMessagesReport(formatStrategy, lastInfo == null ? null : lastInfo.messageId,
          reportTmp, d.getId(), lastInfo == null ? d.getCreateDate() : lastInfo.date, now);
      helper.rename(reportTmp, report);
    }catch (Exception e){
      if(helper.exists(reportTmp)) {
        try{
          helper.delete(reportTmp);
        }catch (AdminException ignored){}
      }
      throw e;
    }

    if(lst != null) {
      ReportInfo rI = new ReportInfo();
      rI.messageId = lst.getId();
      rI.date = lst.getDate();

      Collection<File> previous = getReportInfoFiles(localCsvFile);

      buildReportInfo(localCsvFile, rI);

      for(File f : previous) {
        helper.delete(f);
      }
    }
  }

  private Message writeMessagesReport(final FileFormatStrategy formatStrategy, final Long fromtId, File file, int deliveryId, Date from, Date till) throws AdminException {
    if(log.isDebugEnabled()) {
      log.debug("Write report: deliveryId="+deliveryId+" fromId="+fromtId+" from="+from);
    }
    PrintStream ps = null;
    final Message[] lastMessasge = new Message[]{null};
    try {
      if (encoding == null) encoding = "UTF-8";
      ps = new PrintStream(context.getFileSystem().getOutputStream(file, true), true, encoding);
      final PrintStream psFinal = ps;
      MessageFilter filter = new MessageFilter(deliveryId, new Date(from.getTime()-1000), new Date(till.getTime()+1000));
      filter.setStates(MessageState.Failed, MessageState.Expired, MessageState.Delivered);

      final boolean[] startReport = new boolean[]{fromtId == null};
      System.out.println("From: "+filter.getStartDate()+" Till: "+filter.getEndDate());      //todo
      context.getMessagesStates(user.getLogin(), filter, 1000, new Visitor<Message>() {
        public boolean visit(Message mi) throws AdminException {
          if (startReport[0]) {
            formatStrategy.writeReportLine(psFinal, SaveStrategyHelper.getMessageRecipient(mi),
                mi.getProperty("udata"), new Date(), mi.getState(), mi.getErrorCode());
            lastMessasge[0] = mi;
          } else if (mi.getId().equals(fromtId)) {
            startReport[0] = true;
          }
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
    return lastMessasge[0];
  }
}
