package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;

/**
 * @author Aleksandr Khalitov
 */
public interface CreateDeliveryPage {

  public CreateDeliveryPage process(String user) throws AdminException;

  public String getPageId();
}
