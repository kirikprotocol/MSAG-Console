package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.config.Configuration;

import java.io.Serializable;

/**
 * @author Aleksandr Khalitov
 */
public interface CommonDeliveryResender extends Serializable{

  public void resend(User u, Configuration config, String newText) throws AdminException;

  public int getDeliveryId();
  
}
