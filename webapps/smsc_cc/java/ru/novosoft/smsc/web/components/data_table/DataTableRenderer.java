package ru.novosoft.smsc.web.components.data_table;

import ru.novosoft.smsc.web.components.AjaxFacesContext;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.ResourceBundle;

/**
 * @author Artem Snopkov
 */
public class DataTableRenderer extends Renderer {

  private boolean ajax = false;

  public static void doDecode(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) {
    DataTable t = (DataTable) component;
    Map<String, String> reqParams = context.getExternalContext().getRequestParameterMap();
    String column = reqParams.get(t.getId() + "_column");
    if (column != null && column.trim().length() == 0)
      column = null;
    t.setSortOrder(column);

    String rowCheckPrefix = t.getId() + "_row";
    t.clearSelectedRows();
    for (Map.Entry<String, String> e : reqParams.entrySet()) {
      if (e.getKey().startsWith(rowCheckPrefix)) {
        String rowId = e.getKey().substring(rowCheckPrefix.length());
        t.addSelectedRow(rowId);
      }
    }

    String page = reqParams.get(t.getId() + "_page");
    if (page != null && page.trim().length() > 0)
      t.setCurrentPage(Integer.parseInt(page));

    String pageSize = reqParams.get(t.getId() + "_pageSize");
    String previousPageSize = reqParams.get(t.getId() + "_previousPageSize");
    if (pageSize != null && pageSize.trim().length() > 0 && previousPageSize != null && previousPageSize.trim().length() > 0)
      t.updatePageSize(Integer.parseInt(previousPageSize), Integer.parseInt(pageSize));

  }

  public void decode(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) {
    doDecode(context, component);
  }

