package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.UserDataConsts;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.monitoring.MBean;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.MD5Encoder;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;

/**
 * author: Aleksandr Khalitov
 */
class SaveStrategyHelper {

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");
  private static final MBean mbean = MBean.getInstance(MBean.Source.CONTENT_PROVIDER);

  private final ContentProviderContext context;

  private final FileSystem fileSys;

  private final User user;

  private final ResourceOptions resourceOpts;

  SaveStrategyHelper(ContentProviderContext context, User user, ResourceOptions opts) {
    this.context = context;
    this.fileSys = context.getFileSystem();
    this.user = user;
    this.resourceOpts = opts;
  }

  private String getResourceHost() {
    if(resourceOpts.getProtocol() == null) {
      return "";
    }
    switch (resourceOpts.getProtocol()) {
      case file :
      case localFtp: return resourceOpts.getProtocol()+"://"+resourceOpts.getPath();
      default: return resourceOpts.getProtocol()+"://"+resourceOpts.getHost();
    }
  }


  void notifyInternalError(String errorId, String message, String ... atts) {
    mbean.notifyInternalError(errorId, message, atts);
  }

  void notifyInteractionError(String message, String ... attrs) {
    mbean.notifyInteractionError(getResourceHost(), resourceOpts.getPort(), message, attrs);
  }

  void notifyInteractionOk(String ... attrs) {
    mbean.notifyInteractionOk(getResourceHost(), resourceOpts.getPort(), attrs);
  }

