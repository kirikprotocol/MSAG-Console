package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.HashSet;
import java.util.Set;

/**
 * author: Aleksandr Khalitov
 */
class SimpleSaveStrategy implements ResourceProcessStrategy{

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");

  private final ContentProviderContext context;
  private final FileSystem fileSys;
  private final User user;
  private final FileResource resource;
  private final Address sourceAddr;
  private final String encoding;


  private static final String DELIVERY_PROC_ERR = "Delivery's processing";


  private final File localCopy;

  private final SaveStrategyHelper helper;

  private static final String CSV_POSFIX  = ".csv";


  SimpleSaveStrategy(ContentProviderContext context, FileResource resource, ResourceOptions opts) throws AdminException {
    this.context = context;
    this.user = opts.getUser();
    this.resource = resource;
    File workDir = opts.getWorkDir();
    this.fileSys = context.getFileSystem();
    this.sourceAddr = opts.getSourceAddress();
    this.encoding = opts.getEncoding();

    if (!fileSys.exists(workDir))
      fileSys.mkdirs(workDir);

    localCopy = new File(workDir, "simpleLocalCopy");
    helper = new SaveStrategyHelper(context, fileSys, user, opts);
  }


  private Set<String> getExcludeFiles() {
    Set<String> existed = new HashSet<String>();
    for(File f : helper.getFiles(localCopy, CSV_POSFIX)) {
      existed.add(f.getName());
    }
    return existed;
  }


  void synchronize(boolean allowDownloadNew) throws AdminException{
    if(!fileSys.exists(localCopy)) {
      fileSys.mkdirs(localCopy);
    }


    Set<String> exclude = getExcludeFiles();

    try{
      resource.open();
      if(allowDownloadNew) {
        exclude.addAll(
            helper.downloadNewFiles(resource, exclude, localCopy)
        );
      }
      for(String r : resource.listCSVFiles()) {
        if(exclude.contains(r)) {
          helper.logRemoveFromResource(r);
          resource.remove(r);
        }
      }
      helper.notifyInteractionOk("type", "synchronization");
    }catch (AdminException e){
      helper.notifyInteractionError("Can't synchronize", "type","synchronization");
      throw e;
    } finally {
      try{
        resource.close();
      }catch (Exception ignored){}
    }
  }


  public void process(boolean allowDeliveryCreation) throws AdminException{

    try{
      helper.logStartProcess(allowDeliveryCreation);

      synchronize(allowDeliveryCreation);

      if(allowDeliveryCreation) {
        for(File f : helper.getFiles(localCopy, CSV_POSFIX)) {
          try{
            helper.logProcessFile(f.getName());
            processFile(f);
          }catch (Exception e){
            helper.notifyInternalError(DELIVERY_PROC_ERR + " file=" + f.getName(), "Can't process delivery for user=" + user.getLogin());
            log.error(e,e);
          }
        }
      }
    }finally {
      helper.logEndProcess();
    }
  }


  private void processFile(File f) throws Exception {
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
      helper.logCreateDelivery(deliveryName);
      helper.createDelivery(f, deliveryName, sourceAddr, encoding, md5, null);
    }
    fileSys.delete(f);
  }

}
