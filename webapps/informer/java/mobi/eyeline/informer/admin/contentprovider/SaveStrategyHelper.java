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
import java.security.MessageDigest;
import java.util.Collection;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;

/**
 * author: Aleksandr Khalitov
 */
class SaveStrategyHelper {

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private final ContentProviderContext context;

  private final FileSystem fileSys;

  private final User user;

  private final String host;

  private final Integer port;

  private final String path;

  private final UserCPsettings.Protocol protocol;

  SaveStrategyHelper(ContentProviderContext context, FileSystem fileSys, User user, ResourceOptions opts) {
    this.context = context;
    this.fileSys = fileSys;
    this.user = user;

    this.protocol = opts.getProtocol();
    this.host = opts.getHost();
    this.port = opts.getPort();
    this.path = opts.getPath();
  }


  private static MBean getMBean() {
    return MBean.getInstance(MBean.Source.CONTENT_PROVIDER);
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


  void notifyInternalError(String errorId, String message, String ... atts) {
    getMBean().notifyInternalError(errorId, message, atts);
  }

  void notifyInteractionError(String message, String ... attrs) {
    getMBean().notifyInteractionError(getResourceHost(), port, message, attrs);

  }

  void notifyInteractionOk(String ... attrs) {
    getMBean().notifyInteractionOk(getResourceHost(), port, attrs);
  }


  private String isSingleText(File f, String encoding) throws AdminException {
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


  static String getDeliveryName(File f) {
    String fileName = f.getName();
    return fileName.substring(0, fileName.lastIndexOf(".csv"));
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

  File[] getFiles(File localCopy, final String posfix) {
    return fileSys.listFiles(localCopy, new FileFilter() {
      @Override
      public boolean accept(File pathname) {
        return pathname.getName().endsWith(posfix);
      }
    });
  }

  private void addMessagesToDelivery(Delivery d, File f, CreateStrategy strategy, String encoding, RejectListener rejectListener) throws AdminException, UnsupportedEncodingException {
    BufferedReader is = null;
    try {
      if (encoding == null) encoding = "UTF-8";
      is = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f), encoding));
      strategy.addMessages(user, d, is, rejectListener);
    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (Exception ignored) {}
      }
    }
  }

  private DeliveryPrototype createProto(String deliveryName, Address sourceAddr) throws AdminException {
    DeliveryPrototype delivery = new DeliveryPrototype();
    delivery.setStartDate(new Date());
    context.copyUserSettingsToDeliveryPrototype(user.getLogin(), delivery);
    delivery.setSourceAddress(sourceAddr);
    delivery.setName(deliveryName);
    delivery.setProperty(UserDataConsts.CP_DELIVERY, "true");
    return delivery;
  }

  void createDelivery(File f, String deliveryName, Address sourceAddr, String encoding, String md5, RejectListener rejectListener) throws Exception{
    String sigleText = isSingleText(f, encoding);
    DeliveryPrototype proto = createProto(deliveryName, sourceAddr);
    proto.setProperty(MD5_PROPERTY, md5);
    CreateStrategy strategy;
    if(sigleText == null) {
      strategy = new InvidualStrategy();
    }else {
      proto.setSingleText(decodeText(sigleText));
      strategy = new SingleStrategy();
    }

    Delivery d = strategy.createDelivery(proto);
    addMessagesToDelivery(d, f, strategy, encoding, rejectListener);
    context.activateDelivery(user.getLogin(), d.getId());
  }

  private static final String MD5_PROPERTY = "cp-md5";

  byte[] createChecksum(File file) throws Exception {
    MessageDigest complete = MessageDigest.getInstance("MD5");
    InputStream is = null;
    try{
      is = fileSys.getInputStream(file);
      byte[] buffer = new byte[1024];
      int numRead;
      do {
        numRead = is.read(buffer);
        if (numRead > 0) {
          complete.update(buffer, 0, numRead);
        }
      } while (numRead != -1);
    }finally{
      if(is != null) {
        try{is.close();
        }catch (Exception ignored) {}
      }
    }
    return complete.digest();
  }

  public String getMD5Checksum(File file) throws Exception {
    byte[] b = createChecksum(file);
    String result = "";
    for (byte aB : b) {
      result +=
          Integer.toString((aB & 0xff) + 0x100, 16).substring(1);
    }
    return result;
  }

  class CPMessageSource implements DataSource<Message> {
    private final BufferedReader reader;
    final List<Integer> regions;
     final RejectListener rejectListener;
    private final boolean isSingleText;

    public CPMessageSource(List<Integer> regions, BufferedReader reader,  RejectListener rejectListener, boolean isSingleText) {
      this.regions = regions;
      this.reader = reader;
      this.rejectListener = rejectListener;
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
              if(rejectListener != null) {
                rejectListener.reject(abonent, m.getProperty("udata"));
              }
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

  private static final String ADDR_PREF = "addr_prfx";

  private static final String eight = "8";
  private static final String seven = "7";

  static String getCpAbonent(Message mi) {
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
        switch (c) {
          case '\\':
            sb.append('\\');
            break;
          case 'n':
            sb.append('\n');
            break;
          default:
            sb.append('\\').append(c);
            break;
        }

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

  Delivery getDelivery(String name, final String md5) throws AdminException {
    DeliveryFilter filter = new DeliveryFilter();
    filter.setNameFilter(name);
    final Delivery[] ds = new Delivery[1];
    context.getDeliveries(user.getLogin(), filter, new Visitor<Delivery>() {
      @Override
      public boolean visit(Delivery value) throws AdminException {
        if(md5.equals(value.getProperty(MD5_PROPERTY))) {
          ds[0] = value;
          return false;
        }
        return true;
      }
    });
    return ds[0];
  }



  void downloadFile(FileResource res, String remoteFile, File toFile) throws AdminException {
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

  Collection<String> downloadNewFiles(FileResource resource, Collection<String> exclude, File localCopy) throws AdminException {
    Collection<String> downloaded = new LinkedList<String>();
    try {
      List<String> remoteFiles = resource.listCSVFiles();
      for (String remoteFile : remoteFiles) {
        if(exclude.contains(remoteFile)) {
          continue;
        }

        File localTmpFile = new File(localCopy, remoteFile + ".tmp");

        if (fileSys.exists(localTmpFile))
          fileSys.delete(localTmpFile);

        downloadFile(resource, remoteFile, localTmpFile);

        File newFile = new File(localCopy, remoteFile);
        fileSys.rename(localTmpFile, newFile);
        logDownloadedFromResource(newFile.getName());
        downloaded.add(newFile.getName());
      }
      notifyInteractionOk("type", "downloading");
      return downloaded;
    } catch (AdminException e){
      notifyInteractionError(e.getMessage(), "type", "downloading");
      throw e;
    }
  }


  void uploadFile(FileResource resource, File file, String toFile) throws AdminException {
    logUploadToResource(toFile);
    InputStream is = null;
    try {
      is = fileSys.getInputStream(file);
      resource.put(is, toFile);
      getMBean().notifyInteractionOk(getResourceHost(), port, "type","uploading");
    } catch (AdminException e){
      getMBean().notifyInteractionError(getResourceHost(), port, "Can't upload file= " + toFile, "type", "uploading");
      throw e;
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }
    }

  }

  class InvidualStrategy implements CreateStrategy{
    @Override
    public Delivery createDelivery(DeliveryPrototype proto) throws AdminException {
      return context.createDeliveryWithIndividualTexts(user.getLogin(), proto, null);
    }

    public void addMessages(User user, Delivery d, BufferedReader is, RejectListener rejectListener) throws AdminException {
      context.addMessages(user.getLogin(),
          new CPMessageSource(
              user.isAllRegionsAllowed() ? null : user.getRegions(),
              is,
              rejectListener, false
          ),
          d.getId());
    }
  }

  class SingleStrategy implements CreateStrategy{
    @Override
    public Delivery createDelivery(DeliveryPrototype proto) throws AdminException {
      return context.createDeliveryWithSingleTextWithData(user.getLogin(), proto, null);
    }

    public void addMessages(User user, Delivery d, BufferedReader is,  RejectListener rejectListener) throws AdminException {
      context.addSingleMessagesWithData(user.getLogin(),
          new CPMessageSource(
              user.isAllRegionsAllowed() ? null : user.getRegions(),
              is,
              rejectListener, true
          ),
          d.getId());
    }
  }


  static interface CreateStrategy {

    Delivery createDelivery(DeliveryPrototype proto) throws AdminException;

    public void addMessages(User user, Delivery d, BufferedReader is, RejectListener rejectListener) throws AdminException;

  }

  static interface RejectListener {

    void reject(String abonent, String userData);

  }



  void logStartProcess(boolean allowCreate) {
    if(log.isDebugEnabled()) {
      log.debug("Start to process resource: "+getResourceHost()+":"+port+" allowCreate="+allowCreate);
    }
  }

  void logEndProcess() {
    if(log.isDebugEnabled()) {
      log.debug("Finish to process resource: "+getResourceHost()+":"+port);
    }
  }

  void logDownloadedFromResource(String fileName) {
    if(log.isDebugEnabled()) {
      log.debug("File downloaded from resource: "+fileName);
    }
  }

  void logProcessFile(String fileName) {
    if(log.isDebugEnabled()) {
      log.debug("Process file: "+fileName);
    }
  }

  void logCreateDelivery(String deliveryName) {
    if(log.isDebugEnabled()) {
      log.debug("Create delivery: "+deliveryName);
    }
  }

  void logFinishDelivery(String deliveryName) {
    if(log.isDebugEnabled()) {
      log.debug("Delivery is finished: "+deliveryName);
    }
  }

  void logDropBrokenDelivery(String deliveryName) {
    if(log.isDebugEnabled()) {
      log.debug("Drop broken delivery: "+deliveryName);
    }
  }

  void logRemoveFromResource(String fileName) {
    if(log.isDebugEnabled()) {
      log.debug("Remove file from resource: "+fileName);
    }
  }

  void logUploadToResource(String fileName) {
    if(log.isDebugEnabled()) {
      log.debug("Upload file to resource: "+fileName);
    }
  }

  void logRenameAtResource(String fileName, String newName) {
    if(log.isDebugEnabled()) {
      log.debug("Rename file at resource: "+fileName+" => "+newName);
    }
  }


}
