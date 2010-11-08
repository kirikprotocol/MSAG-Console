package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.DeliveryException;
import mobi.eyeline.informer.admin.delivery.DeliveryMode;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.config.Configuration;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * @author Aleksandr Khalitov
 */
public class StartPage implements CreateDeliveryPage{

  private boolean singleText;

  private String text;

  private static void setDefaults(String user, Configuration config, Delivery delivery) throws AdminException {
    delivery.setOwner(user);
    User u = config.getUser(user);
    try{
      delivery.setSourceAddress(u.getSourceAddr());
      delivery.setEmailNotificationAddress(u.getEmail());
      if(u.getPhone() != null) {
        delivery.setSmsNotificationAddress(new Address(u.getPhone()));
      }
      if(u.getDeliveryType() != null) {
        switch (u.getDeliveryType()) {
          case SMS:
            delivery.setDeliveryMode(DeliveryMode.SMS);
            break;
          case USSD_PUSH:
            delivery.setDeliveryMode(DeliveryMode.USSD_PUSH);
            delivery.setTransactionMode(true);
            break;
          case USSD_PUSH_VIA_VLR:
            delivery.setDeliveryMode(DeliveryMode.USSD_PUSH_VLR);
            delivery.setTransactionMode(true);
            break;
        }
      }
      if(u.getPolicyId() != null) {
        delivery.setRetryPolicy(u.getPolicyId());
        delivery.setRetryOnFail(true);
      }
      delivery.setPriority(u.getPriority());
      Time t;
      if((t = u.getDeliveryStartTime()) != null) {
        delivery.setActivePeriodStart(t.getTimeDate());
      }
      if((t = u.getDeliveryEndTime()) != null) {
        delivery.setActivePeriodEnd(t.getTimeDate());
      }
      delivery.setValidityPeriod(Integer.toString(u.getValidHours()));
      if(u.getDeliveryDays() != null && !u.getDeliveryDays().isEmpty()) {
        List<Delivery.Day> days = new ArrayList<Delivery.Day>(7);
        for(Integer i : u.getDeliveryDays()) {
          days.add(Delivery.Day.valueOf(i == 0 ? 7 : i));
        }
        delivery.setActiveWeekDays(days.toArray(new Delivery.Day[days.size()]));
      }
    }catch (AdminException e){}
  }

  public CreateDeliveryPage process(String user, Configuration config, Locale locale) throws AdminException{
    Delivery delivery;
    if(singleText) {
      if(text == null || (text = text.trim()).length() == 0) {
        throw new DeliveryException("delivery_text_empty");
      }
      delivery = Delivery.newSingleTextDelivery();
      delivery.setSingleText(text);
    }else {
      delivery = Delivery.newCommonDelivery();
    }
    setDefaults(user, config, delivery);

    return new UploadFilePage(delivery, new File(config.getWorkDir(),"messages_"+user+System.currentTimeMillis()));
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

  public void cancel() {}
}
