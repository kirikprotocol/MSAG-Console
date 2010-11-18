package mobi.eyeline.informer.web.controllers;

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
public class DaemonsConverter implements Converter {

  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    return null;
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    return getAsString(facesContext, o);
  }

  public String getAsString(FacesContext facesContext, Object o) throws ConverterException {
    if (o == null || o.toString().equals("")) {
      return null;
    }
    String key = o.toString();
    Locale l = (Locale) facesContext.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER);
    if (l == null) {
      l = new Locale("en");
    }
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", l);
    String result = bundle.getString("daemon.name." + key);
    return result == null ? key : result;
  }
}
