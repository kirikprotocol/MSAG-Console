package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.config.Configuration;

import java.io.Serializable;
import java.util.Locale;

/**
 * @author Aleksandr Khalitov
 */
public interface CreateDeliveryPage extends Serializable{

  public CreateDeliveryPage process(String user, Configuration config, Locale locale) throws AdminException;

  public String getPageId();

  public void cancel();
}
