package mobi.eyeline.util.jsf.components.input_text;

import mobi.eyeline.util.jsf.components.HtmlWriter;

import javax.faces.application.FacesMessage;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.Converter;
import javax.faces.convert.ConverterException;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.util.Iterator;
import java.util.Map;

/**
 * author: Aleksandr Khalitov
 */
public class InputTextRenderer extends Renderer{

  private static String decodeString(Object value) {
    String v = value == null ? null : value.toString().trim();
    if (v == null || v.length() == 0)
      return null;
    return v;
  }

  @Override
  public void decode(FacesContext context, UIComponent component) {
    String id = component.getId();
    Map<String, String> reqParams = context.getExternalContext().getRequestParameterMap();
    String submittedValue = decodeString(reqParams.get(id));
    ((InputText) component).setSubmittedValue(new String[]{submittedValue});
  }

  @Override
  public Object getConvertedValue(FacesContext context, UIComponent component, Object submittedValue) throws ConverterException {
    Converter c = ((InputText)component).getConverter();
    String v = submittedValue == null ? null : ((String[])submittedValue)[0];
    return c == null ? v : c.getAsObject(context, component, v);
  }

  private String getErrorMessage(FacesContext context, UIComponent t) {
    Iterator messages = context.getMessages(t.getClientId(context));
    if (messages != null && messages.hasNext()) {
      FacesMessage m = (FacesMessage) messages.next();
      String text = m.getDetail();
      if (text == null || text.length() == 0)
        text = m.getSummary();
      return text;
    }
    return null;
  }

  @Override
  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    InputText inputText = (InputText) component;

    Map<String, Object> attributes = inputText.getAttributes();
    Integer cols = inputText.getCols();
    Integer rows = inputText.getRows();
    String readonly = decodeString(attributes.get("readonly"));


    Object submitted = inputText.getSubmittedValue();
    String submittedValue = null;
    if (submitted != null)
      submittedValue = ((String[]) submitted)[0];

    String value = null;

    if (submittedValue != null) {
      value = submittedValue;
    } else if (inputText.getValue() != null) {
      Converter c = inputText.getConverter();
      value = c == null ? inputText.getValue().toString() : c.getAsString(context, component, inputText.getValue());
    }

    if(value == null) {
      value = "";
    }


    String inputClass = "";
    if (!inputText.isValid())
      inputClass = "class=\"validationError\"";

    String id = inputText.getId();

    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    w.a("<div class=\"eyeline_inputText\" id=\"").a(id).a(".div\">");

    boolean textArea = rows != null;

    if(!textArea) {
      w.a("<input type=\"text\" value=\"").a(value).a('\"');
    }else {
      w.a("<textarea rows=\"").a(rows.toString()).a('\"');
    }
    w.a(" id=\"").a(id).a("\" name=\"").a(id).a("\" ").a(inputClass);
    if(cols != null) {
      if(!textArea) {
        w.a(" size");
      }else {
        w.a(" cols");
      }
      w.a("=\"").a(cols.toString()).a('\"');
    }
    if(readonly != null && Boolean.parseBoolean(readonly)) {
      w.a(" readonly");
    }
    w.a('>');
    if(textArea) {
      w.a(value).a("</textarea>");
    }

    if (!inputText.isValid()) {
      String text = getErrorMessage(context, inputText);
      if (text != null)
        w.a("<span class=\"error\" title=\"").a(text).a("\"> (!)</span>");
    }
    w.a("</div>");
  }
}
