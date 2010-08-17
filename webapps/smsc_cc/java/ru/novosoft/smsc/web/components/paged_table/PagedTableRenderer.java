package ru.novosoft.smsc.web.components.paged_table;

import ru.novosoft.smsc.web.components.AjaxRenderer;

import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Map;
import java.util.ResourceBundle;

/**
 * @author Artem Snopkov
 */
public class PagedTableRenderer extends AjaxRenderer {

  public void decode(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) {
    PagedTable t = (PagedTable) component;
    Map<String, String> reqParams = context.getExternalContext().getRequestParameterMap();
    String column = reqParams.get(t.getId() + "_column");
    if (column != null && column.trim().length() == 0)
      column = null;
    t.setSortOrder(column);

    String rowCheckPrefix = t.getId() + "_rowCheck";
    t.clearSelectedRows();
    for (Map.Entry<String, String> e : reqParams.entrySet()) {
      if (e.getKey().startsWith(rowCheckPrefix)) {
        int rowNum = Integer.parseInt(e.getKey().substring(rowCheckPrefix.length()));
        t.addSelectedRow(rowNum + t.getCurrentPage() * t.getPageSize());
      }
    }

    String page = reqParams.get(t.getId() + "_page");
    if (page != null && page.trim().length() > 0)
      t.setCurrentPage(Integer.parseInt(page));

    String pageSize = reqParams.get(t.getId() + "_pageSize");
    String previousPageSize = reqParams.get(t.getId() + "_previousPageSize");
    if (pageSize != null && pageSize.trim().length() > 0 && previousPageSize != null && previousPageSize.trim().length() > 0)
      t.updatePageSize(Integer.parseInt(previousPageSize), Integer.parseInt(pageSize));

    String selectedColumn = reqParams.get(t.getId() + "_selectedColumn");
    if (selectedColumn != null && selectedColumn.trim().length() > 0)
      t.setSelectedColumn(selectedColumn);

    String selectedRow = reqParams.get(t.getId() + "_selectedRow");
    if (selectedRow != null && selectedRow.trim().length() > 0)
      t.setSelectedRow(Integer.parseInt(selectedRow));
  }

