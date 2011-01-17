package mobi.eyeline.informer.web.components.input_time;

import mobi.eyeline.informer.util.Time;
import mobi.eyeline.informer.web.components.collapsing_group.CollapsingGroup;

import javax.faces.application.FacesMessage;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import java.io.IOException;

/**
 * User: artem
 * Date: 17.01.11
 */
public class InputTimeRenderer extends Renderer {

  public void decode(FacesContext context, UIComponent component) {
    InputTime cg = (InputTime) component;
    decodeTime(context, cg);
  }

  static void decodeTime(FacesContext context, InputTime cg) {
    String hours = context.getExternalContext().getRequestParameterMap().get("hours_" + cg.getId());
    String minutes = context.getExternalContext().getRequestParameterMap().get("minutes_" + cg.getId());
    if (hours != null && minutes != null) {
      try {
        cg.setValue(new Time(hours + ':' + minutes));
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

    String hoursId = "\"hours_" + t.getId() + "\"";
    String hoursVal = t.getValue() == null ? "" : String.valueOf(t.getValue().getHour());
    String minId = "\"minutes_" + t.getId() + "\"";
    String minVal = t.getValue() == null ? "" : String.valueOf(t.getValue().getMin());
    w.append("<div>");
    w.append("<input type=\"text\" id=").append(hoursId).append("\" name=").append(hoursId).append(" value=\"").append(hoursVal).append("\" maxlength=\"3\" size=\"2\">");
    w.append(":");
    w.append("<input type=\"text\" id=").append(minId).append("\" name=").append(minId).append(" value=\"").append(minVal).append("\" maxlength=\"2\" size=\"2\">");
    w.append("</div>");
  }

}
