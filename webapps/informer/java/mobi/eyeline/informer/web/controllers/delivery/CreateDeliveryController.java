package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.controllers.InformerController;

/**
 * @author Aleksandr Khalitov
 */
public class CreateDeliveryController extends InformerController {

  private CreateDeliveryPage activePage = new StartPage();

  public String next() {
    try {
      CreateDeliveryPage page = activePage.process(getUserName(), getConfig(), getLocale());
      if(page != null) {
        activePage = page;
      }
    } catch (AdminException e) {
      addError(e);
    }
    return null;
  }

  public String cancel() {
    activePage.cancel();
    activePage = new StartPage();
    return null;
  }

  public CreateDeliveryPage getActivePage() {
    return activePage;
  }
}
