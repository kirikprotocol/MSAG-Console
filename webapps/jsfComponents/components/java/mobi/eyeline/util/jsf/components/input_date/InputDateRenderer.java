package mobi.eyeline.util.jsf.components.input_date;

import mobi.eyeline.util.jsf.components.HtmlWriter;
import mobi.eyeline.util.jsf.components.ResourceUtils;

import javax.faces.application.FacesMessage;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.ConverterException;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Iterator;
import java.util.Map;

/**
 * User: artem
 * Date: 01.08.11
 */
public class InputDateRenderer extends Renderer {

  private final SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy");
  private final SimpleDateFormat dateTimeFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

  private static String decodeString(String value) {
    if (value != null && value.trim().length() == 0)
      return null;
    return value;
  }

  @Override
  public void decode(FacesContext context, UIComponent component) {
    String id = component.getId();
    Map<String, String> reqParams = context.getExternalContext().getRequestParameterMap();
    String submittedValue = decodeString(reqParams.get(id));
    ((InputDate) component).setSubmittedValue(new String[]{submittedValue});
  }

  @Override
  public Object getConvertedValue(FacesContext context, UIComponent component, Object submitted) throws ConverterException {
    String submittedValue = ((String[]) submitted)[0];

    if (submittedValue == null)
      return null;


    InputDate inputDate = (InputDate) component;

    try {
      if (inputDate.isInputTime())
        return dateTimeFormat.parse(submittedValue);
      else
        return dateFormat.parse(submittedValue);
    } catch (ParseException e) {
      e.printStackTrace();
      throw new ConverterException();
    }
  }

  private void importResorces(HtmlWriter writer) throws IOException {
    String resourceURL = ResourceUtils.getResourceUrl("js/input_date.js");
    writer.a("<script type=\"text/javascript\" src=\"").a(resourceURL).a("\"></script>");
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
    InputDate inputDate = (InputDate) component;

    Object submitted = inputDate.getSubmittedValue();
    String submittedValue = null;
    if (submitted != null)
      submittedValue = ((String[]) submitted)[0];

    String id = inputDate.getId();
    String value;
    if (submittedValue != null) {
      value = submittedValue;
    } else if (inputDate.getValue() != null) {
      if (inputDate.isInputTime())
        value = dateTimeFormat.format(inputDate.getValue());
      else
        value = dateFormat.format(inputDate.getValue());
    } else
      value = "";

    String inputClass = "";
    if (!inputDate.isValid())
      inputClass = "class=\"validationError\"";

    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    importResorces(w);
    w.a("<div class=\"eyeline_inputDate\" id=\"").a(id).a(".div\">");
    w.a("<input type=\"text\" id=\"").a(id).a("\" name=\"").a(id).a("\" value=\"").a(value).a("\" ").a(inputClass).a(">");
    if (!inputDate.isValid()) {
      String text = getErrorMessage(context, inputDate);
      if (text != null)
        w.a("<span class=\"error\" title=\"").a(text).a("\"> (!)</span>");
    }
    w.a("</div>");

    String minDate = inputDate.getMinDate() == null ? "null" : "\"" + dateFormat.format(inputDate.getMinDate()) + "\"";
    String maxDate = inputDate.getMaxDate() == null ? "null" : "\"" + dateFormat.format(inputDate.getMaxDate()) + "\"";
    String numberOfMonths = String.valueOf(inputDate.getNumberOfMonths());
    String inputTime = String.valueOf(inputDate.isInputTime());
    String locale = context.getViewRoot().getLocale().getLanguage();

    w.a("<script type=\"text/javascript\">initInputDate(\"").a(id)
        .a("\", \"").a(value)
        .a("\", ").a(minDate)
        .a(", ").a(maxDate)
        .a(", ").a(inputTime)
        .a(", ").a(numberOfMonths)
        .a(", \"").a(locale)
        .a("\");</script>");
  }
}
