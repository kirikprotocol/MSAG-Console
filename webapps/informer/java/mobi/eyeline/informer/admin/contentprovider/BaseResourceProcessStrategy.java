package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.monitoring.MBean;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.admin.users.UserCPsettings;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.Date;
import java.util.List;

/**
 * User: artem
 * Date: 27.01.11
 */
abstract class BaseResourceProcessStrategy implements ResourceProcessStrategy {

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");

  ContentProviderContext context;
  FileSystem fileSys;
  User user;
  FileResource resource;
  File workDir;
  Address sourceAddr;
  String encoding;
  boolean createReports;
  String host;
  int port;
  UserCPsettings.Protocol protocol;

  private static final String DELIVERY_PROC_ERR = "Delivery's processing";
  private static final String UPLOAD_REPORTS_ERR = "Results uploading";

  protected BaseResourceProcessStrategy(ContentProviderContext context, FileResource resource, ResourceOptions opts) throws AdminException {
    this.context = context;
    this.user = opts.getUser();
    this.resource = resource;
    this.workDir = opts.getWorkDir();
    this.fileSys = context.getFileSystem();
    this.sourceAddr = opts.getSourceAddress();
    this.encoding = opts.getEncoding();
    this.createReports = opts.isCreateReports();
    this.protocol = opts.getProtocol();
    this.host = opts.getHost();
    this.port = opts.getPort() == null ? 0 : opts.getPort();

    if (!fileSys.exists(workDir))
      fileSys.mkdirs(workDir);
  }

  protected abstract void fileProccesed(FileResource resource, String remoteFile) throws AdminException;

  protected abstract void deliveryCreationError(FileResource resource, Exception e, String remoteFile);

  protected abstract void deliveryFinished(FileResource resource, String remoteFile) throws AdminException;


  private static MBean getMBean() {
    return MBean.getInstance(MBean.Source.CONTENT_PROVIDER);
  }

