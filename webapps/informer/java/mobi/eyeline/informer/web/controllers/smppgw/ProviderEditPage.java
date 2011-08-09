package mobi.eyeline.informer.web.controllers.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.controllers.InformerController;

/**
 * author: Aleksandr Khalitov
 */
public abstract class ProviderEditPage extends InformerController{

  public abstract ProviderEditPage nextPage() throws AdminException;

  public abstract ProviderEditPage backPage() throws AdminException;

  public abstract String getPageId();

}
