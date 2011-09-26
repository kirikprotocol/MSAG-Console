package mobi.eyeline.util.jsf.components.data_table;

import mobi.eyeline.util.jsf.components.AjaxFacesContext;
import mobi.eyeline.util.jsf.components.HtmlWriter;
import mobi.eyeline.util.jsf.components.MessageUtils;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import static mobi.eyeline.util.jsf.components.JsonUtils.*;

/**
 * User: artem
 * Date: 05.09.11
 */
public class DataTableRenderer extends Renderer {

  private static final String TOTAL = "mobi.eyeline.util.jsf.components.data_table.DataTable.TOTAL";
  private static final String SELECTED = "mobi.eyeline.util.jsf.components.data_table.DataTable.SELECTED";
  private static final String PAGE = "mobi.eyeline.util.jsf.components.data_table.DataTable.PAGE";
  private static final String SELECTED_ONLY = "mobi.eyeline.util.jsf.components.data_table.DataTable.SELECTED.ONLY";
  private static final String SELECT_ALL = "mobi.eyeline.util.jsf.components.data_table.DataTable.PAGE.SELECT.ALL";
  private static final String UNSELECT_ALL = "mobi.eyeline.util.jsf.components.data_table.DataTable.PAGE.UNSELECT.ALL";

  private boolean ajax = false;
  private boolean renderChilds = false;


  public void decode(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) {
    DataTable t = (DataTable) component;
    Map<String, String> reqParams = context.getExternalContext().getRequestParameterMap();
    String column = reqParams.get(t.getId() + "_column");
    if (column != null && column.trim().length() == 0)
      column = null;
    t.setSortOrder(column);


    String page = reqParams.get(t.getId() + "_page");
    if (page != null && page.trim().length() > 0)
      t.setCurrentPage(Integer.parseInt(page) - 1);

    String pageSize = reqParams.get(t.getId() + "_pageSize");
    t.setPageSize(Integer.parseInt(pageSize));

    String select = reqParams.get(t.getId() + "_select");
    t.setSelectedRows(toArray(select));

    String selectAll = reqParams.get(t.getId() + "_selectAll");
    t.setSelectAll(Boolean.parseBoolean(selectAll));

    String showSelected = reqParams.get(t.getId() + "_showSelected");
    t.setShowSelectedOnly(Boolean.parseBoolean(showSelected));

    t.setInternalUpdate(reqParams.containsKey("eyelineComponentUpdate"));
  }

  private static List<Column> getColumns(DataTable t) {
    List<Column> result = new ArrayList<Column>();
    for (UIComponent c : t.getFirstRow().getChildren()) {
      if ((c instanceof Column) && c.isRendered())
        result.add((Column) c);
    }
    return result;
  }

  private void encodeBeginAjax(FacesContext context, DataTable t) throws IOException {
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    if (t.getError() != null || t.getLoadCurrent() != null) {
      ((HttpServletResponse) context.getExternalContext().getResponse()).setContentType("application/json");
      w.a("{\"type\":");
      if (t.getError() != null) {
        w.a("\"error\"");
      } else if (t.getLoadCurrent() != null) {
        w.a("\"progress\"");
      }
      w.a(",\"data\":\"");
      if (t.getError() != null) {
        w.a(
            jsonEscape(t.getError().getMessage())
        );
      } else if (t.getLoadCurrent() != null) {
        int progress = t.getLoadCurrent() * 100;
        if (t.getLoadTotal() == 0) {
          progress = 0;
        } else {
          progress /= t.getLoadTotal();
        }
        w.a("" + progress);
      }
    }
  }

  private int getNumberOfColumns(DataTable t) {
    int columnsNumber = getColumns(t).size();
    if (t.hasInnerData())
      columnsNumber++;
    if (t.isRowsSelectionEnabled())
      columnsNumber++;
    return columnsNumber;
  }

