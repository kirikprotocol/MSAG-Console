package mobi.eyeline.util.jsf.components.data_table;

import mobi.eyeline.util.jsf.components.HtmlWriter;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.io.Writer;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class RowRenderer extends Renderer {

  private int getRowNumber(UIComponent row, UIComponent table) {
    int res = 0;
    List<UIComponent> rows = table.getChildren();
    if(rows != null && !rows.isEmpty()) {
      for(UIComponent c : rows) {
        if(c == row) {
          break;
        }
        if(c instanceof Row) {
          if(!((Row) c).isInner()){
            res++;
          }
        }
      }
    }
    return res;
  }

  @Override
  public void encodeBegin(FacesContext context, UIComponent component) throws IOException {
    Row r = (Row) component;

    if (r.isHeader())
      return;

    DataTable t = (DataTable) r.getParent();

    Writer w = context.getResponseWriter();

    int rowNumber = getRowNumber(r, t);

    String rowId = r.getRowId();

    if (r.isInner()) {

      w.append("\n<tr class=\"eyeline_inner\" name=\"innerData" + t.getId() + rowId + "\"" + (r.isOpened() ? "" : " style=\"display:none\"") + ">");
      if (t.isRowsSelectionEnabled())
        w.append("\n<td>&nbsp;</td>");
      w.append("\n<td>&nbsp;</td>");

    } else {

      w.append("\n<tr class=\"eyeline_row" + ((rowNumber+1) & 1) + "\" id=\"" + t.getId() + rowId + "\">");
      if (t.isRowsSelectionEnabled())
     w.append("\n  <td align=\"center\"><input "+ (
            (t.isSelectAll() && !t.isRowSelected(rowId)) || (!t.isSelectAll() && t.isRowSelected(rowId)) ? "CHECKED" : "") +
            ((t.isShowSelectedOnly()) ? " disabled=\"true\"" : "") +
            " onclick=\""+ t.getId() + "_dataTable.selectRow(this.checked, '"+rowId+"')\" class=\"check\" type=\"checkbox\" name=\"" + t.getId() + "_row" + rowId + "\" id=\"" + t.getId() + "_rowCheck" + rowNumber + "\"" + "/></td>");

      if (r.hasInnerData() || r.hasInnerRows())
        w.append("\n  <td class=\"clickable\" onclick=\"" + t.getId() +"_dataTable.expandRow('" + t.getId() + rowId + "')\"><div id=\"innerDataHeader" + t.getId() + rowId + "\" class=\"eyeline_" + (r.isOpened() ? "inner_data_opened" : "inner_data_closed") + "\">&nbsp;</div></td>");
      else if (t.hasInnerData())
        w.append("\n  <td>&nbsp;</td>");

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

  int getColumnsCount(Row r) {
    int res = 0;
    for (UIComponent c : r.getChildren()) {
      if (c instanceof Column)
        res++;
    }
    return res;
  }

  @Override
  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    Row r = (Row) component;

    if (r.isHeader())
      return;

    DataTable t = (DataTable) r.getParent();
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());

    w.a("\n</tr>");

    if (r.hasInnerData()) {
      UIComponent innerDataFacet = r.getFacet("innerData");
      if (innerDataFacet != null) {
        w.a("\n<tr class=\"eyeline_inner\" name=\"innerData" + t.getId() + r.getRowId() + "\"" + (r.isOpened() ? "" : " style=\"display:none\"") + ">");
        w.a("\n  <td align=\"left\" colspan=\"" + (getColumnsCount(r) + 2) + "\">");
        innerDataFacet.encodeBegin(context);
        innerDataFacet.encodeEnd(context);
        w.a("\n  </td>");
        w.a("\n</tr>");
      }
    }
  }
}
