package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryPrototype;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.*;

/**
 * User: artem
 * Date: 27.01.11
 */
class DetailedResourceProcessStrategy extends BaseResourceProcessStrategy {

  private static final Logger log = Logger.getLogger("CONTENT_PROVIDER");

  DetailedResourceProcessStrategy(ContentProviderContext context, FileResource resource, ResourceOptions opts) throws AdminException {
    super(context, resource, opts);
  }

  @Override
  protected void fileDownloaded(FileResource resource, String remoteFile) throws AdminException {
    resource.rename(remoteFile, remoteFile + ".active");
  }

  @Override
  protected void deliveryCreationError(FileResource resource, Exception e, String remoteFile) {
    try {
      resource.rename(remoteFile + ".active", remoteFile + ".error");
    } catch (AdminException e1) {
      log.error(e1, e1);
    }
  }

  @Override
  protected void deliveryFinished(FileResource resource, String remoteFile) throws AdminException {
    resource.rename(remoteFile + ".active", remoteFile + ".finished");
  }
}
