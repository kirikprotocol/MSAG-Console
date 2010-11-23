package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.delivery.DeliveryStatus;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryStatusConverter implements Converter {

  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    return null;
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if (o == null || !(o instanceof DeliveryStatus)) {
      return null;
    }
    DeliveryStatus status = (DeliveryStatus) o;
    switch (status) {
      case Finished: break;
      case Paused: break;
      case Active: break;
      case Cancelled: break;
      case Planned: break;
    }
    return null;
  }
}
