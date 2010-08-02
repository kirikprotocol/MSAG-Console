package ru.novosoft.smsc.web.controllers;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import java.util.ResourceBundle;

/**
 * alkhal: alkhal
 */
public class SmppStatusConverter implements Converter {

  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    return null; 
  }

  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if(o == null || o.toString().equals("") ){
      return null;
    }
    Integer code = Integer.parseInt(o.toString());
    ResourceBundle bundle = ResourceBundle.getBundle("ru.novosoft.smsc.web.resources.Smsc",
        FacesContext.getCurrentInstance().getExternalContext().getRequestLocale());
    return "("+code+") "+bundle.getString("smsc.errcode."+code);
  }


}