  public void _encodeBegin(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws IOException {
    PagedTable t = (PagedTable) component;

    Writer w = context.getResponseWriter();

    String sortOrder = t.getSortOrder();
    if (sortOrder == null)
      sortOrder = "";

    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_column" + "\" name=\"" + t.getId() + "_column\" value=\"" + sortOrder + "\"/>");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_page" + "\" name=\"" + t.getId() + "_page\" value=\"\"/>");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_pageSize" + "\" name=\"" + t.getId() + "_pageSize\" value=\"" + t.getPageSize() + "\"/>");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_previousPageSize" + "\" name=\"" + t.getId() + "_previousPageSize\" value=\"" + t.getPageSize() + "\"/>");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_selectedColumn" + "\" name=\"" + t.getId() + "_selectedColumn\" value=\"\"/>");
    w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_selectedRow" + "\" name=\"" + t.getId() + "_selectedRow\" value=\"\"/>");

    if (t.isHasFilter()) {

      ResourceBundle b = ResourceBundle.getBundle(PagedTable.class.getCanonicalName(), context.getExternalContext().getRequestLocale());

      w.append("<div id=\"" + t.getId() + "_filter\">");
      w.append("<table class=\"page_menu\" width=\"100%\">");
      w.append("<tbody><tr>");
      w.append("<td align=\"left\" width=\"10%\">");
      w.append(b.getString("filter") + ":");
      w.append("</td>");
      w.append("<td align=\"left\" width=\"90%\">");
      w.append("&nbsp;");
      w.append("</td>");
      w.append("<td width=\"1px\">&nbsp;</td><td width=\"1px\"><a href=\"javascript:pagedTable" + t.getId() + ".updateTable()\">" + b.getString("apply") + "</a></td>");
      w.append("</tr>");
      w.append("</tbody></table>");
      w.append("<hr/>");
    }
  }

  public void encodeAjaxPart(FacesContext ctx, ResponseWriter w, UIComponent component) throws IOException {

    String ctxPath = ctx.getExternalContext().getRequestContextPath();

    PagedTable t = (PagedTable) component;

    w.append("\n<table class=\"list\" cellspacing=\"1\">");
    if (t.isAllowCheckBoxes())
      w.append("\n<col width=\"1%\"/>");
    for (Column column : t.getColumns())
      w.append("\n<col width=\"" + column.getWidth() + "%\" align=\"" + column.getAlign() + "\"/>");

    w.append("\n<thead>");
    if (t.isAllowCheckBoxes())
      w.append("\n<th class=\"ico\"><img src=\"" + ctxPath + "/images/ico16_checked_sa.gif\" class=\"ico16\" onclick=\"javascript:pagedTable" + t.getId() + ".selectAll()\"></th>");

    for (Column column : t.getColumns()) {
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
        w.append("\n<th>" + column.getTitle() + "</th>");
    }
    w.append("\n</thead>");

    PagedTableSortOrder s = null;
    if (t.getSortOrder() != null) {
      boolean asc = t.getSortOrder().charAt(0) == '-';
      if (asc)
        s = new PagedTableSortOrder(t.getSortOrder().substring(1), true);
      else
        s = new PagedTableSortOrder(t.getSortOrder().substring(0), false);
    }

    PagedTableModel m = t.getModel();
    if (m != null) {
      w.append("\n<tbody>");
      int i = 0;
      int startPos = t.getCurrentPage() * t.getPageSize();
      for (PagedTableRow row : m.getRows(startPos, t.getPageSize(), s)) {
        w.append("\n<tr class=\"row" + (i & 1) + "\">");

        if (t.isAllowCheckBoxes())
          w.append("\n  <td><input class=\"check\" type=\"checkbox\" name=\"" + t.getId() + "_rowCheck" + i + "\" id=\"" + t.getId() + "_rowCheck" + i + "\"" + (t.getSelectedRows().contains(startPos + i) ? "checked" : "") + "/></td>");

        for (Column column : t.getColumns()) {
          w.append("\n  <td>");
          if (column.getActionListener() != null)
            w.append("<a href=\"javascript:pagedTable" + t.getId() + ".selectElement('" + column.getName() + "','" + (i + startPos) + "')\">");


          if (column instanceof TextColumn) {
            TextColumn tc = (TextColumn) column;
            if (tc.getTextColor() != null)
              w.append("<span style=\"color:" + tc.getTextColor() + "\">");
            w.append(row.getData(column.getName()) + "");
            if (tc.getTextColor() != null)
              w.append("</span>");
          } else {
            ImageColumn ic = (ImageColumn) column;
            Object data = row.getData(column.getName());
            if (data != null) {
              String image = ic.getValueMapping().get(data.toString());
              if (image != null)
                w.append("<img src=\"" + ctxPath + image + "\"/>");
            }
          }
          if (column.getActionListener() != null)
            w.append("</a>");
          w.append("</td>");
        }

        w.append("\n</tr>");
        i++;
      }
      w.append("\n</tbody>");
    } else
      System.out.println("MODEL IS NULL");
    w.append("\n</table>");


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

    ResourceBundle b = ResourceBundle.getBundle(PagedTable.class.getCanonicalName(), ctx.getExternalContext().getRequestLocale());

    w.append("<td class=\"total\">" + b.getString("total") + ": " + m.getRowsCount() + "&nbsp;" + b.getString("page") + ": ");
    w.append("<select id=\"" + t.getId() + "_pageSizeSelect\" name=\"" + t.getId() + "_pageSizeSelect\" onchange=\"javascript:pagedTable" + t.getId() + ".setPageSize(this.options[this.selectedIndex].value)\">");
    w.append("<option value=\"10\" " + (t.getPageSize() == 10 ? "selected" : "") + ">10</option>");
    w.append("<option value=\"20\" " + (t.getPageSize() == 20 ? "selected" : "") + ">20</option>");
    w.append("<option value=\"30\" " + (t.getPageSize() == 30 ? "selected" : "") + ">30</option>");
    w.append("<option value=\"40\" " + (t.getPageSize() == 40 ? "selected" : "") + ">40</option>");
    w.append("<option value=\"50\" " + (t.getPageSize() == 50 ? "selected" : "") + ">50</option>");
    w.append("<option value=\"" + m.getRowsCount() + "\" " + (t.getPageSize() == m.getRowsCount() ? "selected" : "") + ">" + b.getString("all") + "</option>");
    w.append("</select>");
    w.append("</td>");
    w.append("</tr>");
    w.append("</table>");
  }

  public void _encodeEnd(FacesContext context, UIComponent component) throws IOException {
    PagedTable t = (PagedTable) component;

    Writer w = context.getResponseWriter();

    if (t.isHasFilter())
      w.append("<div>");

    w.append("\n<div id=\"" + t.getId() + "\">");
    encodeAjaxPart(context, context.getResponseWriter(), component);
    w.append("\n</div>");


    String requestUrl = context.getExternalContext().getRequestContextPath() + context.getExternalContext().getRequestServletPath();

    w.append("\n<script language=\"javascript\" type=\"text/javascript\">");
    w.append("\npagedTable" + t.getId() + "=new PagedTable('" + t.getId() + "','" + requestUrl + "');");
    if (t.getAutoUpdate() != null)
      w.append("\npagedTable" + t.getId() + ".autoUpdateTable(" + t.getAutoUpdate() * 1000 + ");");
    w.append("\n</script>");

  }
}
