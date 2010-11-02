package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.util.Address;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;

/**
 * @author Aleksandr Khalitov
 */
public class AddressConverter implements Converter {
  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    if(s == null || (s = s.trim()).length() == 0) {
      return null;
    }
    try{
      return new Address(s);
    }catch (IllegalArgumentException e){
      throw new ConverterException();
    }
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if(o == null || !(o instanceof Address)) {
      return null;
    }
    return ((Address)o).getSimpleAddress();
  }
}
