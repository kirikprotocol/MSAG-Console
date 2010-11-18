package mobi.eyeline.informer.web.controllers;

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
public class ErrorCodeConverter implements Converter {

  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    return null;
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if (o == null) {
      return null;
    }
    return getAsString(getLocale(facesContext), o.toString());
  }

  private static Locale getLocale(FacesContext facesContext) {
    Locale l = (Locale) facesContext.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    return l == null ? Locale.ENGLISH : l;
  }

  public static String getAsString(Locale locale, String code) {
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.admin.SmppStatus", locale);
    try {
      return bundle.getString("informer.errcode." + code);
    } catch (MissingResourceException e) {
      return bundle.getString("informer.errcode.unknown");
    }
  }
}
