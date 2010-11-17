package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.config.Configuration;

import java.util.Locale;

/**
 * @author Aleksandr Khalitov
 */
public class StartPage implements CreateDeliveryPage{

  private boolean singleText;

  public CreateDeliveryPage process(String user, Configuration config, Locale locale) throws AdminException{
//    Delivery delivery;
//    if(singleText) {
//      delivery = Delivery.newSingleTextDelivery();
//    }else {
//      delivery = Delivery.newCommonDelivery();
//    }
//    config.getDefaultDelivery(user, delivery);
//
//    return new UploadFilePage(delivery, new File(config.getWorkDir(),"messages_"+user+System.currentTimeMillis()), config.getFileSystem());
    return null;
  }

  public boolean isSingleText() {
    return singleText;
  }

  public void setSingleText(boolean singleText) {
    this.singleText = singleText;
  }

  public String getPageId() {
    return "DELIVERY_CREATE";
  }

  public void cancel() {}
}
