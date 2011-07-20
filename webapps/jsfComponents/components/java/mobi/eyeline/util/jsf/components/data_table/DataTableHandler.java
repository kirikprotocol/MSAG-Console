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
  private final TagAttribute autoUpdate;

  private final TagAttribute pageSize;
  private final TagAttribute pageSizeRendered;
  private final TagAttribute selectedRows;
  private final TagAttribute updateUsingSubmit;
  private final TagAttribute var;
  private final TagAttribute disallowSelectAll;

  public DataTableHandler(ComponentConfig config) {
    super(config);

    value = getRequiredAttribute("value");
    autoUpdate = getAttribute("autoUpdate");
    pageSize = getAttribute("pageSize");
    pageSizeRendered = getAttribute("pageSizeRendered");
    selectedRows = getAttribute("selectedRows");
    updateUsingSubmit = getAttribute("updateUsingSubmit");
    disallowSelectAll = getAttribute("disallowSelectAll");
    var = getAttribute("var");
  }

  protected UIComponent createComponent(FaceletContext ctx) {
    DataTable t = new DataTable();
    if (autoUpdate != null)
      t.setAutoUpdate(autoUpdate.getInt(ctx));
    if (pageSize != null)
      t.setPageSize(pageSize.getInt(ctx));
    if (pageSizeRendered != null)
      t.setPageSizeRendered(pageSizeRendered.getBoolean(ctx));
    if (selectedRows != null)
      t.setSelectedRowsExpression(selectedRows.getValueExpression(ctx, List.class));
    if (updateUsingSubmit != null)
      t.setUpdateUsingSubmit(updateUsingSubmit.getBoolean(ctx));
    if(value != null) {
      t.setModelExpression(value.getValueExpression(ctx, DataTableModel.class));
    }
    if(disallowSelectAll != null) {
      t.setDisallowSelectAll(disallowSelectAll.getBoolean(ctx));
    }
    return t;
  }

  @SuppressWarnings({"unchecked"})
  private void loadSelectedRows(int startPos, DataTable t, DataTableSortOrder s, List rows) throws ModelException {
    Set<String> ids = new HashSet<String>(t.getSelectedRows());
    int i = 0;
    int c = 0;
    ModelWithObjectIds ident = (ModelWithObjectIds)t.getModel();
    List _rows;
    do{
      _rows = t.getModel().getRows(i*1000, 1000, s);
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
    } while(_rows.size() == 1000 && (t.isSelectAll() || rows.size() < ids.size()));

  }

  @SuppressWarnings({"unchecked"})
  private void getRows(List rows, DataTable t, int startPos, int number, DataTableSortOrder s) throws ModelException {
    if(!t.isShowSelectedOnly()) {
      rows.addAll(t.getModel().getRows(startPos, t.getPageSize(), s));
    } else {
      loadSelectedRows(startPos, t, s, rows);
    }
  }

  private void load(List rows, DataTable t, int startPos, int number,  DataTableSortOrder s) throws ModelException {
    getRows(rows, t, startPos, number, s);
    if (rows.isEmpty() && t.getCurrentPage() > 0) {
      t.setCurrentPage(0);
      getRows(rows,t, 0, number, s);
    }
  }

  protected void applyNextHandler(com.sun.facelets.FaceletContext ctx, javax.faces.component.UIComponent c) throws java.io.IOException, javax.faces.FacesException, javax.el.ELException {
    final DataTable t = (DataTable) c;
    String tid = t.getId();

    DataTableModel m = (DataTableModel) value.getValueExpression(ctx, DataTableModel.class).getValue(ctx);
    if (t.getSelectedRowsExpression() != null && !(m instanceof ModelWithObjectIds))
      throw new FacesException("Model should implement ModelWithObjectIds interface.");

    t.setModel(m);

    ctx.getVariableMapper().setVariable("___tid", new ConstantExpression(tid));

    // Header
    nextHandler.apply(ctx, c);

    handleColumns(t);

    if(!t.isInternalUpdate()) {
      t.setSelectAll(false);
      t.setSelectedRows(Collections.<String>emptyList());
      t.setShowSelectedOnly(false);
      return;
    }

    List rows;
    try{
      rows = loadRows(t);
      t.setTotalSize(t.getModel().getRowsCount());
    } catch (ModelException e) {
      //todo
      t.setError(e);
      rows = Collections.emptyList();
    }

    handleRows(ctx, t, rows);
  }

  private List loadRows(final DataTable t) throws ModelException {
    final DataTableSortOrder s = t.getSortOrder() == null ? null :
        (t.getSortOrder().charAt(0) == '-') ? new DataTableSortOrder(t.getSortOrder().substring(1), true) :
            new DataTableSortOrder(t.getSortOrder().substring(0), false);

    int startPos = t.getCurrentPage() * t.getPageSize();

    List rows = new LinkedList();

    if(t.getModel() instanceof PreloadableModel) {
      LoadListener listener = ((PreloadableModel)t.getModel()).prepareRows(startPos, t.getPageSize(), s);
      if(listener != null) {
        t.setLoadCurrent(listener.getCurrent());
        t.setLoadTotal(listener.getTotal());
        t.setError(listener.getLoadError());
        return rows;
      }
    }
    load(rows, t, startPos, t.getPageSize(), s);
    return rows;
  }


  private void handleColumns(DataTable t) throws IOException {
    if (t.getSortOrder() == null) {
      for (UIComponent col : t.getFirstRow().getChildren()) {
        if (col instanceof Column) {
          Column column = (Column) col;
          if (column.getDefaultSortOrder() != null) {
            if (column.getDefaultSortOrder().equalsIgnoreCase("asc"))
              t.setSortOrder("-" + column.getName());
            else if (column.getDefaultSortOrder().equalsIgnoreCase("desc"))
              t.setSortOrder(column.getName());
          }
        }
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