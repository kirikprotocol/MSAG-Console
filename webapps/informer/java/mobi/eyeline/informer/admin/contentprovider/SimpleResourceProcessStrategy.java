package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryPrototype;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;
import java.util.Date;
import java.util.List;

/**
* User: artem
* Date: 27.01.11
*/
class SimpleResourceProcessStrategy extends BaseResourceProcessStrategy {

  SimpleResourceProcessStrategy(ContentProviderContext context, FileResource resource, ResourceOptions opts) throws AdminException {
    super(context, resource, opts);
  }

  @Override
  protected void fileDownloaded(FileResource resource, String remoteFile) throws AdminException {
    resource.remove(remoteFile);
  }

  @Override
  protected void deliveryCreationError(FileResource resource, Exception e, String remoteFile) {
  }

  @Override
  protected void deliveryFinished(FileResource resource, String remoteFile) throws AdminException {
  }
}
