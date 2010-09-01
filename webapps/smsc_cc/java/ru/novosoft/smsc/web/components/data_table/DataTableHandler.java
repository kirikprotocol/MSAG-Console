package ru.novosoft.smsc.web.components.data_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRow;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRowBase;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;

import javax.el.ValueExpression;
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
  private final TagAttribute var;

  public DataTableHandler(ComponentConfig config) {
    super(config);

    value = getRequiredAttribute("value");
    autoUpdate = getAttribute("autoUpdate");
    pageSize = getAttribute("pageSize");
    selectedRows = getAttribute("selectedRows");
    updateUsingSubmit = getAttribute("updateUsingSubmit");
    var = getAttribute("var");
  }

  protected UIComponent createComponent(FaceletContext ctx) {
    DataTable t = new DataTable();
    if (autoUpdate != null)
      t.setAutoUpdate(autoUpdate.getInt(ctx));
    if (pageSize != null)
      t.setPageSize(pageSize.getInt(ctx));
    if (selectedRows != null)
      t.setSelectedRowsExpression(selectedRows.getValueExpression(ctx, List.class));
    if (updateUsingSubmit != null)
      t.setUpdateUsingSubmit(updateUsingSubmit.getBoolean(ctx));

    return t;
  }

  protected void applyNextHandler(com.sun.facelets.FaceletContext ctx, javax.faces.component.UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {

    DataTable t = (DataTable) c;

    DataTableModel m = (DataTableModel) value.getValueExpression(ctx, DataTableModel.class).getValue(ctx);
    t.setModel(m);

    // Header
    nextHandler.apply(ctx, c);

    if (t.getSortOrder() == null) {
      for (UIComponent col : t.getFirstRow().getChildren()) {
        if (col instanceof Column) {
          Column column = (Column)col;
          if (column.getDefaultSortOrder() != null) {
            if (column.getDefaultSortOrder().equalsIgnoreCase("asc"))
              t.setSortOrder("-" + column.getName());
            else if (column.getDefaultSortOrder().equalsIgnoreCase("desc"))
              t.setSortOrder(column.getName());
          }
        }
      }
    }

    DataTableSortOrder s = null;
    if (t.getSortOrder() != null) {
      boolean asc = t.getSortOrder().charAt(0) == '-';
      s = (asc) ? new DataTableSortOrder(t.getSortOrder().substring(1), true) : new DataTableSortOrder(t.getSortOrder().substring(0), false);
    }

    int startPos = t.getCurrentPage() * t.getPageSize();
    List rows = m.getRows(startPos, t.getPageSize(), s);

    // Body
    ctx.getVariableMapper().setVariable("___var", new ConstantExpression(var.getValue()));
    for (Object row : rows) {
      ctx.getVariableMapper().setVariable("___currentRow", new ConstantExpression(row));

      nextHandler.apply(ctx, c);

      ValueExpression innerRowsExpr = ctx.getVariableMapper().resolveVariable("___innerRows");
      if (innerRowsExpr != null) {
        List innerRowsData = (List) innerRowsExpr.getValue(ctx);

        if (innerRowsData != null && !innerRowsData.isEmpty()) {

          ctx.getVariableMapper().setVariable("___innerRow", new ConstantExpression("inner"));
          for (Object innerRow : innerRowsData) {
            ctx.getVariableMapper().setVariable("___currentRow", new ConstantExpression(innerRow));
            nextHandler.apply(ctx, c);
          }
          ctx.getVariableMapper().setVariable("___innerRow", null);
        }
      }


    }

  }

}
