package mobi.eyeline.util.jsf.components.sample.controllers;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;

/**
 * author: Aleksandr Khalitov
 */
public class StringWrapperConverter implements Converter{

  @Override
  public Object getAsObject(FacesContext context, UIComponent uiComponent, String s) throws ConverterException {
    return new StringWrapper(s);
  }

  @Override
  public String getAsString(FacesContext context, UIComponent uiComponent, Object o) throws ConverterException {
    return o == null || !(o instanceof StringWrapper) ? null : ((StringWrapper)o).getStr();
  }
}
