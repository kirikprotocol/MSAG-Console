package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.web.controllers.InformerController;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryController extends InformerController{

  public static final String DELIVERY_PARAM = "delivery";

  public String getDeliveryParam() {
    return DELIVERY_PARAM;
  }
}
