package mobi.eyeline.util.jsf.components.data_table;

import com.sun.facelets.FaceletContext;
import com.sun.facelets.tag.TagAttribute;
import com.sun.facelets.tag.jsf.ComponentConfig;
import com.sun.facelets.tag.jsf.ComponentHandler;
import mobi.eyeline.util.jsf.components.data_table.model.*;

import javax.el.ValueExpression;
import javax.faces.FacesException;
import javax.faces.component.UIComponent;
import java.io.IOException;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class DataTableHandler extends ComponentHandler {

  private final TagAttribute value;
  private final TagAttribute var;

  public DataTableHandler(ComponentConfig config) {
    super(config);

    value = getRequiredAttribute("value");
    var = getAttribute("var");
  }

  protected void applyNextHandler(com.sun.facelets.FaceletContext ctx, javax.faces.component.UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {
    if (!c.isRendered())
      return;

    final DataTable t = (DataTable) c;
    String tid = t.getId();

    DataTableModel m = (DataTableModel) value.getValueExpression(ctx, DataTableModel.class).getValue(ctx);
    if (t.getSelectedRowsExpression() != null && !(m instanceof ModelWithObjectIds))
      throw new FacesException("Model should implement ModelWithObjectIds interface.");

    ctx.getVariableMapper().setVariable("___tid", new ConstantExpression(tid));

    // Header
    nextHandler.apply(ctx, c);

    if (t.getSortOrder() == null)
      setDefaultSortOrder(t);

    if (!t.isInternalUpdate()) {
      t.setSelectAll(false);
      t.setSelectedRows(null);
      t.setShowSelectedOnly(false);
      return;
    }

    List rows;
    try {
      rows = loadRows(t);
      t.setTotalSize(t.getModel().getRowsCount());
      t.setRowsOnPage(rows.size());
    } catch (ModelException e) {
      t.setError(e);
      rows = Collections.emptyList();
    }

    handleRows(ctx, t, rows);
  }

  private List loadRows(final DataTable t) throws ModelException {
    DataTableSortOrder s = null;
    String sortOrderStr = t.getSortOrder();
    if (sortOrderStr != null) {
      if (sortOrderStr.charAt(0) == '-')
        s = new DataTableSortOrder(sortOrderStr.substring(1), true);
      else
        s = new DataTableSortOrder(sortOrderStr.substring(0), false);
    }

    int startPos = t.getCurrentPage() * t.getPageSize();

    List rows = new LinkedList();

    if (t.getModel() instanceof PreloadableModel) {
      LoadListener listener = ((PreloadableModel) t.getModel()).prepareRows(startPos, t.getPageSize(), s);
      if (listener != null) {
        t.setLoadCurrent(listener.getCurrent());
        t.setLoadTotal(listener.getTotal());
        t.setError(listener.getLoadError());
        return rows;
      }
    }
    load(rows, t, startPos, s);
    return rows;
  }

  private void load(List rows, DataTable t, int startPos, DataTableSortOrder s) throws ModelException {
    getRows(rows, t, startPos, s);
    if (rows.isEmpty() && t.getCurrentPage() > 0) {
      t.setCurrentPage(0);
      getRows(rows, t, 0, s);
    }
  }

  @SuppressWarnings({"unchecked"})
  private void getRows(List rows, DataTable t, int startPos, DataTableSortOrder s) throws ModelException {
    if (!t.isShowSelectedOnly()) {
      rows.addAll(t.getModel().getRows(startPos, t.getPageSize(), s));
    } else {
      loadSelectedRows(startPos, t, s, rows);
    }
  }

  @SuppressWarnings({"unchecked"})
  private void loadSelectedRows(int startPos, DataTable t, DataTableSortOrder s, List rows) throws ModelException {
    Set<String> ids = new HashSet<String>(t.getSelectedRows());
    int i = 0;
    int c = 0;
    ModelWithObjectIds ident = (ModelWithObjectIds) t.getModel();
    List _rows;
    do {
      _rows = t.getModel().getRows(i * 1000, 1000, s);
      for (Object r : _rows) {
        String id = ident.getId(r);
        if ((t.isSelectAll() && !ids.contains(id)) || (!t.isSelectAll() && ids.contains(id))) {
          c++;
          if (c > startPos) {
            rows.add(r);
            if (rows.size() == t.getPageSize()) {
              return;
            }
          }
        }
      }
      i++;
    } while (_rows.size() == 1000 && (t.isSelectAll() || rows.size() < ids.size()));
  }


  private void setDefaultSortOrder(DataTable t) throws IOException {
    for (UIComponent col : t.getFirstRow().getChildren()) {
      if (!(col instanceof Column))
        continue;

      Column column = (Column) col;
      if (column.getDefaultSortOrder() != null) {
        if (column.getDefaultSortOrder().equalsIgnoreCase("asc"))
          t.setSortOrder("-" + column.getName());
        else if (column.getDefaultSortOrder().equalsIgnoreCase("desc"))
          t.setSortOrder(column.getName());
      }
    }
  }

  private void handleRows(FaceletContext ctx, DataTable t, List rows) throws IOException {
    String tid = t.getId();
    ctx.getVariableMapper().setVariable(tid + "___var", new ConstantExpression(var.getValue()));
    ctx.getVariableMapper().setVariable(tid + "___dataTable", new ConstantExpression(t));

    for (Object row : rows) {
      ctx.getVariableMapper().setVariable(tid + "___currentRow", new ConstantExpression(row));

      nextHandler.apply(ctx, t);

      ValueExpression innerRowsExpr = ctx.getVariableMapper().resolveVariable(tid + "___innerRows");
      if (innerRowsExpr != null) {
        List innerRowsData = (List) innerRowsExpr.getValue(ctx);

        if (innerRowsData != null && !innerRowsData.isEmpty()) {

          ctx.getVariableMapper().setVariable(tid + "___innerRow", new ConstantExpression("inner"));
          for (Object innerRow : innerRowsData) {
            ctx.getVariableMapper().setVariable(tid + "___currentRow", new ConstantExpression(innerRow));
            nextHandler.apply(ctx, t);
          }
          ctx.getVariableMapper().setVariable(tid + "___innerRow", null);
        }
      }
    }
  }

}