  private void encodeBeginNonAjax(FacesContext context, DataTable t) throws IOException {

    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    String sOrder = t.getSortOrder();

    w.a("\n<input type=\"hidden\" id=\"" + t.getId() + "_column" + "\" name=\"" + t.getId() + "_column\" value=\"" + sOrder + "\">");
    w.a("\n<input type=\"hidden\" id=\"" + t.getId() + "_page" + "\" name=\"" + t.getId() + "_page\" value=\"" + t.getCurrentPage() + "\">");
    w.a("\n<input type=\"hidden\" id=\"" + t.getId() + "_pageSize" + "\" name=\"" + t.getId() + "_pageSize\" value=\"" + t.getPageSize() + "\">");
    w.a("\n<input type=\"hidden\" id=\"" + t.getId() + "_select" + "\" name=\"" + t.getId() + "_select\" value=\"" + toJson(t.getSelectedRows()) + "\">");
    w.a("\n<input type=\"hidden\" id=\"" + t.getId() + "_selectAll" + "\" name=\"" + t.getId() + "_selectAll\" value=\"" + t.isSelectAll() + "\">");
    w.a("\n<input type=\"hidden\" id=\"" + t.getId() + "_showSelected" + "\" name=\"" + t.getId() + "_showSelected\" value=\"" + t.isShowSelectedOnly() + "\">");


    List<Column> columns = getColumns(t);

    boolean hasInnerData = t.hasInnerData();
    boolean isRowsSelectionEnabled = t.isRowsSelectionEnabled();

    String classStr = "eyeline_list";
    if (t.getTableClass() != null)
      classStr += " " + t.getTableClass();

    w.a("\n<table class=\"" + classStr + "\" id=\"" + t.getId() + "\" cellspacing=\"1\">");

    // RENDER <colgroup>
//    w.a("<colgroup>");
//    if (isRowsSelectionEnabled)
//      w.a("\n<col width=\"1%\"/>");
//    if (hasInnerData)
//      w.a("\n<col width=\"1%\"/>");
//    for (Column column : columns)
//      w.a("\n<col width=\"" + column.getWidth() + "\" align=\"" + column.getAlign() + "\"/>");
//    w.a("</colgroup>");

    //RENDER <thead>
    w.a("\n<thead>");
    if (isRowsSelectionEnabled)
      w.a("\n<th id=\"").a(t.getId()).a("_selectAllButton\">").a("</th>");
    if (hasInnerData)
      w.a("\n<th id=\"").a(t.getId()).a("_expandAll\">").a("</th>");
    for (Column column : columns) {
      w.a("\n<th");
      if (column.getColumnClass() != null)
        w.a(" class=\"" + column.getColumnClass() + "\"");
      w.a("><span id=\"" + t.getId() + "_" + column.getName() + "\">" + (column.getTitle().length() > 0 ? column.getTitle() : "&nbsp;") + "</span></th>");
    }
    w.a("\n</thead>");

    // RENDER <tbody>
    int columnsNumber = getNumberOfColumns(t);
    w.a("\n<tbody>");
    for (int i=0; i<t.getPageSize(); i++)
      w.a("<tr class=\"eyeline_row" + (i&1) + "\"><td colspan=\"" + columnsNumber + "\">&nbsp;</td></tr>");
    w.a("\n</tbody>");

    //RENDER <tfoot>
    w.a("<tfoot>");
    w.a("<tr><td id=\"").a(t.getId()).a("_navbar\" colspan=\"").a(columnsNumber+"").a("\"></td></tr>");
    w.a("</tfoot>");
    w.a("\n</table>");

    //RENDER initialization javascript
    StringBuilder columnNames = new StringBuilder();
    for (Column column : columns) {
      if (!column.isSortable())
        continue;
      if (columnNames.length() > 0)
        columnNames.append(',');
      columnNames.append("\"" + t.getId() + "_" + column.getName() + "\"");
    }

    StringBuilder navbarLabels = new StringBuilder();
    navbarLabels.append("'")
        .append(MessageUtils.getMessageString(context, TOTAL)).append("','")
        .append(MessageUtils.getMessageString(context, SELECTED)).append("','")
        .append(MessageUtils.getMessageString(context, PAGE)).append("','")
        .append(MessageUtils.getMessageString(context, SELECTED_ONLY)).append("'");

    StringBuilder selectionLabels = new StringBuilder();
    selectionLabels.append("'")
        .append(MessageUtils.getMessageString(context, SELECT_ALL)).append("','")
        .append(MessageUtils.getMessageString(context, UNSELECT_ALL)).append("'");


    w.a("\n<script language=\"javascript\" type=\"text/javascript\">");
    w.a("\n  var options" + t.getId() + "={};");
    w.a("\n  options" + t.getId() + "[\"navbar\"] = \"" + t.getId() + "_navbar\";");
    w.a("\n  options" + t.getId() + "[\"totalRows\"] = " + 0 + ";");
    w.a("\n  options" + t.getId() + "[\"currentPageNumber\"] = " + (t.getCurrentPage() + 1) + ";");
    w.a("\n  options" + t.getId() + "[\"pageSize\"] = " + t.getPageSize() + ";");
    w.a("\n  options" + t.getId() + "[\"columns\"] = [" + columnNames + "];");
    w.a("\n  options" + t.getId() + "[\"navbarLabels\"] = [" + navbarLabels + "];");
    w.a("\n  options" + t.getId() + "[\"selectionLabels\"] = [" + selectionLabels + "];");
    if (t.getSortOrder() != null)
      w.a("\noptions" + t.getId() + "[\"sortOrder\"] = \"" + t.getId() + "_" + t.getSortOrder() + "\";");
    if (isRowsSelectionEnabled)
      w.a("\noptions" + t.getId() + "[\"selectButton\"] = \"" + t.getId() + "_selectAllButton\";");
    if (hasInnerData)
      w.a("\noptions" + t.getId() + "[\"toggleButton\"] = \"" + t.getId() + "_expandAll\";");

    w.a("\n  var " +t.getId() + "_dataTable = new DataTable(\"" + t.getId() + "\", options" + t.getId() + ");");

    w.a("\n  $(function(){");
    w.a("\n    " + t.getId() + "_dataTable.update()");
    w.a("\n  });");
    w.a("\n</script>");
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

    if (ajax) {
      encodeBeginAjax(context, t);
    } else {
      encodeBeginNonAjax(context, t);
    }
  }

