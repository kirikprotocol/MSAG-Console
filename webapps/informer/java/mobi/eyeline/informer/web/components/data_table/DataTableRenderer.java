package mobi.eyeline.informer.web.components.data_table;

import mobi.eyeline.informer.web.components.AjaxFacesContext;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.io.Writer;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class DataTableRenderer extends Renderer {


  private boolean ajax = false;

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

  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    DataTable t = (DataTable) component;

    ajax = false;
    if (context instanceof AjaxFacesContext) {
      AjaxFacesContext ctx = (AjaxFacesContext) context;
      if (ctx.getAjaxComponentId().equals(t.getId())) {
        ajax = true;
        ctx.setSkipContent(false);
      }
    }

    Writer w = context.getResponseWriter();

    String sOrder = t.getSortOrder();
    if (sOrder == null)
      sOrder = "";

    if (!ajax) {
      w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_column" + "\" name=\"" + t.getId() + "_column\" value=\"" + sOrder + "\">");
      w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_page" + "\" name=\"" + t.getId() + "_page\" value=\"" + t.getCurrentPage() + "\">");
      w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_pageSize" + "\" name=\"" + t.getId() + "_pageSize\" value=\"" + t.getPageSize() + "\">");
      w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_previousPageSize" + "\" name=\"" + t.getId() + "_previousPageSize\" value=\"" + t.getPageSize() + "\">");
      w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_previousPageSize" + "\" name=\"" + t.getId() + "_previousPageSize\" value=\"" + t.getPageSize() + "\">");
      w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_select" + "\" name=\"" + t.getId() + "_select\" value=\""+toJson(t.getSelectedRows())+"\">");
      w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_selectAll" + "\" name=\"" + t.getId() + "_selectAll\" value=\""+t.isSelectAll()+"\">");
      w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_showSelected" + "\" name=\"" + t.getId() + "_showSelected\" value=\""+t.isShowSelectedOnly()+"\">");
    }
    String ctxPath = context.getExternalContext().getRequestContextPath();

    List<Column> columns = getColumns(t);
    boolean hasInnerData = t.hasInnerData();


    if (!ajax)
      w.append("\n<div id=\"" + t.getId() + "\">");
    w.append("\n<table class=\"list\" id=\"" + t.getId() + "_table\" cellspacing=\"1\">");
    if (t.isSelection())
      w.append("\n<col width=\"1%\"/>");
    if (hasInnerData)
      w.append("\n<col width=\"1%\"/>");
    for (Column column : columns) {
      if (column.isRendered())
        w.append("\n<col width=\"" + column.getWidth() + "\" align=\"" + column.getAlign() + "\"/>");
    }

    ResourceBundle b = ResourceBundle.getBundle(DataTable.class.getCanonicalName(), context.getViewRoot() != null ? context.getViewRoot().getLocale() : context.getExternalContext().getRequestLocale());

    w.append("\n<thead>");
    if (t.isSelection()) {
      w.append("\n<th class=\"ico\" width=\"1px\">");
      if(!t.isDisallowSelectAll() && !t.isShowSelectedOnly()) {
        w.append(
            "<div id=\"clicked"+t.getId()+"\" clicked=\"0\"></div>"+
            "<div id=\"select_all_"+t.getId()+"\">");
        w.append("<table id=\"select_all_button_"+t.getId()+"\" class=\"select_all_button\"><tr><td>");
      }

      w.append("<div id=\"" + t.getId() + "_check\" src=\"" + ctxPath + "/images/ico16_checked_sa.gif\" class=\"select_page\" selectpage=\"\" " +
          "onclick=\"changeSelectAllPage(this, '"+t.getId()+"');\">&nbsp;</div>");


      if(!t.isDisallowSelectAll() && !t.isShowSelectedOnly()) {
        w.append("</td><td>");
        w.append("<img style=\"cursor: pointer;\" id=\"" + t.getId() + "_check_all\" src=\"" + ctxPath + "/images/list_opened_center.gif\" class=\"ico16\" onclick=\"" +
            "    var itemsElm = document.getElementById('select_all_content"+t.getId()+"');" +
            "    var clicked = document.getElementById('clicked"+t.getId()+"');" +
            "    var main = document.getElementById('select_all_"+t.getId()+"');" +
            "    var button = document.getElementById('select_all_button_"+t.getId()+"');" +
            "if(clicked.getAttribute('clicked') == '0') {"+
            "    itemsElm.style.visibility = 'visible';\n" +
            "    clicked.setAttribute('clicked', '1');" +
            "    button.className='select_all_button_selected';" +
            "}else {"+
            "    itemsElm.style.visibility = 'hidden';\n" +
            "    clicked.setAttribute('clicked', '0');" +
            "    button.className='select_all_button';" +
            "}"+"\"></img>");
        w.append("</td></tr></table>");
        w.append("</div>" +
            "<div id=\"select_all_content"+t.getId()+"\" class=\"select_all_content\"" +
            " onclick=\"document.getElementById('"+t.getId()+"_check_all').onclick();\">"+
            "<div style=\"margin: 4px 4px 4px 4px\"/>" +
            "         <a id=\"select_item_1"+t.getId()+"\" class=\"xi\" href=\"#\" onclick=\"changeSelectAll('"+t.getId()+"', true);\">"+ b.getString("page.select.all")+"</a>\n" +
            "         <br/>" +
            "         <a id=\"unselect_item_1"+t.getId()+"\" class=\"xi\" href=\"#\" onclick=\"changeSelectAll('"+t.getId()+"', false);\">"+b.getString("page.unselect.all")+"</a>\n" +
            "</div>"+
            "     </div>\n");
      }
      w.append("</th>");

    }
    if (hasInnerData)
      w.append("\n<th class=\"clickable\" onclick=\"pagedTable" + t.getId() + ".expandAll()\"><div id=\"" + t.getId() + "_expand\" class=\"inner_data_closed\">&nbsp;</div></th>");

    for (Column column : columns) {
      if (!column.isRendered())
        continue;
      String classStr = "";
      String sortOrder = column.getName();
      if (t.getSortOrder() != null && t.getSortOrder().endsWith(column.getName())) {
        if (t.getSortOrder().charAt(0) == '-') {
          classStr = "class=\"down\"";
          sortOrder = column.getName();
        } else {
          classStr = "class=\"up\"";
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

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    DataTable t = (DataTable) component;

    Writer w = context.getResponseWriter();

    String ctxPath = context.getExternalContext().getRequestContextPath();

    boolean columnFooter = false;
    for (UIComponent ch : t.getFirstRow().getChildren()) {
      if (ch.getFacet("footer") != null) {
        columnFooter = true;
        break;
      }
    }
    if(columnFooter) {
      int currentRow = t.getChildCount() + 1;
      w.append("\n<tr class=\"row" + (currentRow & 1) + "\">");
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
        w.append("\n<tr  class=\"row" + (currentRow & 1) + "\">");
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

    w.append("\n</tbody>");
    w.append("\n</table>");


    DataTableModel m = t.getModel();

    w.append("<table class=\"navbar\" cellspacing=\"1\" cellpadding=\"0\">");
    w.append("<tr>");


    int rowsCount = m.getRowsCount();
    int selected = t.getSelectedRows().size();
    if(t.isSelectAll()) {
      selected = rowsCount - selected;
    }
    if(t.isShowSelectedOnly()) {
      rowsCount = selected;
    }


    if (rowsCount > t.getPageSize()) {

      w.append("<td class=\"first\"><a href=\"#\" onclick=\"pagedTable" + t.getId() + ".setPage(0)\"><img src=\"" + ctxPath + "/images/nav_first.gif\" width=\"12\" height=\"11\"/></a></td>");
      if (t.getCurrentPage() > 0)
        w.append("<td class=\"prev\"><a href=\"#\" onclick=\"pagedTable" + t.getId() + ".setPage(" + (t.getCurrentPage() - 1) + ")\"><img src=\"" + ctxPath + "/images/nav_prev.gif\" width=\"12\" height=\"11\"/></a></td>");

      int numberOfPages = rowsCount / t.getPageSize();
      if (t.getPageSize() * numberOfPages == rowsCount)
        numberOfPages--;

      int firstPageNumber = Math.max(t.getCurrentPage() - 5, 0);
      int lastPageNumber = Math.min(t.getCurrentPage() + 5, numberOfPages);

      for (int i = firstPageNumber; i <= lastPageNumber; i++)
        w.append("<td class=\"" + (i == t.getCurrentPage() ? "current" : "page") + "\"><a href=\"#\" onclick=\"pagedTable" + t.getId() + ".setPage(" + i + ")\">" + (i + 1) + "</a></td>");

      if (t.getCurrentPage() < numberOfPages)
        w.append("<td class=\"next\"><a href=\"#\" onclick=\"pagedTable" + t.getId() + ".setPage(" + (t.getCurrentPage() + 1) + ")\"><img src=\"" + ctxPath + "/images/nav_next.gif\" width=\"12\" height=\"11\"/></a></td>");

      w.append("<td class=\"last\"><a href=\"#\" onclick=\"pagedTable" + t.getId() + ".setPage(" + numberOfPages + ")\"><img src=\"" + ctxPath + "/images/nav_last.gif\" width=\"12\" height=\"11\"/></a></td>");
    }

    ResourceBundle b = ResourceBundle.getBundle(DataTable.class.getCanonicalName(), context.getViewRoot() != null ? context.getViewRoot().getLocale() : context.getExternalContext().getRequestLocale());

    w.append("<td class=\"total\">" + b.getString("total") + ": " + "<span id=\""+t.getId()+"_totalCount\">"+rowsCount + "</span>&nbsp;|&nbsp;");
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

    if (!ajax) {
      w.append("\n</div>");
      w.append("<table id=\""+t.getId()+"_overlay\" class=\"overlay\"><tr><td align=\"center\" valign=\"center\">")
          .append("<img src=\"" + ctxPath + "/images/loading.gif\" alt=\"\" style=\"margin-top:auto;margin-bottom:auto;\"/>")
          .append("</td></tr></table>");
      w.append("\n<script language=\"javascript\" type=\"text/javascript\">");

      if (t.isUpdateUsingSubmit() == null)
        w.append("\nvar updateUsingSubmit" + t.getId() + "= false;");
      else
        w.append("\nvar updateUsingSubmit" + t.getId() + "= " + t.isUpdateUsingSubmit() + ";");

      w.append("\npagedTable" + t.getId() + "=new DataTable('" + t.getId() + "',updateUsingSubmit" + t.getId() + ");");
      if (t.getAutoUpdate() != null && !t.isUpdateUsingSubmit()) {
        w.append("\nfunction autoUpdate" + t.getId() + "(){");
        w.append("\n  pagedTable" + t.getId() + ".updateTable();");
        w.append("\n  window.setTimeout(autoUpdate" + t.getId() + "," + t.getAutoUpdate() * 1000 + ");");
        w.append("\n};");
        w.append("\nautoUpdate" + t.getId() + "();");
      }
      w.append("\n</script>");
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
}
