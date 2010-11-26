package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.util.Day;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;

/**
 * @author Aleksandr Khalitov
 */
public class WeekDaysConverter implements Converter {

  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    if (s == null || (s = s.trim()).length() == 0) {
      return null;
    }
    try {
      return Day.valueOf(s);
    } catch (IllegalArgumentException e) {
      throw new ConverterException(e);
    }
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if (!(o instanceof Day)) {
      return null;
    }
    return o.toString();
  }

}
