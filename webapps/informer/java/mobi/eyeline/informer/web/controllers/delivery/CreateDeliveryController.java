package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.controllers.InformerController;

/**
 * @author Aleksandr Khalitov
 */
public class CreateDeliveryController extends InformerController {

  private CreateDeliveryPage activePage;

  public CreateDeliveryController() {
    activePage = new UploadFilePage(getConfig(), getUserName());
  }

  public String next() {
    try {
      CreateDeliveryPage page = activePage.process(getUserName(), getConfig(), getLocale());
      if (page != null) {
        activePage = page;
      }
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String finish() {
    activePage = new UploadFilePage(getConfig(), getUserName());
    return "DELIVERIES";
  }

  public String returnToStart() {
    System.out.println("RETURN TO START CALLED");
    if (activePage != null)
      activePage.cancel();
    activePage = new UploadFilePage(getConfig(), getUserName());
    return null;
  }

  public String cancel() {
    activePage.cancel();
    activePage = new UploadFilePage(getConfig(), getUserName());
    return "DELIVERIES";
  }

  public CreateDeliveryPage getActivePage() {
    return activePage;
  }

  public boolean isAllowUssdPushDelivery() {
    return getConfig().isAllowUssdPushDeliveries();
  }
}
