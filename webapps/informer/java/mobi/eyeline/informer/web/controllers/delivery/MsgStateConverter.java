package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.web.LocaleFilter;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

/**
 * @author Artem Snopkov
 */
public class MsgStateConverter implements Converter {

  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    if (s != null && s.length() > 0)
      return MsgState.valueOf(s);
    return null;
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    System.out.println("GET AS STRING: " + o);
    if (o == null || !(o instanceof MsgState)) {
      return null;
    }

    return getAsString(getLocale(facesContext), (MsgState) o);

  }

  private static Locale getLocale(FacesContext facesContext) {
    Locale l = (Locale) facesContext.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    return l == null ? Locale.ENGLISH : l;
  }

  public static String getAsString(Locale locale, MsgState state) {
    return getAsString(locale, state.toString());
  }

  public static String getAsString(Locale locale, String state) {
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale);
    try {
      return bundle.getString("message.state." + state);
    } catch (MissingResourceException e) {
      return state;
    }
  }
}


