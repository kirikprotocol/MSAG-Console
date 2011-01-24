package mobi.eyeline.informer.web.controllers.users;

import mobi.eyeline.informer.admin.users.UserCPsettings;
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
public class CpProtocolConverter implements Converter {

  @Override
  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    return null;
  }

  @Override
  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if(o == null || !(o instanceof UserCPsettings.Protocol)) {
      return null;
    }
    return getAsString(getLocale(facesContext), (UserCPsettings.Protocol)o);
  }


  private static Locale getLocale(FacesContext facesContext) {
    Locale l = (Locale) facesContext.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    return l == null ? Locale.ENGLISH : l;
  }

  private static String getAsString(Locale locale, UserCPsettings.Protocol pr) {
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", locale);
    try {
      return bundle.getString("user.edit.import.protocol." + pr.toString());
    } catch (MissingResourceException e) {
      return pr.toString();
    }
  }

}
