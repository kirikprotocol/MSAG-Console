package mobi.eyeline.informer.web.controllers.archive;

import mobi.eyeline.informer.admin.archive.Request;
import mobi.eyeline.informer.web.LocaleFilter;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

/**
 * author: Aleksandr Khalitov
 */
public class RequestTypeStringConverter implements Converter {
  @Override
  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    return null;
  }

  @Override
  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if(o != null && (o instanceof Request.Type)) {
      return getAsString(getLocale(facesContext), o.toString());
    }
    return null;
  }

  private static Locale getLocale(FacesContext facesContext) {
    Locale l = (Locale) facesContext.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    return l == null ? Locale.ENGLISH : l;
  }

  public static String getAsString(Locale locale, String type) {
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale);
    try {
      return bundle.getString("archive.request.type." + type);
    } catch (MissingResourceException e) {
      return type;
    }
  }
}
