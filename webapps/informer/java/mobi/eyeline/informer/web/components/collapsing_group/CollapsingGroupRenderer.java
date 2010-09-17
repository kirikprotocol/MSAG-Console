package mobi.eyeline.informer.web.components.collapsing_group;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class  CollapsingGroupRenderer extends Renderer {

  public void decode(FacesContext context, UIComponent component) {
    CollapsingGroup cg = (CollapsingGroup)component;

    String opened = context.getExternalContext().getRequestParameterMap().get("sectionOpened_" + cg.getId());
    cg.setOpened((opened != null && opened.equals("true")));
  }

  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {

    CollapsingGroup cg = (CollapsingGroup)component;
    ResponseWriter writer = context.getResponseWriter();
    writer.
        append("\n<div class=\"");
    if (!cg.isOpened())
      writer.append("collapsing_group_closed");
    else
      writer.append("collapsing_group_opened");

    writer.append("\" id=\"sectionHeader_").
        append(cg.getId()).
        append("\" onclick=\"collasping_group_showhide_section('").
        append(cg.getId()).
        append("')\">");
    writer.append(cg.getLabel());
    writer.
        append("\n<input type=\"hidden\" id=\"sectionOpened_").
        append(cg.getId()).
        append("\" name=\"sectionOpened_").
        append(cg.getId()).
        append("\" value=\"").
        append(cg.isOpened() ? "true" : "false").
        append("\"/>");
    writer.append("\n</div>");

    writer.
        append("\n<table cellspacing=\"0\" cellpadding=\"0\" id=\"sectionValue_").
        append(cg.getId()).
        append("\"");
    if (!cg.isOpened())
      writer.append("style=\"display:none\"");
    writer.append(">");

    writer.append("\n<tr>");
    writer.append("\n<th width=\"30px\"/>");
    writer.append("\n<td>");
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    ResponseWriter writer = context.getResponseWriter();
    writer.append("\n</td>");
    writer.append("\n</tr>");
    writer.append("\n</table>");
  }
}
