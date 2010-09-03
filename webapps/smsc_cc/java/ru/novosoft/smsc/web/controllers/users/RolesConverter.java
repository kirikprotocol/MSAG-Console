package ru.novosoft.smsc.web.controllers.users;

import ru.novosoft.smsc.web.LocaleFilter;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * author: alkhal
 */
public class RolesConverter implements Converter {

  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    return null;
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    return getAsString(facesContext, o);
  }

  public String getAsString(FacesContext facesContext, Object o) throws ConverterException {
    if(o == null || o.toString().equals("") ){
      return null;
    }
    String role = o.toString();
    ResourceBundle bundle = ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc",
        (Locale)facesContext.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER));
    String result = bundle.getString("users.roles."+role);
    return result == null ? role : result;
  }

}
