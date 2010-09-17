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

  public Object getAsObject(FacesContext context, UIComponent uiComponent, String s) throws ConverterException {
    return null;
  }

  public String getAsString(FacesContext context, UIComponent uiComponent, Object o) throws ConverterException {
    return o == null ? null : o instanceof Address ? ((Address)o).getSimpleAddress() : o.toString();
  }
}
