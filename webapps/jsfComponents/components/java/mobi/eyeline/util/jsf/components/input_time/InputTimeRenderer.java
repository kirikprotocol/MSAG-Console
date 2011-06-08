package mobi.eyeline.util.jsf.components.input_time;

import mobi.eyeline.util.Time;

import javax.faces.application.FacesMessage;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import java.io.IOException;

/**
 * @author Aleksandr Khalitov
 */
public class InputTimeRenderer extends Renderer {

  public void decode(FacesContext context, UIComponent component) {
    InputTime cg = (InputTime) component;
    decodeTime(context, cg);
  }

  static void decodeTime(FacesContext context, InputTime cg) {
    String hours = context.getExternalContext().getRequestParameterMap().get(cg.getId()+".hours");
    if (hours != null && hours.trim().length() == 0)
      hours = null;

    String minutes = context.getExternalContext().getRequestParameterMap().get(cg.getId()+".minutes");
    if (minutes != null && minutes.trim().length() == 0)
      minutes = null;

    String seconds = context.getExternalContext().getRequestParameterMap().get(cg.getId()+".seconds");
    if (seconds != null && seconds.trim().length() == 0)
      seconds = null;

    if (hours != null && minutes != null && seconds != null) {
      try {
        cg.setValue(new Time(hours + ':' + minutes + ':' + seconds));
      } catch (IllegalArgumentException e) {
        if (context.getMessages("inputTime_" + cg.getId()) == null || !context.getMessages("inputTime_" + cg.getId()).hasNext()) {
          String errMsg = cg.getErrorMessage();
          if (errMsg == null)
            errMsg = "Invalid time";
          context.addMessage("inputTime_" + cg.getId(), new FacesMessage(errMsg));
        }
      }
    }
  }

  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    InputTime t = (InputTime) component;
    ResponseWriter w = context.getResponseWriter();

    String hoursId = "\"" + t.getId() + ".hours\"";
    String hoursVal = t.getValue() == null ? "" : String.valueOf(t.getValue().getHour());
    if (hoursVal.length() == 1)
      hoursVal = '0' + hoursVal;

    String minId = "\"" + t.getId() + ".minutes\"";
    String minVal = t.getValue() == null ? "" : String.valueOf(t.getValue().getMin());
    if (minVal.length() == 1)
      minVal = '0' + minVal;

    String secId = "\"" + t.getId() + ".seconds\"";
    String secVal = t.getValue() == null ? "" : String.valueOf(t.getValue().getSec());
    if (secVal.length() == 1)
      secVal = '0' + secVal;

    w.append("<div ").append(" id=\"").append(t.getId()).append("\">");
    w.append("<input type=\"text\" id=").append(hoursId).append("\" name=").append(hoursId).append(" value=\"").append(hoursVal).append("\" maxlength=\"3\" size=\"2\">");
    w.append(":");
    w.append("<input type=\"text\" id=").append(minId).append("\" name=").append(minId).append(" value=\"").append(minVal).append("\" maxlength=\"2\" size=\"2\">");
    w.append(":");
    w.append("<input type=\"text\" id=").append(secId).append("\" name=").append(secId).append(" value=\"").append(secVal).append("\" maxlength=\"2\" size=\"2\">");
    w.append("</div>");
  }

}
