package mobi.eyeline.util.jsf.components.data_table;

import mobi.eyeline.util.jsf.components.AjaxFacesContext;
import mobi.eyeline.util.jsf.components.ResourceUtils;
import mobi.eyeline.util.jsf.components.data_table.model.PreloadableModel;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.io.Writer;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class DataTableRenderer extends Renderer {



  private void importResources(ResponseWriter writer) throws IOException{

    writer.startElement("script", null);
    writer.writeAttribute("type", "text/javascript", null);
    writer.writeAttribute("src", ResourceUtils.getResourceUrl("js/ajax.js"), null);
    writer.endElement("script");

    writer.startElement("script", null);
    writer.writeAttribute("type", "text/javascript", null);
    writer.writeAttribute("src", ResourceUtils.getResourceUrl("js/data_table.js"), null);
    writer.endElement("script");

  }
  private boolean ajax = false;

  private boolean renderChilds = false;

  public void decode(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) {
    DataTable t = (DataTable) component;
    decodeTable(context, t);
  }

  static void decodeTable(javax.faces.context.FacesContext context, DataTable t) {
    Map<String, String> reqParams = context.getExternalContext().getRequestParameterMap();
    String column = reqParams.get(t.getId() + "_column");
    if (column != null && column.trim().length() == 0)
      column = null;
    t.setSortOrder(column);


    String page = reqParams.get(t.getId() + "_page");
    if (page != null && page.trim().length() > 0)
      t.setCurrentPage(Integer.parseInt(page));

    String pageSize = reqParams.get(t.getId() + "_pageSize");
    String previousPageSize = reqParams.get(t.getId() + "_previousPageSize");
    if (pageSize != null && pageSize.trim().length() > 0 && previousPageSize != null && previousPageSize.trim().length() > 0)
      t.updatePageSize(Integer.parseInt(previousPageSize), Integer.parseInt(pageSize));

    String select = reqParams.get(t.getId() + "_select");

    t.clearSelectedRows();

    t.updateSelected(toArray(select));

    String selectAll = reqParams.get(t.getId() + "_selectAll");

    t.setSelectAll(Boolean.parseBoolean(selectAll));

    String showSelected = reqParams.get(t.getId() + "_showSelected");

    t.setShowSelectedOnly(Boolean.parseBoolean(showSelected));

    t.setInternalUpdate(reqParams.containsKey("eyelineComponentUpdate"));

  }


  private static List<Column> getColumns(DataTable t) {
    List<Column> result = new ArrayList<Column>();
    for (UIComponent c : t.getFirstRow().getChildren()) {
      if (c instanceof Column) {
        Column col = (Column) c;
        result.add(col);
      }
    }
    return result;
  }

  private void encodeBeginAjax(FacesContext context, DataTable t) throws IOException{
    Writer w = context.getResponseWriter();
    if(t.getError() != null || t.getLoadCurrent() != null) {
      ((HttpServletResponse)context.getExternalContext().getResponse()).setContentType("application/json");
      w.append("{\"type\":");
      if(t.getError() != null) {
        w.append("\"error\"");
      }else if(t.getLoadCurrent() != null) {
        w.append("\"progress\"");
      }
      w.append(",\"data\":\"");
      if(t.getError() != null) {
        w.append(
            jsonEscape(t.getError().getMessage())
        );
      }else if(t.getLoadCurrent() != null) {
        int progress = t.getLoadCurrent()*100;
        if(t.getLoadTotal() == 0) {
          progress = 0;
        }else {
          progress/=t.getLoadTotal();
        }
        w.append(""+progress);
      }
    }else {
      encodeBeginGeneral(context, t);
    }

  }

  private void encodeBeginNonAjax(FacesContext context, DataTable t) throws IOException{

    ResponseWriter w = context.getResponseWriter();
    String sOrder = t.getSortOrder();

    importResources(w);

    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_column" + "\" name=\"" + t.getId() + "_column\" value=\"" + sOrder + "\">");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_page" + "\" name=\"" + t.getId() + "_page\" value=\"" + t.getCurrentPage() + "\">");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_pageSize" + "\" name=\"" + t.getId() + "_pageSize\" value=\"" + t.getPageSize() + "\">");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_previousPageSize" + "\" name=\"" + t.getId() + "_previousPageSize\" value=\"" + t.getPageSize() + "\">");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_select" + "\" name=\"" + t.getId() + "_select\" value=\""+toJson(t.getSelectedRows())+"\">");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_selectAll" + "\" name=\"" + t.getId() + "_selectAll\" value=\""+t.isSelectAll()+"\">");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_showSelected" + "\" name=\"" + t.getId() + "_showSelected\" value=\""+t.isShowSelectedOnly()+"\">");

    w.append("\n<div id=\"" + t.getId() + "\">");

    encodeBeginGeneral(context, t);
  }

  private void encodeBeginGeneral(FacesContext context, DataTable t) throws IOException {
    String ctxPath = context.getExternalContext().getRequestContextPath();
    Writer w = context.getResponseWriter();

    List<Column> columns = getColumns(t);
    boolean hasInnerData = t.hasInnerData();

    Locale l = context.getViewRoot() != null ? context.getViewRoot().getLocale() : context.getExternalContext().getRequestLocale();
    ResourceBundle b = ResourceBundle.getBundle(DataTable.class.getCanonicalName(), l);


    w.append("\n<table class=\"eyeline_list\" id=\"" + t.getId() + "_table\" cellspacing=\"1\">");
    if (t.isSelection())
      w.append("\n<col width=\"1%\"/>");
    if (hasInnerData)
      w.append("\n<col width=\"1%\"/>");
    for (Column column : columns) {
      if (column.isRendered())
        w.append("\n<col width=\"" + column.getWidth() + "\" align=\"" + column.getAlign() + "\"/>");
    }

    w.append("\n<thead>");
    if (t.isSelection()) {
      w.append("\n<th class=\"eyeline_ico\" width=\"1px\">");
      if(!t.isDisallowSelectAll() && !t.isShowSelectedOnly()) {
        w.append("<table id=\"select_all_button_"+t.getId()+"\" class=\"select_all_button\"><tr><td>");
      }

      w.append("<div id=\"" + t.getId() + "_check\" class=\"select_page\" selectpage=\"\" " +
          "onclick=\"changeSelectAllPage(this, '"+t.getId()+"');\">&nbsp;</div>");


      if(!t.isDisallowSelectAll() && !t.isShowSelectedOnly()) {
        w.append("</td><td>");
        w.append("<div clicked=\"0\" style=\"cursor: pointer;\" id=\"" + t.getId() + "_check_all\" class=\"select_all_button\" onclick=\"" +
            "    var itemsElm = document.getElementById('select_all_content"+t.getId()+"');" +
            "    var main = document.getElementById('select_all_"+t.getId()+"');" +
            "    var button = document.getElementById('select_all_button_"+t.getId()+"');" +
            "if(this.getAttribute('clicked') == '0') {"+
            "    itemsElm.style.visibility = 'visible';\n" +
            "    this.setAttribute('clicked', '1');" +
            "    button.className='select_all_button_selected';" +
            "}else {"+
            "    itemsElm.style.visibility = 'hidden';\n" +
            "    this.setAttribute('clicked', '0');" +
            "    button.className='select_all_button';" +
            "}"+"\">&nbsp;</div");
        w.append("</td></tr></table>");
        w.append(
            "<div id=\"select_all_content"+t.getId()+"\" class=\"select_all_content\"" +
                " onclick=\"document.getElementById('"+t.getId()+"_check_all').onclick();\">"+
                "   <div style=\"margin: 4px 4px 4px 4px\">" +
                "         <a id=\"select_item_1"+t.getId()+"\" class=\"xi\" href=\"#\" onclick=\"changeSelectAll('"+t.getId()+"', true);\">"+ b.getString("page.select.all")+"</a>\n" +
                "         <br/>" +
                "         <a id=\"unselect_item_1"+t.getId()+"\" class=\"xi\" href=\"#\" onclick=\"changeSelectAll('"+t.getId()+"', false);\">"+b.getString("page.unselect.all")+"</a>\n" +
                "   </div>"+
                "</div>\n");
      }
      w.append("</th>");

    }
    if (hasInnerData)
      w.append("\n<th class=\"eyeline_clickable\" onclick=\"pagedTable" + t.getId() + ".expandAll()\"><div id=\"" + t.getId() + "_expand\" class=\"eyeline_inner_data_closed\">&nbsp;</div></th>");

    for (Column column : columns) {
      if (!column.isRendered())
        continue;
      String classStr = "";
      String sortOrder = column.getName();
      if (t.getSortOrder() != null && t.getSortOrder().endsWith(column.getName())) {
        if (t.getSortOrder().charAt(0) == '-') {
          classStr = "class=\"eyeline_down\"";
          sortOrder = column.getName();
        } else {
          classStr = "class=\"eyeline_up\"";
          sortOrder = '-' + column.getName();
        }
      }
      if (column.isSortable())
        w.append("\n<th><a href=\"#\" onclick=\"pagedTable" + t.getId() + ".setSortOrder('" + sortOrder + "')\" " + classStr + ">" + column.getTitle() + "</a></th>");
      else
        w.append("\n<th>" + (column.getTitle().length() > 0 ? column.getTitle() : "&nbsp;") + "</th>");
    }
    w.append("\n</thead>");

    w.append("\n<tbody>");

  }

  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    DataTable t = (DataTable) component;
    renderChilds = t.getError() == null && t.getLoadCurrent() == null;

    ajax = false;
    if (context instanceof AjaxFacesContext) {
      AjaxFacesContext ctx = (AjaxFacesContext) context;
      if (ctx.getAjaxComponentId().equals(t.getId())) {
        ajax = true;
        ctx.setSkipContent(false);
      }
    }

    if(ajax) {
      encodeBeginAjax(context, t);
    }else {
      encodeBeginNonAjax(context, t);
    }
  }

  @Override
  public boolean getRendersChildren() {
    return true;
  }

  @Override
  public void encodeChildren(FacesContext context, UIComponent component) throws IOException {
    if(renderChilds) {
      super.encodeChildren(context, component);
    }
  }

  private void encodeEndAjax(FacesContext context, DataTable t) throws IOException{

    Writer w = context.getResponseWriter();
    if (t.getError() == null && t.getLoadCurrent() == null) {
      boolean columnFooter = false;
      for (UIComponent ch : t.getFirstRow().getChildren()) {
        if (ch.getFacet("footer") != null) {
          columnFooter = true;
          break;
        }
      }
      if(columnFooter) {
        int currentRow = t.getChildCount() + 1;
        w.append("\n<tr class=\"eyeline_row" + (currentRow & 1) + "\">");
        if(t.isSelection())
          w.append("\n<td>&nbsp;</td>");
        if (t.hasInnerData())
          w.append("\n<td>&nbsp;</td>");
        for (UIComponent ch : t.getFirstRow().getChildren()) {
          Column col = (Column)ch;
          UIComponent footer = ch.getFacet("footer");
          w.append("\n<td align=\"").append(col.getAlign()).append("\">");
          if (footer != null) {
            footer.encodeBegin(context);
            footer.encodeEnd(context);
          }else {
            w.append("&nbsp;");
          }
          w.append("</td>");
        }

        w.append("\n</tr>");

      }else {
        UIComponent footer = t.getFacet("footer");
        int currentRow = t.getChildCount()+1;
        if(footer != null) {
          int colspan = t.getFirstRow().getChildCount();
          if(colspan == 0) {
            colspan = 1;
          }
          w.append("\n<tr  class=\"eyeline_row" + (currentRow & 1) + "\">");
          if(t.isSelection()) {
            w.append("\n<td>&nbsp;</td>");
          }
          w.append("\n<td colspan=\""+colspan+"\">");
          footer.encodeBegin(context);
          footer.encodeEnd(context);
          w.append("</td>");
          w.append("\n</tr>");
        }
      }
      encodeEndGeneral(context, t);
    }else {
      w.append("\"}");
    }
  }

  private void encodeEndNoneAjax(FacesContext context, DataTable t) throws IOException {

    Writer w = context.getResponseWriter();

    String ctxPath = context.getExternalContext().getRequestContextPath();

    encodeEndGeneral(context, t);

    w.append("\n</div>");
    w.append("<table id=\""+t.getId()+"_progress\" class=\"overlay\"><tr><td align=\"center\" valign=\"center\">")
        .append("<div style=\"margin-top:auto;margin-bottom:auto;\">" +
            "<table style=\"width:0%\"><tr>"+
            "<td><div class=\"eyeline_loading\" alt=\"\">&nbsp;</div></td>" +
            "<td><span style=\"font-size:15px\" id=\"" + t.getId() + "_progress_content\"></span></td>" +
            "</tr></table>"+
            "</div>")
        .append("</td></tr></table>");
    w.append("\n</div>");
    w.append("\n<script language=\"javascript\" type=\"text/javascript\">");

    if (t.isUpdateUsingSubmit() == null)
      w.append("\nvar updateUsingSubmit" + t.getId() + "= false;");
    else
      w.append("\nvar updateUsingSubmit" + t.getId() + "= " + t.isUpdateUsingSubmit() + ";");

    ResourceBundle b = ResourceBundle.getBundle(DataTable.class.getCanonicalName(), context.getViewRoot() != null ? context.getViewRoot().getLocale() : context.getExternalContext().getRequestLocale());


    w.append("\npagedTable" + t.getId() + "=new DataTable('" + t.getId() + "',updateUsingSubmit" + t.getId() + ","+(t.getModel() instanceof PreloadableModel)+", \""+b.getString("error.title")+"\");");
    if (t.getAutoUpdate() != null && (t.isUpdateUsingSubmit() == null || !t.isUpdateUsingSubmit())) {
      w.append("\nfunction autoUpdate" + t.getId() + "(){");
      w.append("\n  pagedTable" + t.getId() + ".updateTable();");
      w.append("\n  window.setTimeout(autoUpdate" + t.getId() + "," + t.getAutoUpdate() * 1000 + ");");
      w.append("\n};");
      w.append("\nautoUpdate" + t.getId() + "();");
    } else {
      w.append("\nfunction load" + t.getId() + "(){");
      w.append("\n  pagedTable" + t.getId() + ".updateTable();");
      w.append("\n};");
      w.append("\n  window.onload =load" + t.getId() +";");
    }
    w.append("\n</script>");
  }

  private void encodeEndGeneral(FacesContext context, DataTable t) throws IOException {

    Writer w = context.getResponseWriter();

    String ctxPath = context.getExternalContext().getRequestContextPath();


    w.append("\n</tbody>");
    w.append("\n</table>");


    w.append("<table class=\"eyeline_navbar\" cellspacing=\"1\" cellpadding=\"0\">");
    w.append("<tr>");


    int rowsCount = 0 , selected = 0;
    rowsCount = t.getTotalSize();
    selected = t.getSelectedRows().size();
    if(t.isSelectAll()) {
      selected = rowsCount - selected;
    }
    if(t.isShowSelectedOnly()) {
      rowsCount = selected;
    }

    if (rowsCount > t.getPageSize()) {

      w.append("<td class=\"eyeline_first\"><a href=\"#\" onclick=\"pagedTable" + t.getId() + ".setPage(0)\"><div class=\"eyeline_first\">&nbsp;</div></a></td>");
      if (t.getCurrentPage() > 0)
        w.append("<td class=\"eyeline_prev\"><a href=\"#\" onclick=\"pagedTable" + t.getId() + ".setPage(" + (t.getCurrentPage() - 1) + ")\"><div class=\"eyeline_prev\">&nbsp;</div></a></td>");

      int numberOfPages = rowsCount / t.getPageSize();
      if (t.getPageSize() * numberOfPages == rowsCount)
        numberOfPages--;

      int firstPageNumber = Math.max(t.getCurrentPage() - 5, 0);
      int lastPageNumber = Math.min(t.getCurrentPage() + 5, numberOfPages);

      for (int i = firstPageNumber; i <= lastPageNumber; i++)
        w.append("<td class=\"eyeline_" + (i == t.getCurrentPage() ? "current" : "page") + "\"><a href=\"#\" onclick=\"pagedTable" + t.getId() + ".setPage(" + i + ")\">" + (i + 1) + "</a></td>");

      if (t.getCurrentPage() < numberOfPages)
        w.append("<td class=\"eyeline_next\"><a href=\"#\" onclick=\"pagedTable" + t.getId() + ".setPage(" + (t.getCurrentPage() + 1) + ")\"><div class=\"eyeline_next\">&nbsp;</div></a></td>");

      w.append("<td class=\"eyeline_last\"><a href=\"#\" onclick=\"pagedTable" + t.getId() + ".setPage(" + numberOfPages + ")\"><div class=\"eyeline_last\">&nbsp;</div></a></td>");
    }

    ResourceBundle b = ResourceBundle.getBundle(DataTable.class.getCanonicalName(), context.getViewRoot() != null ? context.getViewRoot().getLocale() : context.getExternalContext().getRequestLocale());

    w.append("<td class=\"eyeline_total\">" + b.getString("total") + ": " + "<span id=\""+t.getId()+"_totalCount\">"+rowsCount + "</span>&nbsp;|&nbsp;");
    if(t.isSelection()) {
      w.append("<a href=\"#\" class=\"xi\" title=\""+b.getString("page.show.choise")+"\" style=\"cursor: pointer;\" onclick=\"pagedTable" + t.getId() + ".onlySelected()\">"+
          "<span style=\"font-size: x-small\">"+
          (t.isShowSelectedOnly() ? b.getString("selected.only"): b.getString("selected")+": ")
          +"<span id=\""+ t.getId()+"_selectedCount\">"
          +(t.isShowSelectedOnly() ? "" : selected)+
          "</span></span>"+
          "</a>&nbsp;|&nbsp;"
      );
    }
    if (t.isPageSizeRendered()) {
      w.append(b.getString("page") + ": ");
      w.append("<select id=\"" + t.getId() + "_pageSizeSelect\" name=\"" + t.getId() + "_pageSizeSelect\" onchange=\"pagedTable" + t.getId() + ".setPageSize(this.options[this.selectedIndex].value)\">");
      w.append("<option value=\"10\" " + (t.getPageSize() == 10 ? "selected" : "") + ">10</option>");
      w.append("<option value=\"20\" " + (t.getPageSize() == 20 ? "selected" : "") + ">20</option>");
      w.append("<option value=\"30\" " + (t.getPageSize() == 30 ? "selected" : "") + ">30</option>");
      w.append("<option value=\"40\" " + (t.getPageSize() == 40 ? "selected" : "") + ">40</option>");
      w.append("<option value=\"50\" " + (t.getPageSize() == 50 ? "selected" : "") + ">50</option>");
    }
    w.append("</select>");
    w.append("</td>");
    w.append("</tr>");
    w.append("</table>");
  }


  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    DataTable t = (DataTable) component;

    if(ajax) {
      encodeEndAjax(context, t);
    }else {
      encodeEndNoneAjax(context, t);
    }

    if (ajax && (context instanceof AjaxFacesContext))
      ((AjaxFacesContext) context).setSkipContent(true);
  }


  private static String toJson(Collection<String> cl) {
    return toJson(new ArrayList<String>(cl).toArray(new String[cl.size()]));
  }


  private static String toJson(String[] strings) {
    StringBuilder res = new StringBuilder();
    res.append('[');
    if(strings != null) {
      boolean first = true;
      for(String s : strings) {
        if(!first) {
          res.append(',');
        }else {
          first = false;
        }
        res.append("&quot;").append(s).append("&quot;");
      }
    }
    res.append(']');
    return res.toString();
  }

  private static String[] toArray(String json) {
    if(json == null || json.length()<=2) {
      return null;
    }
    json = json.substring(1, json.length()-1); //remove []

    String[] ss = json.split(",");
    String[] res = new String[ss.length];

    int i =0;
    for(String s : ss) {
      res[i] = s.substring(1, s.length()-1); //remove ""
      i++;
    }
    return res;
  }


  static String jsonEscape(String s) {
    StringBuilder sb = new StringBuilder();
    for(int i=0;i<s.length();i++){
      char ch=s.charAt(i);
      switch(ch){
        case '"':
          sb.append("\\\"");
          break;
        case '\\':
          sb.append("\\");
          break;
        case '\b':
          sb.append("\\b");
          break;
        case '\f':
          sb.append("\\f");
          break;
        case '\n':
//				sb.append("\\n");
          break;
        case '\r':
          sb.append("\\r");
          break;
        case '\t':
          sb.append("\\t");
          break;
        case '/':
          sb.append("/");
          break;
        default:
          if((ch>='\u0000' && ch<='\u001F') || (ch>='\u007F' && ch<='\u009F') || (ch>='\u2000' && ch<='\u20FF')){
            String ss=Integer.toHexString(ch);
            sb.append("\\u");
            for(int k=0;k<4-ss.length();k++){
              sb.append('0');
            }
            sb.append(ss.toUpperCase());
          }
          else{
            sb.append(ch);
          }
      }
    }
    return sb.toString();
  }
}
