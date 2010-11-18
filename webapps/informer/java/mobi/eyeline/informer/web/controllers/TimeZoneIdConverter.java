package mobi.eyeline.informer.web.controllers;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import java.util.TimeZone;

/**
 * @author Aleksandr Khalitov
 */
public class TimeZoneIdConverter implements Converter {

  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    if (s == null || (s = s.trim()).length() == 0) {
      return null;
    }
    return TimeZone.getTimeZone(s);
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if (o == null || !(o instanceof TimeZone)) {
      return null;
    }
    return ((TimeZone) o).getID();
  }
}
