package ru.novosoft.smsc.web.controllers.closed_groups;

import ru.novosoft.smsc.util.Address;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;

/**
 * author: alkhal
 */
public class AdressesConverter implements Converter {

  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    if (s == null || (s = s.trim()).length() == 0) {
      return null;
    }
    try {
      return new Address(s);
    } catch (IllegalArgumentException e) {
      throw new ConverterException();
    }
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if (o == null || !(o instanceof Address)) {
      return null;
    }
    return ((Address) o).getSimpleAddress();
  }
}
