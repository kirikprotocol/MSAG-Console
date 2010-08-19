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

    Writer w = context.getResponseWriter();

    Integer rowNum = currentRowNumber.get();
    if (rowNum == null)
      rowNum = -1;

    DataTableColumn c = (DataTableColumn) component;
    DataTable t = (DataTable)c.getParent();
    if (c.getRowNum() != rowNum) {
      if (rowNum != -1)
        w.append("\n</tr>");
      w.append("<tr class=\"row" + (c.getRowNum() & 1) + "\">");
      if (t.isRowSelection())
        w.append("\n  <td><input class=\"check\" type=\"checkbox\" name=\"" + t.getId() + "_rowCheck" + c.getRowNum() + "\" id=\"" + t.getId() + "_rowCheck" + c.getRowNum() + "\"" + "/></td>");
      currentRowNumber.set(c.getRowNum());
    }
    w.append("<td align=\"" + c.getAlign() + "\">");
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    Writer w = context.getResponseWriter();
    w.append("</td>");
  }
}
