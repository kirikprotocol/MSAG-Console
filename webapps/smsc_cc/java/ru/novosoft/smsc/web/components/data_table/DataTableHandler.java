package ru.novosoft.smsc.web.components.data_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRow;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRowBase;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;

import javax.faces.component.UIComponent;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class DataTableHandler extends ComponentHandler {

  private final TagAttribute value;
  private final TagAttribute autoUpdate;
  private final TagAttribute pageSize;
  private final TagAttribute selectedRows;
  private final TagAttribute updateUsingSubmit;

  public DataTableHandler(ComponentConfig config) {
    super(config);

    value = getRequiredAttribute("value");
    autoUpdate = getAttribute("autoUpdate");
    pageSize = getAttribute("pageSize");
    selectedRows = getAttribute("selectedRows");
    updateUsingSubmit = getAttribute("updateUsingSubmit");
  }

  protected UIComponent createComponent(FaceletContext ctx) {
    DataTable t = new DataTable();
    if (autoUpdate != null)
      t.setAutoUpdate(autoUpdate.getInt(ctx));
    if (pageSize != null)
      t.setPageSize(pageSize.getInt(ctx));
    if (selectedRows != null)
      t.setRowSelection(true);
    if (updateUsingSubmit != null)
      t.setUpdateUsingSubmit(updateUsingSubmit.getBoolean(ctx));

    return t;
  }

  protected void applyNextHandler(com.sun.facelets.FaceletContext ctx, javax.faces.component.UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {

    DataTable t = (DataTable) c;

    DataTableModel m = (DataTableModel) value.getValueExpression(ctx, DataTableModel.class).getValue(ctx);
    t.setModel(m);

    if (t.getSelectedRows().size() > 0 && selectedRows != null)
      selectedRows.getValueExpression(ctx, List.class).setValue(ctx, new ArrayList<String>(t.getSelectedRows()));

    DataTableSortOrder s = null;
    if (t.getSortOrder() != null) {
      boolean asc = t.getSortOrder().charAt(0) == '-';
      s = (asc) ? new DataTableSortOrder(t.getSortOrder().substring(1), true) : new DataTableSortOrder(t.getSortOrder().substring(0), false);
    }


    int startPos = t.getCurrentPage() * t.getPageSize();
    List<DataTableRow> rows = m.getRows(startPos, t.getPageSize(), s);
    ctx.getVariableMapper().setVariable("dataTable", new ConstantExpression(t));

    // Header
    nextHandler.apply(ctx, c);

    // Body
    for (DataTableRow row : rows) {
      ctx.getVariableMapper().setVariable("___currentRow", new ConstantExpression(row));

      nextHandler.apply(ctx, c);

      List<Object> innerRowsData = row.getInnerRows();

      if (innerRowsData != null && !innerRowsData.isEmpty()) {

        List<DataTableRow> innerRows = new ArrayList<DataTableRow>(innerRowsData.size());
        for (Object o : innerRowsData)
          innerRows.add(new DataTableRowBase(row.getId(), o, null));

        ctx.getVariableMapper().setVariable("___innerRow", new ConstantExpression("inner"));
        for (DataTableRow innerRow : innerRows) {
          ctx.getVariableMapper().setVariable("___currentRow", new ConstantExpression(innerRow));
          nextHandler.apply(ctx, c);
        }
        ctx.getVariableMapper().setVariable("___innerRow", null);
      }
    }

    ctx.getVariableMapper().setVariable("dataTable", null);
  }

}
