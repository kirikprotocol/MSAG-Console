package mobi.eyeline.util.jsf.components.graph;

import mobi.eyeline.util.jsf.components.AjaxFacesContext;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class LinesRenderer extends Renderer {


  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    if (context instanceof AjaxFacesContext) {
      AjaxFacesContext ctx = (AjaxFacesContext) context;
      if (ctx.getAjaxComponentId().equals(component.getId())) {
        if(component instanceof Lines) {
          ((AjaxFacesContext) context).setSkipContent(false);
          JSONLines jsonLines = new JSONLines();
          jsonLines.labels.addAll(((Lines)component).getLabels());
          for(UIComponent child : component.getChildren()) {
            if(child instanceof LineChart) {
              LineChart c = (LineChart)child;
              jsonLines.lines.add(new JSONLine(c.getValues(), c.getColor(), c.getShade()));
            }
          }
          jsonLines.toJson(context.getResponseWriter());
          ((AjaxFacesContext) context).setSkipContent(true);
        }
        return;
      }
    }

    if(component instanceof Lines) {
      Lines ls = (Lines)component;
      context.getResponseWriter().append("\n<div id=\"").append(component.getId()).append("\" style=\"width:"+ls.getWidth()+"px;height:"+ls.getHeight()+"px;border:1px dashed #CCC;\">");
    }

  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    ResponseWriter w = context.getResponseWriter();

    if (context instanceof AjaxFacesContext) {
      AjaxFacesContext ctx = (AjaxFacesContext) context;
      if (ctx.getAjaxComponentId().equals(component.getId())) {
        return;
      }
    }
    if(component instanceof Lines) {
      Lines l = (Lines)component;
      w.append("\n</div>");
      w.append("\n<script language=\"javascript\" type=\"text/javascript\">");


      w.append("\n  new Lines('" + component.getId() + "', '"+l.getUpdatePeriod()+"', "+l.getHeight()+", "+l.getWidth()+");");
      w.append("\n</script>");
    }

  }
//
//  private static void printCoords(ResponseWriter w, List values) throws IOException {
//    if(values == null || values.isEmpty()) {
//      return;
//    }
//    boolean first = true;
//    for(Object o : values) {
//      if(!first) {
//        w.append(",");
//      }
//      first = false;
//      w.append(o.toString());
//    }
//
//  }


  private static class JSONLines {

    private List<JSONLine> lines = new LinkedList<JSONLine>();

    private List<String> labels = new LinkedList<String>();

    private void toJson(ResponseWriter w) throws IOException {

      w.append("{");
      w.append("\n  \"labels\":[");
      boolean first = true;
      for(String l : labels) {
        if(!first) {
          w.append(',');
        }
        first = false;
        w.append("\"").append(l).append("\"");
      }
      w.append("], ");
      w.append("\n  \"lines\": [");
      first = true;
      for(JSONLine line : lines) {
        if(!first) {
          w.append(',');
        }
        first = false;
        w.append("\n  ");
        line.toJson(w);
      }
      w.append("]");
      w.append("\n}");
    }
  }

  private static class JSONLine {

    private int[][] values;

    private String color;

    private boolean shade;

    private JSONLine(int[][] values, String color, boolean shade) {
      this.values = values;
      this.color = color;
      this.shade = shade;

    }

    private void toJson(ResponseWriter w) throws IOException {

      w.append("{").
          append("\n      \"values\":").append("{");

      boolean first = true;
      for (int[] value : values) {
        int x = value[0];
        int y = value[1];
        if (!first) {
          w.append(',');
        }
        first = false;
        w.append('\"').append(Integer.toString(x)).append("\":\"").append(Integer.toString(y)).append("\"");
      }

      w.append("}, \n     \"color\":\"").append(color).append("\", \n    ");
      w.append("\"shade\":").append(Boolean.toString(shade)).append("\n    }");
    }


  }

}
