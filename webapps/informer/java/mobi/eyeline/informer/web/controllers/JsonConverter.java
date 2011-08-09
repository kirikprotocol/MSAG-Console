package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.util.StringEncoderDecoder;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import java.util.Collections;
import java.util.List;

/**
 * author: Aleksandr Khalitov
 */
public class JsonConverter implements Converter{

  @Override
  public Object getAsObject(FacesContext facesContext, UIComponent uiComponent, String s) throws ConverterException {
    if(s == null || s.length() <= 2) {
      return Collections.emptyList();
    }
    s = s.substring(1, s.length()-1).replace("\"","");
    return StringEncoderDecoder.csvDecode(',',s);
  }

  @Override
  public String getAsString(FacesContext facesContext, UIComponent uiComponent, Object o) throws ConverterException {
    if(o == null || !(o instanceof List)) {
      return "[]";
    }
    List<String> ol = (List<String>)o;
    if(ol.isEmpty()) {
      return "[]";
    }
    return "[\""+StringEncoderDecoder.toCSVString(',', ol.toArray()).replace(",","\",\"")+"\"]";
  }


}
