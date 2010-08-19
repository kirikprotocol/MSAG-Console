package ru.novosoft.smsc.web.components.data_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;

import javax.faces.component.UIComponent;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class DataTableHandler extends ComponentHandler {

  private final TagAttribute value;
  private final TagAttribute autoUpdate;
  private final TagAttribute pageSize;
  private final TagAttribute rowSelection;
  private final TagAttribute updateUsingSubmit;

  public DataTableHandler(ComponentConfig config) {
    super(config);

    value = getRequiredAttribute("value");
    autoUpdate = getAttribute("autoUpdate");
    pageSize = getAttribute("pageSize");
    rowSelection = getAttribute("rowSelection");
    updateUsingSubmit = getAttribute("filter");
  }

  protected UIComponent createComponent(FaceletContext ctx) {
    DataTable t = new DataTable();
    if (autoUpdate != null)
      t.setAutoUpdate(autoUpdate.getInt(ctx));
    if (pageSize != null)
      t.setPageSize(pageSize.getInt(ctx));
    if (rowSelection != null)
      t.setRowSelection(rowSelection.getBoolean(ctx));
    if (updateUsingSubmit != null)
      t.setUpdateUsingSubmit(updateUsingSubmit.getBoolean(ctx));

    return t;
  }

  protected void applyNextHandler(com.sun.facelets.FaceletContext ctx, javax.faces.component.UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {

    DataTable t = (DataTable) c;

    DataTableModel m = (DataTableModel) value.getValueExpression(ctx, DataTableModel.class).getValue(ctx);
    t.setModel(m);
    
    if (t.getSelectedRows().size() > 0) {
      int[] rows = new int[t.getSelectedRows().size()];
      for (int i = 0; i < rows.length; i++)
        rows[i] = t.getSelectedRows().get(i);
      m.setSelectedRows(rows);
    }

    DataTableSortOrder s = null;
    if (t.getSortOrder() != null) {
      boolean asc = t.getSortOrder().charAt(0) == '-';
      if (asc)
        s = new DataTableSortOrder(t.getSortOrder().substring(1), true);
      else
        s = new DataTableSortOrder(t.getSortOrder().substring(0), false);
    }

    ctx.getVariableMapper().setVariable("dataTable", new ConstantExpression(t));

    int startPos = t.getCurrentPage() * t.getPageSize();
    List<DataTableRow> rows = m.getRows(startPos, t.getPageSize(), s);
    int i = 0;
    for (DataTableRow row : rows) {
      t.setCurrentRow(row);
      t.setCurrentRowNum(i + startPos);

      nextHandler.apply(ctx, c);
      i++;
    }
  }

}
