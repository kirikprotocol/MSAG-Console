package mobi.eyeline.util.jsf.components.data_table;

import mobi.eyeline.util.jsf.components.AjaxFacesContext;
import mobi.eyeline.util.jsf.components.ResourceUtils;
import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
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

     ResponseWriter w = context.getResponseWriter();

     String sOrder = t.getSortOrder();
     if (sOrder == null)
       sOrder = "";

     if (!ajax) {
       importResources(w);
       w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_column" + "\" name=\"" + t.getId() + "_column\" value=\"" + sOrder + "\">");
       w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_page" + "\" name=\"" + t.getId() + "_page\" value=\"" + t.getCurrentPage() + "\">");
       w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_pageSize" + "\" name=\"" + t.getId() + "_pageSize\" value=\"" + t.getPageSize() + "\">");
       w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_previousPageSize" + "\" name=\"" + t.getId() + "_previousPageSize\" value=\"" + t.getPageSize() + "\">");
       w.append("\n<input type=\"hidden\" id=\"" + t.getId() + "_previousPageSize" + "\" name=\"" + t.getId() + "_previousPageSize\" value=\"" + t.getPageSize() + "\">");
     }

     List<Column> columns = getColumns(t);
     boolean hasInnerData = t.hasInnerData();


     if (!ajax)
       w.append("\n<div id=\"" + t.getId() + "\">");
     w.append("\n<table class=\"eyeline_list\" id=\"" + t.getId() + "_table\" cellspacing=\"1\">");
     if (t.isRowSelection())
       w.append("\n<col width=\"1%\"/>");
     if (hasInnerData)
       w.append("\n<col width=\"1%\"/>");
     for (Column column : columns) {
       if (column.isRendered())
         w.append("\n<col width=\"" + column.getWidth() + "\" align=\"" + column.getAlign() + "\"/>");
     }

     w.append("\n<thead>");
     if (t.isRowSelection())
       w.append("\n<th class=\"eyeline_check_all_ico\" onclick=\"pagedTable" + t.getId() + ".selectAll()\"/>" +
           "&nbsp;</th>");
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

   public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
     DataTable t = (DataTable) component;

     Writer w = context.getResponseWriter();

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
       if(t.isRowSelection())
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
         if(t.isRowSelection()) {
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

     w.append("<table class=\"eyeline_navbar\" cellspacing=\"1\" cellpadding=\"0\">");
     w.append("<tr>");
     int rowsCount = m.getRowsCount();
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

     w.append("<td class=\"eyeline_total\">" + b.getString("total") + ": " + rowsCount + "&nbsp;");
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
       w.append("<table id=\""+t.getId()+"_overlay\" class=\"eyeline_overlay\"><tr><td align=\"center\" valign=\"center\">")
        .append("<div class=\"eyeline_loading\">&nbsp;</div>")
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


}
