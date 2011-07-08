package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.Arrays;
import java.util.Date;
import java.util.HashSet;
import java.util.Set;

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

  DetailedSaveStrategy(ContentProviderContext context, FileResource resource, ResourceOptions opts) {
    this.context = context;
    this.user = opts.getUser();
    this.resource = resource;
    this.fileSys = context.getFileSystem();
    this.sourceAddr = opts.getSourceAddress();
    this.encoding = opts.getEncoding();
    this.createReports = opts.isCreateReports();

    localCopy = new File(opts.getWorkDir(), "detailedLocalCopy");

    helper = new SaveStrategyHelper(context, fileSys, user, opts);
  }


  private static final String CSV_POSFIX = ".csv";
  private static final String IN_PROCESS = ".inprocess";
  private static final String FINISHED = ".finished";
  private static final String REPORT = ".rep";

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

  private static String[] csvFilesFromWork(File[] inProcess) {
    String[] csvFromInProcess = new String[inProcess.length];
    int i=0;
    for(File f : inProcess) {
      csvFromInProcess[i] = csvFileFromWorkFile(f.getName());
      i++;
    }
    return csvFromInProcess;
  }

  private static String[] inProcessFromWork(File[] finished) {
    String[] inProcessFromFinished = new String[finished.length];
    int i=0;
    for(File f : finished) {
      inProcessFromFinished[i] = buildInProcess(
          csvFileFromWorkFile(f.getName())
      );
      i++;
    }
    return inProcessFromFinished;
  }

  private void synchronizeInProcess() throws AdminException {
    File[] inProcess = helper.getFiles(localCopy, IN_PROCESS);
    String[] csvFromInProcess = csvFilesFromWork(inProcess);
    for(int i = 0;i < csvFromInProcess.length;i++) {
      String f = csvFromInProcess[i];

      if(resource.contains(f)) {
        String newFile = buildInProcess(f);
        if(!resource.contains(newFile)) {
          helper.logRenameAtResource(f, newFile);
          resource.rename(f, newFile);
        }
      }else if (!resource.contains(inProcess[i].getName())) {
        fileSys.delete(inProcess[i]);
      }
    }
  }


  private void synchronizeFinished() throws AdminException {
    File[] finished = helper.getFiles(localCopy, FINISHED);
    String[] inProcessFromFinished = inProcessFromWork(finished);
    for(int i = 0;i < inProcessFromFinished.length;i++) {
      String file = inProcessFromFinished[i];
      String csvFile = csvFileFromWorkFile(file);
      String finishedFile = buildFinished(csvFile);
      String reportFile = buildReportName(csvFile);

      if(resource.contains(file)) {
        if(createReports && !resource.contains(reportFile)) {
          String reportTmp = reportFile + ".part";
          if(resource.contains(reportTmp)) {
            helper.logRemoveFromResource(reportTmp);
            resource.remove(reportTmp);
          }
          helper.uploadFile(resource, new File(localCopy, reportFile), reportTmp);
          helper.logRenameAtResource(reportTmp, reportFile);
          resource.rename(reportTmp,  reportFile);
        }
        if(!resource.contains(finishedFile)) {
          helper.logRenameAtResource(file, finishedFile);
          resource.rename(file, finishedFile);
        }
      }else if (!resource.contains(finished[i].getName())) {
        fileSys.delete(new File(localCopy, reportFile));
        fileSys.delete(new File(localCopy, finishedFile));
      }
    }
  }

  private Set<String> getExcludeFiles() {
    Set<String> exclude = new HashSet<String>();
    for(File f : helper.getFiles(localCopy, CSV_POSFIX)) {
      exclude.add(f.getName());
    }

    File[] inProcess = helper.getFiles(localCopy, IN_PROCESS);
    String[] csvFromInProcess = csvFilesFromWork(inProcess);
    exclude.addAll(Arrays.asList(csvFromInProcess));

    File[] finished = helper.getFiles(localCopy, FINISHED);
    for(File f : finished) {
      exclude.add(
          csvFileFromWorkFile(f.getName())
      );
    }
    return exclude;
  }


  void synchronize(boolean allowDeliveryCreation) throws AdminException{
    if(!fileSys.exists(localCopy)) {
      fileSys.mkdirs(localCopy);
    }
    try{
      resource.open();
      if(allowDeliveryCreation) {

        Set<String> exclude = getExcludeFiles();

        helper.downloadNewFiles(resource, exclude, localCopy);

      }

      synchronizeInProcess();

      synchronizeFinished();

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


  public void process(boolean allowCreate) throws AdminException{
    try{
      helper.logStartProcess(allowCreate);
      synchronize(allowCreate);
      File[] list = fileSys.listFiles(localCopy);

      for(File f : list) {
        try{
          if(allowCreate && f.getName().endsWith(CSV_POSFIX)) {
            helper.logProcessFile(f.getName());
            processCsvFile(f);
          }else if(f.getName().endsWith(IN_PROCESS)) {
            helper.logProcessFile(f.getName());
            processInProcessFile(f);
          }
        }catch (Exception e){
          helper.notifyInternalError(DELIVERY_PROC_ERR+" file="+f.getName(), "Can't process delivery for user="+user.getLogin());
          log.error(e,e);
        }
      }
      synchronize(allowCreate);
    }finally {
      helper.logEndProcess();
    }
  }

  private void processInProcessFile(File f) throws Exception {
    final String md5 = helper.getMD5Checksum(f);
    String deliveryName = SaveStrategyHelper.getDeliveryName(f);
    Delivery d = helper.getDelivery(deliveryName, md5);
    if(d != null) {
      if(d.getStatus() == DeliveryStatus.Finished)  {
        helper.logFinishDelivery(deliveryName);
        if(createReports) {
          buildReport(new File(localCopy, buildReportName(csvFileFromWorkFile(f.getName()))), d);
        }
        fileSys.rename(f, new File(localCopy, buildFinished(csvFileFromWorkFile(f.getName()))));
      }
    }
  }


  private void processCsvFile(File f) throws Exception {
    final String md5 = helper.getMD5Checksum(f);
    String deliveryName = SaveStrategyHelper.getDeliveryName(f);
    Delivery d = helper.getDelivery(deliveryName, md5);

    boolean create = false;
    if(d != null) {
      if(d.getStatus() == DeliveryStatus.Paused) {
        helper.logDropBrokenDelivery(deliveryName);
        context.dropDelivery(user.getLogin(), d.getId());
        create = true;
      }
    }else {
      create = true;
    }

    if(create) {
      final PrintStream[] ps = new PrintStream[1];
      try{
        ps[0] = new PrintStream(fileSys.getOutputStream(new File(localCopy, buildReportName(f.getName())), false), true, encoding);
        helper.logCreateDelivery(deliveryName);
        helper.createDelivery(f, deliveryName, sourceAddr, encoding, md5, new SaveStrategyHelper.RejectListener() {
          @Override
          public void reject(String abonent, String userData) {
            ReportFormatter.writeReportLine(ps[0], abonent, userData, new Date(), MessageState.Failed, 9999);
          }
        });
      }finally {
        if(ps[0]!=null) {
          ps[0].close();
        }
      }
    }
    fileSys.rename(f, new File(localCopy, buildInProcess(f.getName())));
  }

  private void copyFile(File source, File destination) throws AdminException {
    BufferedInputStream src = null;
    BufferedOutputStream dest = null;
    try {
      src = new BufferedInputStream(fileSys.getInputStream(source));
      dest = new BufferedOutputStream(fileSys.getOutputStream(destination, false));

      byte[] buf = new byte[1024];
      int len;
      while ((len = src.read(buf)) > 0){
        dest.write(buf, 0, len);
      }
    } catch (IOException e){
      log.error(e,e);
      throw new ContentProviderException("ioerror");
    }finally {
      if (src != null) {
        try {
          src.close();
        } catch (IOException ignored) {
        }
      }
      if (dest != null) {
        try {
          dest.close();
        } catch (IOException ignored) {
        }
      }
    }
  }

  private void buildReport(File reportFile, Delivery d) throws AdminException, UnsupportedEncodingException {
    File reportTmp = new File(reportFile.getAbsolutePath()+".tmp."+System.currentTimeMillis());
    try{
      copyFile(reportFile, reportTmp);
      PrintStream ps = null;
      try {
        if (encoding == null) encoding = "UTF-8";
        ps = new PrintStream(context.getFileSystem().getOutputStream(reportTmp, true), true, encoding);
        final PrintStream psFinal = ps;
        MessageFilter filter = new MessageFilter(d.getId(), d.getCreateDate(), new Date());
        context.getMessagesStates(user.getLogin(), filter, 1000, new Visitor<Message>() {
          public boolean visit(Message mi) throws AdminException {
            ReportFormatter.writeReportLine(psFinal, SaveStrategyHelper.getCpAbonent(mi),
                mi.getProperty("udata"), new Date(), mi.getState(), mi.getErrorCode());
            return true;
          }
        });
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
    } catch (UnsupportedEncodingException e){
      try{
        fileSys.delete(reportTmp);
      }catch (AdminException ignored){}
      throw e;
    }
  }



}