  @Override
  public final void process(boolean allowDeliveryCreation) {

    if (allowDeliveryCreation) {
      try{
        downloadFiles();
      }catch (AdminException e) {
        getMBean().notifyInteractionError(createResourceUrl(), e.getMessage(), "type", "downloading");
        log.error(e,e);
      }
    }

    File[] files = fileSys.listFiles(workDir);
    if (files == null) {
      log.error("Error reading dir: " + workDir.getAbsolutePath());
      return;
    }

    for (File f : files) {
      try{
        if (f.getName().endsWith(".csv")) {
          try {
            createDeliveryFromFile(f);
            try {
              resource.open();
              fileProccesed(resource, f.getName());
            }catch (AdminException e){
              log.error(e,e);
              getMBean().notifyInteractionError(createResourceUrl(), e.getMessage(), "type", "file proccesed");
            } finally {
              resource.close();
            }
          } catch (DeliveryException e) {
            log.error(e,e);
            getMBean().notifyInternalError(DELIVERY_PROC_ERR+" file="+f.getName(), "Can't process delivery for user="+user.getLogin());
            try{
              fileSys.delete(f);
            }catch (AdminException ignored){}
            if(e.getErrorStatus() != DeliveryException.ErrorStatus.ServiceOffline) {   // informer is offline ?
              deliveryCreationError(resource, e, f.getName());                         // other problems, notify cp
            }
          } catch (Exception e){
            log.error(e,e);
            getMBean().notifyInternalError(DELIVERY_PROC_ERR+" file="+f.getName(), "Can't process delivery for user="+user.getLogin());
            try{
              fileSys.delete(f);
            }catch (AdminException ignored){}
            deliveryCreationError(resource, e, f.getName());                           // other problems, notify cp
          }
        } else if (f.getName().endsWith(".wait")) {
          uploadDeliveryResults(f);
        }
      }catch (Exception e) {
        log.error(e,e);
        getMBean().notifyInternalError(DELIVERY_PROC_ERR, "Unknown error"+e.getMessage(), "type", "unknown");
      }
    }

    for(File f : files) {
      try{
        if(f.getName().endsWith(".not.generated")) {       // haven't analogs on resource
          cleanNotGenerated(f);
        } else if(f.getName().endsWith(".gen")) {          // haven't analogs on resource
          cleanGenerated(f);
        }
      }catch (Exception e){
        log.error(e,e);
      }
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
      }
    } finally {
      try {
        resource.close();
      } catch (AdminException ignored) {
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
        } catch (IOException ignored) {
        }
    }
  }

  private DeliveryPrototype createDelivery(String deliveryName) throws AdminException {
    log("create empty delivery: " + deliveryName);
    DeliveryPrototype delivery = new DeliveryPrototype();
    delivery.setStartDate(new Date());
    context.copyUserSettingsToDeliveryPrototype(user.getLogin(), delivery);
    delivery.setSourceAddress(sourceAddr);
    delivery.setName(deliveryName);
    delivery.setProperty(UserDataConsts.CP_DELIVERY, "true");
    return delivery;
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

  private File lookupGenFile(final String deliveryName, final File parentDir) {
    return lookupFile(parentDir, new FileFilter() {
      @Override
      public boolean accept(File pathname) {
        String name = pathname.getName();
        return name.startsWith(deliveryName + ".csv.") && name.endsWith(".gen");
      }
    });
  }

  private File lookupFile(File parentDir, FileFilter filter) {
    File[] fs = fileSys.listFiles(parentDir, filter);
    if(fs.length>0) {
      return fs[0];
    }
    return null;
  }

  private static File buildNotGeneratedFile(File parent, String deliveryName) {
    return new File(parent, deliveryName+".not.generated");
  }

  private static File buildGeneratedFile(File parent, String deliveryName, int dId) {
    return new File(parent, deliveryName+".csv."+dId+".gen");
  }

  private static File buildWaitFile(File parent, String deliveryName, int dId) {
    return new File(parent, deliveryName+".csv."+dId+".wait");
  }
  private static File buildRepFile(File parent, String deliveryName, int dId) {
    return new File(parent, deliveryName + ".rep." + dId);
  }

  private void silentRename(File from, File to) throws AdminException{
    fileSys.delete(to);
    fileSys.rename(from, to);
  }

  private void createDeliveryFromFile(File f) throws Exception {
    String fileName = f.getName();
    final String deliveryName = fileName.substring(0, fileName.length() - 4);

    Delivery d = getExistingDelivery(deliveryName);

    File parentDir = f.getParentFile();

    File genFile = lookupGenFile(deliveryName, parentDir);

    File waitFile, reportFile;

    if(d != null && fileSys.exists(genFile)) {          // gen file exist, only activation is neeed

      context.activateDelivery(user.getLogin(), d.getId());
      waitFile = buildWaitFile(parentDir, deliveryName, d.getId());
      fileSys.rename(genFile, waitFile);

      if(!createReports) {
        reportFile = buildRepFile(parentDir, deliveryName, d.getId());
        fileSys.delete(reportFile);
      }
      fileSys.delete(f);

    }else {

      if(d != null) {
        context.dropDelivery(user.getLogin(), d.getId());
      }

      File notGenerated = buildNotGeneratedFile(parentDir, deliveryName);

      silentRename(f, notGenerated);

      DeliveryPrototype proto = createDelivery(deliveryName);
      try{
        d = context.createDeliveryWithIndividualTexts(user.getLogin(), proto, null);
      }catch (Exception e){
        log.error(e,e);
        try{
          fileSys.delete(notGenerated);
        }catch (Exception ignored){}
        throw e;
      }

      waitFile =  buildWaitFile(parentDir, deliveryName, d.getId());
      genFile = buildGeneratedFile(parentDir, deliveryName, d.getId());
      reportFile = buildRepFile(parentDir, deliveryName, d.getId());

      fileSys.delete(reportFile);

      try{

        addMessagesToDelivery(d, notGenerated, reportFile);

        silentRename(notGenerated, genFile);

        context.activateDelivery(user.getLogin(), d.getId());

        silentRename(genFile, waitFile);

        if(!createReports) {
          fileSys.delete(reportFile);
        }

      }catch (Exception e) {
        log.error(e,e);
        if(!(e instanceof DeliveryException) || (((DeliveryException)e).getErrorStatus() != DeliveryException.ErrorStatus.ServiceOffline)) {
          try{
            context.dropDelivery(user.getLogin(), d.getId());
            try{
              fileSys.delete(notGenerated);
            }catch (AdminException ignored){}
            try{
              fileSys.delete(genFile);
            }catch (AdminException ignored){}
            try{
              fileSys.delete(waitFile);
            }catch (AdminException ignored){}
            try{
              fileSys.delete(reportFile);
            }catch (AdminException ignored){}
          }catch (Exception ex){
            log.error(ex,ex);
          }
        }
        throw e;
      }
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
        } catch (IOException ignored) {
        }
    }

  }

  private void cleanNotGenerated(File f) {
    log("not generated file: " + f.getName());

    String deliveryName = f.getName().substring(0, f.getName().length() - ".not.generated".length());

    try{
      Delivery d = getExistingDelivery(deliveryName);
      if(d != null) {
        context.dropDelivery(user.getLogin(), d.getId());
        fileSys.delete(buildRepFile(f.getParentFile(), deliveryName, d.getId()));
      }
      fileSys.delete(f);
      log("not generated file: " + f.getName()+" is cleaned");
    }catch (AdminException e){
      log.error(e,e);
    } finally {
      try {
        fileSys.delete(f);
      } catch (AdminException ignored) {}
    }
  }
  private void cleanGenerated(File f) {
    log("generated file: " + f.getName());

    String name = f.getName().substring(0, f.getName().length() - ".gen".length());
    Integer deliveryId = Integer.parseInt(name.substring(name.lastIndexOf('.') + 1));
    String deliveryName = name.substring(0, name.indexOf('.'));

    try{
      Delivery d = context.getDelivery(user.getLogin(), deliveryId);
      if(d != null) {
        context.dropDelivery(user.getLogin(), d.getId());
      }
      fileSys.delete(buildRepFile(f.getParentFile(), deliveryName, deliveryId));
      fileSys.delete(f);
      log("generated file: " + f.getName()+" is cleaned");
    }catch (AdminException e){
      log.error(e,e);
    }
  }


  private String createResourceUrl() {
    return protocol+"://"+host+':'+port;
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
          try{
            uploadFile(resource, reportTmpFile, deliveryName + ".csv.rep.part");
            resource.rename(deliveryName + ".csv.rep.part", deliveryName + ".csv.report");
          }catch (AdminException e){
            log.error(e, e);
            getMBean().notifyInteractionError(createResourceUrl(), "Can't upload results for delivery="+deliveryName+" user="+user.getLogin(), "type","uploading");
            return;
          }

          fileSys.delete(reportTmpFile);
        } else {
          log("no report needed for: " + f.getName());
        }
        try{
          deliveryFinished(resource, d.getName() + ".csv");
        }catch (AdminException e){
          log.error(e, e);
          getMBean().notifyInteractionError(createResourceUrl(), "Can't finalize delivery="+deliveryName+" user="+user.getLogin(), "type","finalizing");
          return;
        }

        fileSys.delete(f);
      } else {
        log("delivery in status " + d.getStatus() + " for: " + f.getName());
      }
    } catch (Exception e) {
      log.error(e, e);
      getMBean().notifyInternalError(UPLOAD_REPORTS_ERR, "Can't upload results for delivery="+deliveryName+" user="+user.getLogin(), "type", "unknown");
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
          ReportFormatter.writeReportLine(psFinal, mi.getAbonent(), mi.getProperty("udata"), new Date(), mi.getState(), mi.getErrorCode());
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

  private Delivery getExistingDelivery(final String name) throws AdminException {
    DeliveryFilter filter = new DeliveryFilter();
    filter.setNameFilter(name);
    final Delivery[] infos = new Delivery[1];
    context.getDeliveries(user.getLogin(), filter,
        new Visitor<mobi.eyeline.informer.admin.delivery.Delivery>() {
          public boolean visit(Delivery value) throws AdminException {
            if (value.getProperty(UserDataConsts.CP_DELIVERY) != null) {
              infos[0] = value;
              return false;
            }
            return true;
          }
        });
    return infos[0];
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
        throw new IllegalArgumentException("INVALID LINE FORMAT");

      String abonent = line.substring(0, inx).trim();
      Address ab;
      try {
        ab = new Address(abonent);
      } catch (Exception e) {
        throw new IllegalArgumentException("INVALID ABONENT");
      }

      String text, userData = null;
      int nextInx = line.indexOf('|', inx + 1);
      if (nextInx < 0)
        text = line.substring(inx + 1).trim();
      else {
        userData = line.substring(inx + 1, nextInx);
        text = line.substring(nextInx + 1).trim();
      }

      Message m = Message.newMessage(ab, decodeText(text));
      if (userData != null)
        m.setProperty("udata", userData);
      return m;
    }

    private String decodeText(String text) {
      StringBuilder sb = new StringBuilder();
      boolean screeningCharBefore = false;
      for (char c : text.toCharArray()) {

        if (screeningCharBefore) {
          if (c == '\\')
            sb.append('\\');
          else if (c == 'n')
            sb.append('\n');
          else
            sb.append('\\').append(c);

          screeningCharBefore = false;
        } else {
          if (c == '\\')
            screeningCharBefore = true;
          else
            sb.append(c);
        }
      }

      return sb.toString();
    }

    private boolean isRegionAllowed(Address addr) {
      if (regions == null)
        return true;
      Region r = context.getRegion(addr);
      return (r != null && regions.contains(r.getRegionId()));
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

            if (!isRegionAllowed(m.getAbonent())) {
              ReportFormatter.writeReportLine(reportWriter, m.getAbonent(), m.getProperty("udata"), new Date(), MessageState.Failed, 9999);
              continue;
            }

            return m;
          } catch (IllegalArgumentException e) {
            log.error("Error parse line in imported file. Line='" + line + "'. Line will be skipped.",e);
          } catch (Exception e) {
            log.error("Error parse line in imported file. Line='" + line + "'. Line will be skipped.",e);
          }
        }
      } catch (IOException ioe) {
        throw new ContentProviderException("ioerror", ioe);
      }
      return null;
    }
  }
}
