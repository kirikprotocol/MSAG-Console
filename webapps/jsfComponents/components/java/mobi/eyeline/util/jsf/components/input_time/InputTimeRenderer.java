package mobi.eyeline.util.jsf.components.input_time;

import mobi.eyeline.util.Time;
import mobi.eyeline.util.jsf.components.HtmlWriter;

import javax.faces.application.FacesMessage;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.ConverterException;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.util.Iterator;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class InputTimeRenderer extends Renderer {

  private static String decodeString(String value) {
    if (value != null && value.trim().length() == 0)
      return null;
    return value;
  }

  @Override
  public void decode(FacesContext context, UIComponent component) {
    String id = component.getId();
    Map<String, String> reqParams = context.getExternalContext().getRequestParameterMap();

    String hours = decodeString(reqParams.get(id + ".hours"));
    String minutes = decodeString(reqParams.get(id + ".minutes"));
    String seconds = decodeString(reqParams.get(id + ".seconds"));

    ((InputTime) component).setSubmittedValue(new String[]{hours, minutes, seconds});
  }

  @Override
  public Object getConvertedValue(FacesContext context, UIComponent component, Object submitted) throws ConverterException {
    String[] submittedValue = (String[]) submitted;
    if (submittedValue == null)
      return null;

    String hours = submittedValue[0];
    String minutes = submittedValue[1];
    String seconds = submittedValue[2];

    if (hours == null && minutes == null && seconds == null)
      return null;

    try {
      return new Time(hours + ":" + minutes + ":" + seconds);
    } catch (IllegalArgumentException e) {
      e.printStackTrace();
      throw new ConverterException();
    }
  }

  private static String encodeString(String value, boolean fixLen) {
    if (value == null)
      return "";
    if (fixLen && value.length() == 1)
      return '0' + value;
    return value;
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
    InputTime t = (InputTime) component;

    String hours = "", minutes = "", seconds = "";
    String[] submittedValue = (String[]) t.getSubmittedValue();
    Time value = (Time) t.getValue();
    if (submittedValue != null) {
      hours = encodeString(submittedValue[0], t.isValid());
      minutes = encodeString(submittedValue[1], t.isValid());
      seconds = encodeString(submittedValue[2], t.isValid());
    } else if (value != null) {
      hours = encodeString(String.valueOf(value.getHour()), t.isValid());
      minutes = encodeString(String.valueOf(value.getMin()), t.isValid());
      seconds = encodeString(String.valueOf(value.getSec()), t.isValid());
    }

    String id = t.getId();
    String inputClass = "";
    if (!t.isValid())
      inputClass = "class=\"validationError\"";

    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    w.a("<div class=\"eyeline_inputTime\" id=\"").a(id).a("\">");
    w.a("<input type=\"text\" id=\"").a(id).a(".hours\" name=\"").a(id).a(".hours\" value=\"").a(hours).a("\" maxlength=\"3\" size=\"2\"").a(inputClass).a(">");
    w.a(":");
    w.a("<input type=\"text\" id=\"").a(id).a(".minutes\" name=\"").a(id).a(".minutes\" value=\"").a(minutes).a("\" maxlength=\"2\" size=\"2\"").a(inputClass).a(" >");
    w.a(":");
    w.a("<input type=\"text\" id=\"").a(id).a(".seconds\" name=\"").a(id).a(".seconds\" value=\"").a(seconds).a("\" maxlength=\"2\" size=\"2\"").a(inputClass).a(">");
    if (!t.isValid()) {
      String text = getErrorMessage(context, t);
      if (text != null)
        w.a("<span class=\"error\" title=\"").a(text).a("\"> (!)</span>");
    }
    w.a("</div>");
  }

}
