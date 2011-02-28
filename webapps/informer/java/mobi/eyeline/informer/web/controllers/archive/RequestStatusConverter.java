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
 * @author Aleksandr Khalitov
 */
public class RequestStatusConverter implements Converter {
  @Override
  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    return null;
  }


  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if (o == null || !(o instanceof Request.Status)) {
      return null;
    }

    return getAsString(getLocale(facesContext), (Request.Status) o);

  }

  private static Locale getLocale(FacesContext facesContext) {
    Locale l = (Locale) facesContext.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    return l == null ? Locale.ENGLISH : l;
  }

  public static String getAsString(Locale locale, Request.Status status) {
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale);
    try {
      return bundle.getString("archive.request.status." + status.toString());
    } catch (MissingResourceException e) {
      return status.toString();
    }
  }
}
