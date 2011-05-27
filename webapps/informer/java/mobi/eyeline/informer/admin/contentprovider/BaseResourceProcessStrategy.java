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

  private ContentProviderContext context;
  private FileSystem fileSys;
  private User user;
  private FileResource resource;
  private File workDir;
  private Address sourceAddr;
  private String encoding;
  private boolean createReports;
  private String host;
  private Integer port;
  private UserCPsettings.Protocol protocol;
  private String path;

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
    this.port = opts.getPort();
    path = opts.getPath();

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
      downloadForProcess();
    }

    File[] files = fileSys.listFiles(workDir);
    if (files == null) {
      log.error("Error reading dir: " + workDir.getAbsolutePath());
      return;
    }

    for (File f : files) {
      try{
        if (isFileNew(f)) {
          processNewFile(f);
        } else if (isFileForFinish(f)) {
          finishDelivery(f);
        }
      }catch (Exception e) {
        log.error(e,e);
        getMBean().notifyInternalError(DELIVERY_PROC_ERR, "Unknown error"+e.getMessage(), "type", "unknown");
      }
    }

    for(File f : files) {
      try{
        if(isCrashedBeforeGen(f)) {       // haven't analogs on resource (server crashed on create->file is removed on resource->server started)
          cleanNotGenerated(f);
        } else if(isCrashedAfterGen(f)) { // haven't analogs on resource (server crashed on create->file is removed on resource->server started)
          cleanGenerated(f);
        }
      }catch (Exception e){
        log.error(e,e);
      }
    }
  }

  private void downloadForProcess() {
    try{
      downloadFiles();
      getMBean().notifyInteractionOk(getResourceHost(), port, "type", "downloading");
    }catch (AdminException e) {
      getMBean().notifyInteractionError(getResourceHost(), port, e.getMessage(), "type", "downloading");
      log.error(e,e);
    }
  }

  private void processNewFile(File f) {
    try {
      createDeliveryFromFile(f);
      try {
        openResource();
        fileProccesed(resource, f.getName());
        getMBean().notifyInteractionOk(getResourceHost(), port,"type", "file proccesed");
      }catch (AdminException e){
        log.error(e,e);
        getMBean().notifyInteractionError(getResourceHost(), port, e.getMessage(), "type", "file proccesed");
      } finally {
        closeResource();
      }
    } catch (DeliveryException e) {
      log.error(e,e);
      getMBean().notifyInternalError(DELIVERY_PROC_ERR+" file="+f.getName(), "Can't process delivery for user="+user.getLogin());
      silentDelete(f);
      if(e.getErrorStatus() != DeliveryException.ErrorStatus.ServiceOffline) {   // informer is offline ?
        deliveryCreationError(resource, e, f.getName());                         // other problems, notify cp
      }
    } catch (Exception e){
      log.error(e,e);
      getMBean().notifyInternalError(DELIVERY_PROC_ERR+" file="+f.getName(), "Can't process delivery for user="+user.getLogin());
      silentDelete(f);
      deliveryCreationError(resource, e, f.getName());                           // other problems, notify cp
    }
  }

  private void log(String text) {
    if (log.isDebugEnabled())
      log.debug(resource.toString() + ": " + text);
  }

  private void downloadFiles() throws AdminException {
    log("downloading csv files ...");
    try {
      openResource();

      List<String> remoteFiles = resource.listCSVFiles();
      for (String remoteFile : remoteFiles) {

        File localTmpFile = new File(workDir, remoteFile + ".tmp");
        if (fileSys.exists(localTmpFile))
          fileSys.delete(localTmpFile);

        downloadFile(resource, remoteFile, localTmpFile);

        fileSys.rename(localTmpFile, new File(workDir, remoteFile));
      }
    } finally {
      closeResource();
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

  private DeliveryPrototype createProto(String deliveryName) throws AdminException {
    log("create empty delivery: " + deliveryName);
    DeliveryPrototype delivery = new DeliveryPrototype();
    delivery.setStartDate(new Date());
    context.copyUserSettingsToDeliveryPrototype(user.getLogin(), delivery);
    delivery.setSourceAddress(sourceAddr);
    delivery.setName(deliveryName);
    delivery.setProperty(UserDataConsts.CP_DELIVERY, "true");
    return delivery;
  }

  private void openResource() throws AdminException {
    resource.open();
  }
  private void closeResource() {
    try {
      resource.close();
    } catch (AdminException ignored) {}
  }


  private String isSingleText(File f) throws AdminException {
    BufferedReader is = null;
    try {
      if (encoding == null) encoding = "UTF-8";
      is = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f), encoding));
      String line;
      String first = null;
      while ((line = is.readLine()) != null) {
        try{
          String[] data = parseData(line);
          if(first == null) {
            first = data[1];
          }else {
            if(!first.equals(data[1])) {
              if(log.isDebugEnabled()) {
                log.debug("File contains multitext messages: "+f.getName());
              }
              return null;
            }
          }

        } catch (IllegalArgumentException e) {
          log.error("Error parse line in imported file. Line='" + line + "'. Line will be skipped.",e);
        }
      }
      if(log.isDebugEnabled()) {
        log.debug("File contains singletext messages: "+f.getName());
      }
      return first;
    } catch (IOException ioe) {
      throw new ContentProviderException("ioerror", ioe);
    } finally {
      if (is != null) try {
        is.close();
      } catch (Exception ignored) {
      }
    }

  }

  private static String[] parseData(String line) {
    int inx = line.indexOf('|');
    if (inx < 0)
      throw new IllegalArgumentException("INVALID LINE FORMAT");

    String abonent = line.substring(0, inx).trim();

    String text, userData = null;
    int nextInx = line.indexOf('|', inx + 1);
    if (nextInx < 0)
      text = line.substring(inx + 1).trim();
    else {
      userData = line.substring(inx + 1, nextInx);
      text = line.substring(nextInx + 1).trim();
    }

    return new String[]{abonent, text, userData};
  }




  private void addMessagesToDelivery(Delivery d, File f, File reportFile, CreateStrategy strategy) throws AdminException, UnsupportedEncodingException {
    log("add messages to delivery: id=" + d.getId() + "; file=" + f.getName() + " ...");
    BufferedReader is = null;
    PrintStream reportWriter = null;
    try {
      if (encoding == null) encoding = "UTF-8";
      is = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f), encoding));
      reportWriter = new PrintStream(fileSys.getOutputStream(reportFile, true), true, encoding);
      strategy.addMessages(user, d, is, reportWriter);
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
    if(fs != null && fs.length>0) {
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

  private void activateAfterCrash(File existedGenFile, File fileFromResource, Delivery d)  throws AdminException{
    File parentDir = fileFromResource.getParentFile();
    context.activateDelivery(user.getLogin(), d.getId());
    File waitFile = buildWaitFile(parentDir, d.getName(), d.getId());
    fileSys.rename(existedGenFile, waitFile);

    if(!createReports) {
      File reportFile = buildRepFile(parentDir, d.getName(), d.getId());
      fileSys.delete(reportFile);
    }
    fileSys.delete(fileFromResource);
  }

  private void createNewDelivery(String deliveryName, Delivery d, File fromResource)  throws Exception{

    if(d != null) {
      context.dropDelivery(user.getLogin(), d.getId());
    }

    File parentDir = fromResource.getParentFile();

    File notGenerated = buildNotGeneratedFile(parentDir, deliveryName);

    silentRename(fromResource, notGenerated);

    String sigleText = isSingleText(notGenerated);

    DeliveryPrototype proto = createProto(deliveryName);

    CreateStrategy strategy;
    if(sigleText == null) {
      strategy = new InvidualStrategy();
    }else {
      proto.setSingleText(decodeText(sigleText));
      strategy = new SingleStrategy();
    }

    try{
      d = strategy.createDelivery(user,  proto);
    }catch (Exception e){
      log.error(e,e);
      silentDelete(notGenerated);
      throw e;
    }

    File waitFile =  buildWaitFile(parentDir, deliveryName, d.getId());
    File genFile = buildGeneratedFile(parentDir, deliveryName, d.getId());
    File reportFile = buildRepFile(parentDir, deliveryName, d.getId());

    fileSys.delete(reportFile);

    try{

      addMessagesToDelivery(d, notGenerated, reportFile, strategy);

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
          silentDelete(notGenerated, genFile, waitFile, reportFile);
        }catch (Exception ex){
          log.error(ex,ex);
        }
      }
      throw e;
    }
  }

  private void createDeliveryFromFile(File f) throws Exception {
    String fileName = f.getName();
    final String deliveryName = fileName.substring(0, fileName.length() - 4);

    Delivery d = getExistingDelivery(deliveryName);

    File genFile = lookupGenFile(deliveryName, f.getParentFile());

    if(d != null && genFile != null && fileSys.exists(genFile)) {   // gen file exist, only activation is neeed
      activateAfterCrash(genFile, f, d);
    }else {
      createNewDelivery(deliveryName, d, f);
    }
  }

  private void silentDelete(File ... fs) {
    if(fs != null) {
      try{
        for(File f : fs) {
          fileSys.delete(f);
        }
      }catch (AdminException ignored){}
    }
  }

  private static boolean isFileNew(File f) {
    return f != null && f.getName().endsWith(".csv");
  }

  private static boolean isFileForFinish(File f) {
    return f != null && f.getName().endsWith(".wait");
  }

  private static boolean isCrashedBeforeGen(File f) {
    return f != null && f.getName().endsWith(".not.generated");
  }

  private static boolean isCrashedAfterGen(File f) {
    return f != null && f.getName().endsWith(".gen");
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
      silentDelete(f);
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


  private String getResourceHost() {
    if(protocol == null) {
      return "";
    }
    switch (protocol) {
      case file :
      case localFtp: return protocol+"://"+path;
      default: return protocol+"://"+host;
    }
  }

  private boolean uploadReport(File f, String deliveryName) {
    try{
      uploadFile(resource, f, deliveryName + ".csv.rep.part");
      resource.rename(deliveryName + ".csv.rep.part", deliveryName + ".csv.report");
      getMBean().notifyInteractionOk(getResourceHost(), port, "type","uploading");
      return true;
    }catch (AdminException e){
      log.error(e, e);
      getMBean().notifyInteractionError(getResourceHost(), port, "Can't upload results for delivery="+deliveryName+" user="+user.getLogin(), "type","uploading");
      return false;
    }
  }
  private boolean _finishDelivery(String deliveryName) {
    try{
      deliveryFinished(resource, deliveryName + ".csv");
      getMBean().notifyInteractionOk(getResourceHost(), port, "type","finalizing");
      return true;
    }catch (AdminException e){
      log.error(e, e);
      getMBean().notifyInteractionError(getResourceHost(), port, "Can't finalize delivery="+deliveryName+" user="+user.getLogin(), "type","finalizing");
      return false;
    }
  }

  private static String getDeliveryNameFromWait(File f) {
    String name = f.getName().substring(0, f.getName().length() - ".wait".length());
    name = name.substring(0, name.lastIndexOf('.'));
    return name.substring(0, name.lastIndexOf('.'));
  }

  private static int getDeliveryIdFromWait(File f) {
    String name = f.getName().substring(0, f.getName().length() - ".wait".length());
    return Integer.parseInt(name.substring(name.lastIndexOf('.') + 1));
  }

  private void finishDelivery(File f) {
    log("wait file: " + f.getName());

    int deliveryId = getDeliveryIdFromWait(f);
    String deliveryName = getDeliveryNameFromWait(f);

    try {
      Delivery d = context.getDelivery(user.getLogin(), deliveryId);
      if (d == null) {
        log("Delivery not found for: " + f.getName() + ". id=" + deliveryId);
        return;
      }

      if (d.getStatus() == DeliveryStatus.Finished) {
        log("delivery finished for: " + f.getName());

        File reportTmpFile = null;
        if (createReports) {
          log("create report for: " + f.getName());
          reportTmpFile = buildRepFile(f.getParentFile(), deliveryName , deliveryId);
          buildReport(reportTmpFile, d);
        }
        try{
          openResource();
          if(reportTmpFile != null) {
            if(!uploadReport(reportTmpFile, deliveryName)) {
              return;
            }
            fileSys.delete(reportTmpFile);
          }
          if(!_finishDelivery(deliveryName)) {
            return;
          }
        }finally {
          closeResource();
        }
        fileSys.delete(f);
      } else {
        log("delivery in status " + d.getStatus() + " for: " + f.getName());
      }
    } catch (Exception e) {
      log.error(e, e);
      getMBean().notifyInternalError(UPLOAD_REPORTS_ERR, "Can't upload results for delivery="+deliveryName+" user="+user.getLogin(), "type", "unknown");
    }
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
        MessageFilter filter = new MessageFilter(d.getId(), d.getStartDate(), new Date());
        context.getMessagesStates(user.getLogin(), filter, 1000, new Visitor<Message>() {
          public boolean visit(Message mi) throws AdminException {
            ReportFormatter.writeReportLine(psFinal, getCpAbonent(mi), mi.getProperty("udata"), new Date(), mi.getState(), mi.getErrorCode());
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

  private Delivery getExistingDelivery(final String name) throws AdminException {
    DeliveryFilter filter = new DeliveryFilter();
    filter.setNameFilter(name);
    final Delivery[] infos = new Delivery[1];
    context.getDeliveries(user.getLogin(), filter,
        new Visitor<mobi.eyeline.informer.admin.delivery.Delivery>() {
          public boolean visit(Delivery value) throws AdminException {
            if (value.getName().equals(name) && value.getProperty(UserDataConsts.CP_DELIVERY) != null) {
              infos[0] = value;
              return false;
            }
            return true;
          }
        });
    return infos[0];
  }

  private static final String ADDR_PREF = "addr_prfx";

  private static final String eight = "8";
  private static final String seven = "7";

  private static String getCpAbonent(Message mi) {
    String prefix = mi.getProperty(ADDR_PREF);
    String abonent;
    if(prefix != null) {
      if(prefix.endsWith(eight)) {
        abonent = eight+mi.getAbonent().getSimpleAddress().substring(2);
      }else {
        abonent = mi.getAbonent().getSimpleAddress().substring(1);
      }
    }else {
      abonent = mi.getAbonent().getSimpleAddress();
    }
    return abonent;
  }

  private static void setCpAbonent(Message mi, String abonent) {
    if(abonent.startsWith(eight)) {
      mi.setProperty(ADDR_PREF, eight);
    }else if(abonent.startsWith(seven)) {
      mi.setProperty(ADDR_PREF, seven);
    }
  }

  private static String decodeText(String text) {
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

  private class CPMessageSource implements DataSource<Message> {
    private BufferedReader reader;
    List<Integer> regions;
    PrintStream reportWriter;
    private boolean isSingleText;

    public CPMessageSource(List<Integer> regions, BufferedReader reader, PrintStream reportWriter, boolean isSingleText) {
      this.regions = regions;
      this.reader = reader;
      this.reportWriter = reportWriter;
      this.isSingleText = isSingleText;
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

      String userData = null;
      int nextInx = line.indexOf('|', inx + 1);
      if (nextInx >= 0){
        userData = line.substring(inx + 1, nextInx);
      }

      Message m = Message.newMessage(ab, isSingleText ? null :
          decodeText(
              nextInx<0 ? line.substring(inx + 1).trim() : line.substring(nextInx + 1).trim()
          )
      );
      if(!abonent.equals(m.getAbonent().getSimpleAddress())) {
        setCpAbonent(m, abonent);
      }
      if (userData != null)
        m.setProperty("udata", userData);
      return m;
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
              String abonent = getCpAbonent(m);
              ReportFormatter.writeReportLine(reportWriter, abonent, m.getProperty("udata"), new Date(), MessageState.Failed, 9999);
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

  private class InvidualStrategy implements CreateStrategy{
    @Override
    public Delivery createDelivery(User u, DeliveryPrototype proto) throws AdminException {
      return context.createDeliveryWithIndividualTexts(user.getLogin(), proto, null);
    }

    public void addMessages(User user, Delivery d, BufferedReader is, PrintStream reportWriter) throws AdminException {
      context.addMessages(user.getLogin(),
          new CPMessageSource(
              user.isAllRegionsAllowed() ? null : user.getRegions(),
              is,
              reportWriter, false
          ),
          d.getId());
    }
  }

  private class SingleStrategy implements CreateStrategy{
    @Override
    public Delivery createDelivery(User u, DeliveryPrototype proto) throws AdminException {
      return context.createDeliveryWithSingleTextWithData(user.getLogin(), proto, null);
    }

    public void addMessages(User user, Delivery d, BufferedReader is, PrintStream reportWriter) throws AdminException {
      context.addSingleMessagesWithData(user.getLogin(),
          new CPMessageSource(
              user.isAllRegionsAllowed() ? null : user.getRegions(),
              is,
              reportWriter, true
          ),
          d.getId());
    }
  }


  static interface CreateStrategy {

    Delivery createDelivery(User u, DeliveryPrototype proto) throws AdminException;

    public void addMessages(User user, Delivery d, BufferedReader is, PrintStream reportWriter) throws AdminException;

  }
}
