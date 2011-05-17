package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.web.LocaleFilter;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class MessageStateConverter implements Converter {

  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    return null;
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if (o == null || !(o instanceof MessageState)) {
      return null;
    }

    return getAsString(getLocale(facesContext), (MessageState) o);

  }

  private static Locale getLocale(FacesContext facesContext) {
    Locale l = (Locale) facesContext.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    return l == null ? Locale.ENGLISH : l;
  }

  public static String getAsString(Locale locale, MessageState state) {
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale);
    String stateName;
    switch (state) {
      case New:
      case Process:
        stateName="New";
        break;
      default:
        stateName = state.toString();
    }
    try {
      return bundle.getString("message.state." + stateName);
    } catch (MissingResourceException e) {
      return state.toString();
    }
  }
}