  @Override
  public boolean getRendersChildren() {
    return renderChilds;
  }

  private void encodeEndAjax(FacesContext context, DataTable t) throws IOException {

    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    if (t.getError() == null && t.getLoadCurrent() == null) {
//      int columnsNumber = getNumberOfColumns(t);
//      for (int i=t.getRowsOnPage(); i<t.getPageSize(); i++) {
//        w.a("<tr class=\"eyeline_row" + (i&1) + "\"><td colspan=\"" + columnsNumber + "\">&nbsp;</td></tr>");
//      }

      boolean columnFooter = false;
      for (UIComponent ch : t.getFirstRow().getChildren()) {
        if (ch.getFacet("footer") != null) {
          columnFooter = true;
          break;
        }
      }
      if (columnFooter) {
        int currentRow = t.getChildCount() + 1;
        w.a("\n<tr class=\"eyeline_row" + (currentRow & 1) + "\">");
        if (t.isRowsSelectionEnabled())
          w.a("\n<td>&nbsp;</td>");
        if (t.hasInnerData())
          w.a("\n<td>&nbsp;</td>");
        for (UIComponent ch : t.getFirstRow().getChildren()) {
          Column col = (Column) ch;
          UIComponent footer = ch.getFacet("footer");
          w.a("\n<td");
          if (col.getColumnClass() != null)
            w.a(" class=\"" + col.getColumnClass() + "\"");
          w.a(">");
          if (footer != null) {
            footer.encodeBegin(context);
            footer.encodeEnd(context);
          } else {
            w.a("&nbsp;");
          }
          w.a("</td>");
        }

        w.a("\n</tr>");

      } else {
        UIComponent footer = t.getFacet("footer");
        int currentRow = t.getChildCount() + 1;
        if (footer != null) {
          int colspan = t.getFirstRow().getChildCount();
          if (colspan == 0) {
            colspan = 1;
          }
          w.a("\n<tr  class=\"eyeline_row" + (currentRow & 1) + "\">");
          if (t.isRowsSelectionEnabled()) {
            w.a("\n<td>&nbsp;</td>");
          }
          w.a("\n<td colspan=\"" + colspan + "\">");
          footer.encodeBegin(context);
          footer.encodeEnd(context);
          w.a("</td>");
          w.a("\n</tr>");
        }
      }
    } else {
      w.a("\"}");
    }

    int rowsCount = t.getTotalSize();
    int selected = t.getSelectedRows().size();
    if (t.isSelectAll()) {
      selected = rowsCount - selected;
    }
    if (t.isShowSelectedOnly()) {
      rowsCount = selected;
    }
    ((HttpServletResponse)context.getExternalContext().getResponse()).addHeader("rowsCount", String.valueOf(rowsCount));
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    DataTable t = (DataTable) component;

    if (ajax) {
      encodeEndAjax(context, t);
      if (context instanceof AjaxFacesContext)
        ((AjaxFacesContext) context).setSkipContent(true);
    }
  }
}
