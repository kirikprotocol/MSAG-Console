package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryException;

import java.io.File;

/**
 * @author Aleksandr Khalitov
 */
public class StartPage implements CreateDeliveryPage{

  private Delivery delivery;

  private boolean singleText;

  private String text;

  public CreateDeliveryPage process(String user) throws AdminException{
    if(singleText) {
      if(text == null || (text = text.trim()).length() == 0) {
        throw new DeliveryException("delivery_text_empty");
      }
      delivery = Delivery.newSingleTextDelivery();
      delivery.setSingleText(text);
    }else {
      delivery = Delivery.newCommonDelivery();
    }
    delivery.setOwner(user);
    return new UploadFilePage(delivery, new File("messages_"+System.currentTimeMillis()));//todo
  }

  public boolean isSingleText() {
    return singleText;
  }

  public void setSingleText(boolean singleText) {
    this.singleText = singleText;
  }

  public String getText() {
    return text;
  }

  public void setText(String text) {
    this.text = text;
  }
  
  public String getPageId() {
    return "DELIVERY_CREATE";
  }
}
