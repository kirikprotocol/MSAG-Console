package mobi.eyeline.informer.admin.contentprovider;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.contentprovider.resources.FileResource;
import org.apache.log4j.Logger;

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
  protected void fileProccesed(FileResource resource, String remoteFile) throws AdminException {
    resource.rename(remoteFile, remoteFile + ".active");
  }

  @Override
  protected void deliveryCreationError(FileResource resource, Exception e, String remoteFile) {
    try {
      resource.rename(remoteFile, remoteFile + ".error");
    } catch (AdminException e1) {
      log.error(e1, e1);
    }
  }

  @Override
  protected void deliveryFinished(FileResource resource, String remoteFile) throws AdminException {
    resource.rename(remoteFile + ".active", remoteFile + ".finished");
  }
}
