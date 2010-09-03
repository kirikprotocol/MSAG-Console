package ru.novosoft.smsc.web.controllers.reschedule;

import ru.novosoft.smsc.web.LocaleFilter;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * alkhal: alkhal
 */
public class SmppStatusConverter implements Converter {

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
    Integer code = Integer.parseInt(o.toString());
    ResourceBundle bundle = ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc",
        (Locale)facesContext.getExternalContext().getRequestMap().get(LocaleFilter.LOCALE_PARAMETER));
    return "("+code+") "+bundle.getString("smsc.errcode."+code);
  }

  public Integer getAsInteger(String o) throws ConverterException {
    if(o == null || o.equals("") ){
      return null;
    }
    try{
      return Integer.parseInt(o.substring(1, o.indexOf(")")));
    }catch (Exception e) {
      e.printStackTrace();
      return null;
    }
  }


}
