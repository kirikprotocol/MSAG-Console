package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.web.LocaleFilter;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryStatusConverter implements Converter {
  
  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    return null;
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if(o==null || !(o instanceof DeliveryStatus)) {
      return null;
    }
    DeliveryStatus status = (DeliveryStatus)o;
    return getAsString(getLocale(facesContext), status);
  }

  private static Locale getLocale(FacesContext facesContext) {
    Locale l = (Locale) FacesContext.getCurrentInstance().getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    return l == null ? Locale.ENGLISH : l;
  }

  public static String getAsString(Locale locale, DeliveryStatus status) {
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale);
    return bundle.getString("delivery.status."+status);
  }
}
