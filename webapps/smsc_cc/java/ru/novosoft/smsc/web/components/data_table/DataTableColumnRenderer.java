package ru.novosoft.smsc.web.components.data_table;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.io.Writer;

/**
 * @author Artem Snopkov
 */
public class DataTableColumnRenderer extends Renderer {

  private static ThreadLocal<Integer> currentRowNumber = new ThreadLocal<Integer>();

  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {

    DataTableColumn c = (DataTableColumn) component;
    if (c.getRowNum() == -1) // Skip header
      return;

    Writer w = context.getResponseWriter();

    Integer rowNum = currentRowNumber.get();
    if (rowNum == null)
      rowNum = -1;

    DataTable t = (DataTable) c.getParent();
    DataTableRow row = t.getRows().get(c.getRowNum());

    if (c.getRowNum() != rowNum) {
      if (rowNum != -1 && c.getRowNum() > 0) {
        w.append("\n</tr>");

        DataTableRow prevRow = t.getRows().get(c.getRowNum() - 1);

        if (prevRow.getInnerText() != null) {
          w.append("\n<tr class=\"row" + ((c.getRowNum() - 1) & 1) + "\" id=\"innerData" + t.getId() + prevRow.getId() + "\" style=\"display:none\">");
          w.append("\n  <td align=\"left\" colspan=\"" + (t.getColumns().size() + 2) + "\">" + prevRow.getInnerText() + "</td>");
          w.append("\n</tr>");
        }
      }
      w.append("\n<tr class=\"row" + (c.getRowNum() & 1) + "\">");
      if (t.isRowSelection())
        w.append("\n  <td><input class=\"check\" type=\"checkbox\" name=\"" + t.getId() + "_row" + row.getId() + "\" id=\"" + t.getId() + "_rowCheck" + c.getRowNum() + "\"" + "/></td>");
      if (t.hasInnerData()) {
        if (row.getInnerText() != null)
          w.append("\n  <td class=\"clickable\" onclick=\"pagedTable" + t.getId() + ".showRowInnerData('" + t.getId() + row.getId() + "')\"><div id=\"innerDataHeader" + t.getId() + row.getId() + "\" class=\"inner_data_closed\">&nbsp;</div></td>");
        else
          w.append("\n  <td>&nbsp</td>");
      }
      currentRowNumber.set(c.getRowNum());
    }
    w.append("\n  <td align=\"" + c.getAlign() + "\">");
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    DataTableColumn c = (DataTableColumn) component;
    if (c.getRowNum() == -1) // Skip header
      return;

    Writer w = context.getResponseWriter();
    w.append("</td>");
  }
}
