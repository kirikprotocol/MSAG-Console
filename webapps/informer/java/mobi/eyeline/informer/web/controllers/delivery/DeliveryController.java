package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.InformerController;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryController extends InformerController{

  public static final String DELIVERY_PARAM = "delivery";

  public static final String DELIVERY_COMEBACK_PARAM = "delivery_comeback";

  protected Configuration config;

  public DeliveryController() {
    this.config = getConfig();
  }

  public String getDeliveryParam() {
    return DELIVERY_PARAM;
  }

  public String getDeliveryComeBackParam() {
    return DELIVERY_COMEBACK_PARAM;
  }
}
