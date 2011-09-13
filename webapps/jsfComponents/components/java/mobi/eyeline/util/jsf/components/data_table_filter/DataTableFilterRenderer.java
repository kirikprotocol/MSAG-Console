package mobi.eyeline.util.jsf.components.data_table_filter;

import mobi.eyeline.util.jsf.components.HtmlWriter;
import mobi.eyeline.util.jsf.components.MessageUtils;
import mobi.eyeline.util.jsf.components.ResourceUtils;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.util.Map;

/**
 * author: Aleksandr Khalitov
 */
public class DataTableFilterRenderer extends Renderer{

  private static final String APPLY = "mobi.eyeline.util.jsf.components.data_table_filter.DataTableFilter.APPLY";
  private static final String CLEAR = "mobi.eyeline.util.jsf.components.data_table_filter.DataTableFilter.CLEAR";
  private static final String TITLE = "mobi.eyeline.util.jsf.components.data_table_filter.DataTableFilter.TITLE";

  @Override
  public void decode(FacesContext context, UIComponent component) {
    String id = component.getId();
    Map<String, String> reqParams = context.getExternalContext().getRequestParameterMap();
    String action = reqParams.get("action"+id);
    ((DataTableFilter)component).setAction(action);
  }


  private void importResorces(HtmlWriter writer) throws IOException {
    String resourceURL = ResourceUtils.getResourceUrl("js/data_table_filter.js");
    writer.a("<script type=\"text/javascript\" src=\"").a(resourceURL).a("\"></script>");
  }

  @Override
  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    DataTableFilter filter = (DataTableFilter)component;

    String filterTitle = MessageUtils.getMessageString(context, TITLE);

    String applyString = null, clearString = null;

    if(filter.containsApplyAction()) {
      applyString = MessageUtils.getMessageString(context, APPLY);
    }
    if(filter.containsClearAction()) {
      clearString = MessageUtils.getMessageString(context, CLEAR);
    }

    HtmlWriter w = new HtmlWriter(context.getResponseWriter());

    importResorces(w);

    w.a("<div id=\"").a(filter.getId()).a("\" class=\"eyeline_data_table_filter\">\n");
    w.a("<input type=\"hidden\" id=\"").a("action").a(component.getId()).a("\" name=\"action").a(component.getId()).a("\" value=\"\">");
    w.a("  <table class=\"eyeline_data_table_filter\" width=\"100%\" cellpadding=\"0\" cellspacing=\"0\">\n" +
        "    <tr>\n" +
        "      <td align=\"left\" width=\"10%\">\n" +
        "        <span class=\"eyeline_filter\">").a(filterTitle).a("</span>\n" +
        "      </td>\n" +
        "      <td align=\"left\" width=\"80%\">&#160;</td>\n" +
        "      <td width=\"10%\" align=\"right\">\n");
    if(applyString != null) {
      w.a("              <span class=\"eyeline_data_table_filter_button\" onclick=\"new DataTableFilter('").a(filter.getId()).a("').clickApply('apply')\">").a(applyString).a("</span>");
    }
    if(applyString != null && clearString != null) {
      w.a(" | ");
    }
    if(clearString != null) {
      w.a("<span class=\"eyeline_data_table_filter_button\" onclick=\"new DataTableFilter('").a(filter.getId()).a("').clickApply('clear')\">").a(clearString).a("</span>");
    }

    w.a("      </td>\n" +
        "    </tr>\n" +
        "  </table>");
  }

  @Override
  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    w.a("\n</div>");
  }
}
