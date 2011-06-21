package mobi.eyeline.informer.web.components.data_table;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.io.Writer;

/**
 * @author Artem Snopkov
 */
public class RowRenderer extends Renderer {

  private int rowNumber;

  @Override
  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    Row r = (Row) component;
    DataTable t = (DataTable) r.getParent();

    if (t.getFirstRow() == r)
      rowNumber = 0;

    Writer w = context.getResponseWriter();

    if (r.isHeader())
      return;

    String rowId = r.getRowId();

    if (r.isInner()) {

      w.append("\n<tr class=\"inner\" name=\"innerData" + t.getId() + rowId + "\"" + (r.isOpened() ? "" : " style=\"display:none\"") + ">");
      if (t.isSelection())
        w.append("\n<td>&nbsp;</td>");
      w.append("\n<td>&nbsp;</td>");

    } else {

      w.append("\n<tr class=\"row" + (rowNumber & 1) + "\" id=\"" + t.getId() + rowId + "\">");
      if (t.isSelection())
        w.append("\n  <td align=\"center\"><input "+ (
            (t.isSelectAll() && !t.isSelected(rowId)) || (!t.isSelectAll() && t.isSelected(rowId)) ? "CHECKED" : "") +
            ((t.isShowSelectedOnly()) ? " disabled=\"true\"" : "") +
            " onclick=\"changeSelect(this.checked, '"+rowId+"', '"+t.getId()+"')\" class=\"check\" type=\"checkbox\" name=\"" + t.getId() + "_row" + rowId + "\" id=\"" + t.getId() + "_rowCheck" + rowNumber + "\"" + "/></td>");

      if (r.hasInnerData() || r.hasInnerRows())
        w.append("\n  <td class=\"clickable\" onclick=\"pagedTable" + t.getId() + ".expandRow('" + t.getId() + rowId + "')\"><div id=\"innerDataHeader" + t.getId() + rowId + "\" class=\"" + (r.isOpened() ? "inner_data_opened" : "inner_data_closed") + "\">&nbsp;</div></td>");
      else if (t.hasInnerData())
        w.append("\n  <td>&nbsp;</td>");

      rowNumber++;
    }
  }

  public void encodeChildren(javax.faces.context.FacesContext context, javax.faces.component.UIComponent component) throws java.io.IOException {
    Row r = (Row) component;
    if (!r.isHeader())
      super.encodeChildren(context, component);
  }

  public boolean getRendersChildren() {
    return true;
  }

  @Override
  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    Row r = (Row) component;

    if (r.isHeader())
      return;

    DataTable t = (DataTable) r.getParent();
    Writer w = context.getResponseWriter();

    w.append("\n</tr>");

    if (r.hasInnerData()) {
      UIComponent innerDataFacet = r.getFacet("innerData");
      if (innerDataFacet != null) {
        w.append("\n<tr class=\"inner\" name=\"innerData" + t.getId() + r.getRowId() + "\"" + (r.isOpened() ? "" : " style=\"display:none\"") + ">");
        w.append("\n  <td align=\"left\" colspan=\"" + (r.getColumnsCount() + 2) + "\">");

        innerDataFacet.encodeBegin(context);
        innerDataFacet.encodeEnd(context);
        w.append("\n  </td>");
        w.append("\n</tr>");
      }
    }
  }
}