  private List<DataTableColumn> getColumns(DataTable t) {
    List<DataTableColumn> result = new ArrayList<DataTableColumn>();
    Integer rowNum = null;
    for (UIComponent c : t.getChildren()) {
      if (c instanceof DataTableColumn) {
        DataTableColumn col = (DataTableColumn) c;
        if (rowNum == null)
          rowNum = col.getRowNum();
        if (rowNum == col.getRowNum())
          result.add((DataTableColumn) c);
        else
          break;
      }
    }
    return result;
  }

  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    DataTable t = (DataTable) component;

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

    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_column" + "\" name=\"" + t.getId() + "_column\" value=\"" + sOrder + "\"/>");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_page" + "\" name=\"" + t.getId() + "_page\" value=\"" + t.getCurrentPage() + "\"/>");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_pageSize" + "\" name=\"" + t.getId() + "_pageSize\" value=\"" + t.getPageSize() + "\"/>");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_previousPageSize" + "\" name=\"" + t.getId() + "_previousPageSize\" value=\"" + t.getPageSize() + "\"/>");


    String ctxPath = context.getExternalContext().getRequestContextPath();

    List<DataTableColumn> columns = getColumns(t);

    w.append("\n<div id=\"" + t.getId() + "\">");
    w.append("\n<table class=\"list\" cellspacing=\"1\">");
    if (t.isRowSelection())
      w.append("\n<col width=\"1%\"/>");
    if (t.hasInnerData())
      w.append("\n<col width=\"1%\"/>");
    for (DataTableColumn column : columns)
      w.append("\n<col width=\"" + column.getWidth() + "\" align=\"" + column.getAlign() + "\"/>");

    w.append("\n<thead>");
    if (t.isRowSelection())
      w.append("\n<th class=\"ico\"><img src=\"" + ctxPath + "/images/ico16_checked_sa.gif\" class=\"ico16\" onclick=\"javascript:pagedTable" + t.getId() + ".selectAll()\"></th>");
    if (t.hasInnerData())
      w.append("\n<th>&nbsp;</th>");
    for (DataTableColumn column : columns) {
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
        w.append("\n<th><a href=\"javascript:pagedTable" + t.getId() + ".setSortOrder('" + sortOrder + "')\" " + classStr + ">" + column.getTitle() + "</a></th>");
      else
        w.append("\n<th>" + (column.getTitle().length() > 0 ? column.getTitle() : "&nbsp;") + "</th>");
    }
    w.append("\n</thead>");

    w.append("\n<tbody>");
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    DataTable t = (DataTable) component;

    Writer w = context.getResponseWriter();

    if (t.getRows().size() > 0) {
      w.append("\n</tr>");
      int lastRowNum = t.getRows().size() - 1;
      DataTableRow lastRow = t.getRows().get(lastRowNum);
      if (lastRow.getInnerText() != null) {
        w.append("\n<tr class=\"row" + ((lastRowNum) & 1) + "\" id=\"innerData" + t.getId() + lastRow.getId() + "\" style=\"display:none\">");
        w.append("\n  <td align=\"left\" colspan=\"" + (t.getColumns().size() + 2) + "\">" + lastRow.getInnerText() + "</td>");
        w.append("\n</tr>");
      }
    }
    w.append("\n</tbody>");
    w.append("\n</table>");

    w.append("\n<script language=\"javascript\" type=\"text/javascript\">");
    w.append("\npagedTable" + t.getId() + "=new DataTable('" + t.getId() + "'," + t.isUpdateUsingSubmit() + ");");
    if (t.getAutoUpdate() != null)
      w.append("\npagedTable" + t.getId() + ".autoUpdateTable(" + t.getAutoUpdate() * 1000 + ");");
    w.append("\n</script>");

    DataTableModel m = t.getModel();
    String ctxPath = context.getExternalContext().getRequestContextPath();

    w.append("<table class=\"navbar\" cellspacing=\"1\" cellpadding=\"0\">");
    if (m.getRowsCount() > t.getPageSize()) {
      w.append("<tr>");
      w.append("<td class=\"first\"><a href=\"javascript:pagedTable" + t.getId() + ".setPage(0)\"><img src=\"" + ctxPath + "/images/nav_first.gif\" width=\"12\" height=\"11\"></a></td>");
      if (t.getCurrentPage() > 0)
        w.append("<td class=\"prev\"><a href=\"javascript:pagedTable" + t.getId() + ".setPage(" + (t.getCurrentPage() - 1) + ")\"><img src=\"" + ctxPath + "/images/nav_prev.gif\" width=\"12\" height=\"11\"></a></td>");

      int numberOfPages = m.getRowsCount() / t.getPageSize();
      if (t.getPageSize() * numberOfPages == m.getRowsCount())
        numberOfPages--;

      for (int i = 0; i <= numberOfPages; i++)
        w.append("<td class=\"" + (i == t.getCurrentPage() ? "current" : "page") + "\"><a href=\"javascript:pagedTable" + t.getId() + ".setPage(" + i + ")\">" + (i + 1) + "</a></td>");

      if (t.getCurrentPage() < numberOfPages)
        w.append("<td class=\"next\"><a href=\"javascript:pagedTable" + t.getId() + ".setPage(" + (t.getCurrentPage() + 1) + ")\"><img src=\"" + ctxPath + "/images/nav_next.gif\" width=\"12\" height=\"11\"></a></td>");

      w.append("<td class=\"last\"><a href=\"javascript:pagedTable" + t.getId() + ".setPage(" + numberOfPages + ")\"><img src=\"" + ctxPath + "/images/nav_last.gif\" width=\"12\" height=\"11\"></a></td>");
    }

    ResourceBundle b = ResourceBundle.getBundle(DataTable.class.getCanonicalName(), context.getExternalContext().getRequestLocale());

    w.append("<td class=\"total\">" + b.getString("total") + ": " + m.getRowsCount() + "&nbsp;" + b.getString("page") + ": ");
    w.append("<select id=\"" + t.getId() + "_pageSizeSelect\" name=\"" + t.getId() + "_pageSizeSelect\" onchange=\"javascript:pagedTable" + t.getId() + ".setPageSize(this.options[this.selectedIndex].value)\">");
    w.append("<option value=\"10\" " + (t.getPageSize() == 10 ? "selected" : "") + ">10</option>");
    w.append("<option value=\"20\" " + (t.getPageSize() == 20 ? "selected" : "") + ">20</option>");
    w.append("<option value=\"30\" " + (t.getPageSize() == 30 ? "selected" : "") + ">30</option>");
    w.append("<option value=\"40\" " + (t.getPageSize() == 40 ? "selected" : "") + ">40</option>");
    w.append("<option value=\"50\" " + (t.getPageSize() == 50 ? "selected" : "") + ">50</option>");
    w.append("</select>");
    w.append("</td>");
    w.append("</tr>");
    w.append("</table>");
    w.append("\n</div>");

    if (ajax && (context instanceof AjaxFacesContext))
      ((AjaxFacesContext) context).setSkipContent(true);
  }


}