  private String checkAllLinesContainsSameText(FileFormatStrategy formatStrategy, File f, String encoding) throws AdminException {
    BufferedReader is = null;
    try {
      is = new BufferedReader(new InputStreamReader(fileSys.getInputStream(f), encoding));
      String line;
      String first = null;
      while ((line = is.readLine()) != null) {
        FileFormatStrategy.MessageInfo info;
        try {
          info = formatStrategy.parseLine(line);
        } catch (IllegalArgumentException e) {
          log.error("Error parse line in imported file. Line='" + line + "'. Line will be skipped.",e);
          continue;
        }

        if(first == null) {
          first = info.getText();
        } else if(!first.equals(info.getText())) {
          return null;
        }
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

  List<File> listFiles(File localCopy, final String posfix) {
    File[] fs =  fileSys.listFiles(localCopy, new FileFilter() {
      @Override
      public boolean accept(File pathname) {
        return pathname.getName().endsWith(posfix);
      }
    });
    return fs == null ? Collections.<File>emptyList() : Arrays.asList(fs);
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

  void createDelivery(FileFormatStrategy formatStrategy, File f, String deliveryName, Address sourceAddr, String encoding, String md5, RejectListener rejectListener) throws AdminException {
    if (encoding == null)
      encoding="UTF-8";

    String sigleText = checkAllLinesContainsSameText(formatStrategy, f, encoding);

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

    strategy.addMessages(formatStrategy, user, d, f, encoding, rejectListener);

    context.activateDelivery(user.getLogin(), d.getId());
  }

  private static final String MD5_PROPERTY = "cp-md5";

  public String getMD5Checksum(File file) throws IOException,AdminException {
    InputStream is = null;
    try {
      is = fileSys.getInputStream(file);
      return MD5Encoder.calcMD5Checksum(is);
    } finally {
      if (is != null)
        is.close();
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

  Delivery lookupDelivery(String name, final String md5) throws AdminException {
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

  void downloadFileFromResource(FileResource res, String remoteFile, File toFile) throws AdminException {
    File partFile = new File(toFile.getParent(), toFile.getName() + ".part");

    if (fileSys.exists(partFile))
      fileSys.delete(partFile);

    OutputStream os = null;
    try {
      os = fileSys.getOutputStream(partFile, false);

      res.get(remoteFile, os);
    } finally {
      if (os != null)
        try {
          os.close();
        } catch (IOException ignored) {
        }
    }

    fileSys.rename(partFile, toFile);
  }

  void delete(File file) throws AdminException {
    if(log.isDebugEnabled()) {
      log.debug("Delete local file: "+file.getAbsolutePath());
    }
    fileSys.delete(file);
  }

  void rename(File file, File to) throws AdminException {
    if(log.isDebugEnabled()) {
      log.debug("Rename local file: "+file.getAbsolutePath()+" to "+to.getAbsolutePath());
    }
    fileSys.rename(file, to);
  }

  void copy(File src, File dst) throws AdminException {
    if(log.isDebugEnabled()) {
      log.debug("Copy local file: "+src.getAbsolutePath()+" to "+dst.getAbsolutePath());
    }
    fileSys.rename(src, dst);
  }

  File[] listFiles(File dir) {
    return fileSys.listFiles(dir);
  }

  File[] listFiles(File dir, FileFilter filter) {
    return fileSys.listFiles(dir, filter);
  }

  boolean isDirectory(File file) {
    return fileSys.isDirectory(file);
  }

  void createNewFile(File file) throws AdminException {
    fileSys.createNewFile(file);
  }

  boolean isEmpty(File file) throws AdminException {
    return fileSys.length(file) == 0;
  }

  boolean exists(File file) throws AdminException {
    return fileSys.exists(file);
  }

  void mkdirs(File file) throws AdminException {
    fileSys.mkdirs(file);
  }

  OutputStream getOutputStream(File f, boolean append) throws AdminException{
    return fileSys.getOutputStream(f, append);
  }

  void uploadFileToResource(FileResource resource, File file, Collection<String> remoteFilesCache) throws AdminException {
    String partFile = file.getName() + ".part";
    if(remoteFilesCache.contains(partFile))
      resource.remove(partFile);

    InputStream is = null;
    try {
      is = fileSys.getInputStream(file);
      resource.put(is, partFile);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (IOException ignored) {
        }
    }

    resource.rename(partFile, file.getName());
  }



  private static final String ADDR_PREF = "addr_prfx";

  private static final String eight = "8";
  private static final String seven = "7";

  static String getMessageRecipient(Message mi) {
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


  class FileMessageSource implements DataSource<Message> {


    private final BufferedReader reader;
    final List<Integer> regions;
    final RejectListener rejectListener;
    private final boolean loadTextFromFile;

    private final FileFormatStrategy formatStrategy;

    public FileMessageSource(FileFormatStrategy formatStrategy, List<Integer> allowedRegions, File messagesFile, String encoding, RejectListener rejectListener, boolean singleText) throws AdminException, UnsupportedEncodingException {
      this.regions = allowedRegions;
      this.reader = new BufferedReader(new InputStreamReader(fileSys.getInputStream(messagesFile), encoding));
      this.rejectListener = rejectListener;
      this.loadTextFromFile = !singleText;
      this.formatStrategy = formatStrategy;
    }

    void close()  {
      if (reader != null)
        try {
          reader.close();
        } catch (IOException ignored) {}
    }

    private void setCpAbonent(Message mi, String abonent) {
      if(abonent.startsWith(eight)) {
        mi.setProperty(ADDR_PREF, eight);
      }else if(abonent.startsWith(seven)) {
        mi.setProperty(ADDR_PREF, seven);
      }
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
            FileFormatStrategy.MessageInfo info = formatStrategy.parseLine(line);

//            String addr = lineData[0];
//            String text = lineData[1];
//            String userData = lineData[2];
//            String keywords = lineData[3];

            Address ab;
            try {
              ab = new Address(info.getMsisdn());
            } catch (Exception e) {
              rejectListener.reject(info.getMsisdn(), info.getUserData());
              continue;
            }

            if (!isRegionAllowed(ab)) {
              if (rejectListener != null)
                rejectListener.reject(info.getMsisdn(), info.getUserData());
              continue;
            }

            Message m = Message.newMessage(ab, loadTextFromFile ? decodeText(info.getText()) : null);
            if(!info.getMsisdn().equals(ab.getSimpleAddress())) {
              setCpAbonent(m, info.getMsisdn());
            }

            if (info.getUserData() != null)
              m.setProperty("udata", info.getUserData());

            m.setKeywords(info.getKeyword());

            return m;
          } catch (Exception e) {
            log.error("Error parse line in imported file. Line='" + line + "'. Line will be skipped.", e);
          }
        }
      } catch (IOException ioe) {
        throw new ContentProviderException("ioerror", ioe);
      }
      return null;
    }
  }


  private class InvidualStrategy implements CreateStrategy {
    @Override
    public Delivery createDelivery(DeliveryPrototype proto) throws AdminException {
      return context.createDeliveryWithIndividualTexts(user.getLogin(), proto, null);
    }

    public void addMessages(FileFormatStrategy formatStrategy, User user, Delivery d, File messagesFile, String encoding, RejectListener rejectListener) throws AdminException {
      FileMessageSource src = null;
      try {
        src = new FileMessageSource(formatStrategy, user.isAllRegionsAllowed() ? null : user.getRegions(), messagesFile, encoding, rejectListener, false);
        context.addMessages(user.getLogin(),src,d.getId());
      } catch (UnsupportedEncodingException ignored) {
      } finally {
        if (src != null)
          src.close();
      }
    }
  }

  private class SingleStrategy implements CreateStrategy {
    @Override
    public Delivery createDelivery(DeliveryPrototype proto) throws AdminException {
      return context.createDeliveryWithSingleTextWithData(user.getLogin(), proto, null);
    }

    public void addMessages(FileFormatStrategy formatStrategy, User user, Delivery d, File messagesFile, String encoding,  RejectListener rejectListener) throws AdminException {
      FileMessageSource src = null;
      try {
        src = new FileMessageSource(formatStrategy, user.isAllRegionsAllowed() ? null : user.getRegions(), messagesFile, encoding, rejectListener, true);
        context.addSingleMessagesWithData(user.getLogin(), src, d.getId());
      } catch (UnsupportedEncodingException ignored) {
      } finally {
        if (src != null)
          src.close();
      }
    }
  }


  private static interface CreateStrategy {

    Delivery createDelivery(DeliveryPrototype proto) throws AdminException;

    public void addMessages(FileFormatStrategy formatStrategy, User user, Delivery d, File messagesFile, String encoding, RejectListener rejectListener) throws AdminException;

  }

  public interface RejectListener {

    void reject(String abonent, String userData);

  }



  void logStartProcess(boolean allowCreate) {
    if(log.isDebugEnabled()) {
      log.debug("Start to process resource: "+getResourceHost()+":"+resourceOpts.getPort()+" allowCreate="+allowCreate);
    }
  }

  void logEndProcess() {
    if(log.isDebugEnabled()) {
      log.debug("Finish to process resource: "+getResourceHost()+":"+resourceOpts.getPort());
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

  void logRenameOnResource(String from, String to) {
    if(log.isDebugEnabled()) {
      log.debug("Rename file on resource: '"+from+"' => '"+to+'\'');
    }
  }

  void logFileNotFoundOnResource(String file) {
    log.error("File is absent on resource '"+file+"'. Cleanup local files...");
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

}
