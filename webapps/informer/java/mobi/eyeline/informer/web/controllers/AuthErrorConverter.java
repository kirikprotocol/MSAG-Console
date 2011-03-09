package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.web.auth.Authenticator;

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
public class AuthErrorConverter implements Converter{
  @Override
  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    return null;
  }
public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if (o == null || !(o instanceof Authenticator.Error)) {
      return null;
    }
    return getAsString(getLocale(facesContext), (Authenticator.Error) o);
  }

  private static Locale getLocale(FacesContext facesContext) {
    Locale l = facesContext.getViewRoot() != null ? facesContext.getViewRoot().getLocale() : facesContext.getExternalContext().getRequestLocale();
    return l == null ? Locale.ENGLISH : l;
  }

  public static String getAsString(Locale locale, Authenticator.Error error) {
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale);
    try {
      return bundle.getString("auth.error." + error.toString());
    } catch (MissingResourceException e) {
      return error.toString();
    }
  }
}
