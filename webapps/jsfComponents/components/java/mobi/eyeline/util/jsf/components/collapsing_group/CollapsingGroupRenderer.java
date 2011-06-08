package mobi.eyeline.util.jsf.components.collapsing_group;

import mobi.eyeline.util.jsf.components.ResourceUtils;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import java.io.IOException;

/**
 * @author Artem Snopkov
 */
public class CollapsingGroupRenderer extends Renderer {

  public void decode(FacesContext context, UIComponent component) {
    CollapsingGroup cg = (CollapsingGroup) component;
    decodeGroup(context, cg);
  }

  private void importResorces(FacesContext context, ResponseWriter writer) throws IOException{
    String resourceURL = ResourceUtils.getResourceUrl("js/collapsing_group.js");
    writer.startElement("script", null);
    writer.writeAttribute("type", "text/javascript", null);
    writer.writeAttribute("src", resourceURL, null);
    writer.endElement("script");
  }


  static void decodeGroup(FacesContext context, CollapsingGroup cg) {
    String opened = context.getExternalContext().getRequestParameterMap().get("sectionOpened_" + cg.getId());
    cg.setOpened((opened != null && opened.equals("true")));
  }

  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {

    CollapsingGroup cg = (CollapsingGroup) component;
    ResponseWriter writer = context.getResponseWriter();

    importResorces(context, writer);


    writer.
        append("\n<input type=\"hidden\" id=\"sectionOpened_").
        append(cg.getId()).
        append("\" name=\"sectionOpened_").
        append(cg.getId()).
        append("\" value=\"").
        append(cg.isOpened() ? "true" : "false").
        append("\"/>");
    writer.append("");

    writer.append("\n <table cellspacing=\"0\" cellpadding=\"0\"><tr><td id=\"sectionHeaderFlag_");
    writer.append(cg.getId());
    writer.append("\" class=\"");
    if (!cg.isOpened())
      writer.append("eyeline_collapsing_group_flag_closed");
    else
      writer.append("eyeline_collapsing_group_flag_opened");

    writer.append("\" onclick=\"collasping_group_showhide_section('");
    writer.append(cg.getId());
    writer.append("')\">");

    writer.append("</td><td class=\"");

    if (!cg.isOpened())
      writer.append("eyeline_collapsing_group_closed");
    else
      writer.append("eyeline_collapsing_group_opened");

    writer.append("\" id=\"sectionHeader_").
        append(cg.getId()).
        append("\">");

    UIComponent headerFacet = cg.getFacet("header");
    if (headerFacet != null) {
      headerFacet.encodeBegin(context);
      headerFacet.encodeEnd(context);
    } else {
      writer.append(cg.getLabel());
    }

    writer.append("</td></tr>");
    writer.append("<tr id=\"sectionValue_");
    writer.append(cg.getId());
    writer.append("\" ");

    if (!cg.isOpened())
      writer.append("style=\"display:none\"");
    writer.append(">");
    writer.append("\n<td></td><td>");
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    ResponseWriter writer = context.getResponseWriter();
    writer.append("\n</td>");
    writer.append("\n</tr>");
    writer.append("\n</table>");
  }
}
