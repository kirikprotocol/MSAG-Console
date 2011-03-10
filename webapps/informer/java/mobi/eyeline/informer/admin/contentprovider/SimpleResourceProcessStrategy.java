package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;

/**
* User: artem
* Date: 27.01.11
*/
class SimpleResourceProcessStrategy extends BaseResourceProcessStrategy {

  SimpleResourceProcessStrategy(ContentProviderContext context, FileResource resource, ResourceOptions opts) throws AdminException {
    super(context, resource, opts);
  }

  @Override
  protected void fileProccesed(FileResource resource, String remoteFile) throws AdminException {
    resource.remove(remoteFile);
  }

  @Override
  protected void deliveryCreationError(FileResource resource, Exception e, String remoteFile) {
  }

  @Override
  protected void deliveryFinished(FileResource resource, String remoteFile) throws AdminException {
  }
}
