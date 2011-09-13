package mobi.eyeline.util.jsf.components.data_table;


import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import mobi.eyeline.util.jsf.components.data_table.model.ModelException;

import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.component.UIPanel;
import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class DataTable extends UIPanel {

  private Integer autoUpdate;
  private int pageSize = 20;
  private boolean pageSizeRendered = true;
  private Boolean updateUsingSubmit;
  private boolean disallowSelectAll;


  private final Renderer renderer = new DataTableRenderer();


  private String sortOrder;
  private int currentPage;
  private List<String> selectedRows = new LinkedList<String>();
  private boolean selectAll;
  private boolean internalUpdate;
  private boolean showSelectedOnly;
  private ModelException error;
  private int totalSize;
  private Integer loadCurrent;
  private Integer loadTotal;
  private int rowsOnPage;


  // Tag properties

  public Integer getAutoUpdate() {
    ValueExpression exp = getValueExpression("autoUpdate");
    if (exp == null)
      return autoUpdate;
    else
      return (Integer) exp.getValue(getFacesContext().getELContext());
  }

  public void setAutoUpdate(Integer autoUpdate) {
    this.autoUpdate = autoUpdate;
  }

  public int getPageSize() {
    ValueExpression exp = getValueExpression("pageSize");
    if (exp == null)
      return pageSize;
    else
      return (Integer) exp.getValue(getFacesContext().getELContext());
  }

  public void setPageSize(int pageSize) {
    this.pageSize = pageSize;
  }

  public boolean isPageSizeRendered() {
    ValueExpression exp = getValueExpression("pageSizeRendered");
    if (exp == null)
      return pageSizeRendered;
    else
      return (Boolean) exp.getValue(getFacesContext().getELContext());
  }

  public void setPageSizeRendered(boolean pageSizeRendered) {
    this.pageSizeRendered = pageSizeRendered;
  }

  public ValueExpression getSelectedRowsExpression() {
    return getValueExpression("selectedRows");
  }

  public Boolean isUpdateUsingSubmit() {
    ValueExpression exp = getValueExpression("updateUsingSubmit");
    if (exp == null)
      return updateUsingSubmit;
    else
      return (Boolean) exp.getValue(getFacesContext().getELContext());
  }

  public void setUpdateUsingSubmit(boolean updateUsingSubmit) {
    this.updateUsingSubmit = updateUsingSubmit;
  }

  public DataTableModel getModel() {
    ValueExpression exp = getValueExpression("value");
    if (exp == null)
      return null;
    else
      return (DataTableModel) exp.getValue(getFacesContext().getELContext());
  }

  public boolean isDisallowSelectAll() {
    ValueExpression exp = getValueExpression("disallowSelectAll");
    if (exp == null)
      return disallowSelectAll;
    else
      return (Boolean) exp.getValue(getFacesContext().getELContext());
  }

  public void setDisallowSelectAll(boolean disallowSelectAll) {
    this.disallowSelectAll = disallowSelectAll;
  }

  // Work properties

  Integer getLoadTotal() {
    return loadTotal;
  }

  void setLoadTotal(int loadTotal) {
    this.loadTotal = loadTotal;
  }

  Integer getLoadCurrent() {
    return loadCurrent;
  }

  void setLoadCurrent(int loadCurrent) {
    this.loadCurrent = loadCurrent;
  }

  int getTotalSize() {
    return totalSize;
  }

  void setTotalSize(int totalSize) {
    this.totalSize = totalSize;
  }

  ModelException getError() {
    return error;
  }

  void setError(ModelException error) {
    this.error = error;
  }

  boolean isShowSelectedOnly() {
    return showSelectedOnly;
  }

  void setShowSelectedOnly(boolean showSelectedOnly) {
    this.showSelectedOnly = showSelectedOnly;
  }

  boolean isInternalUpdate() {
    return internalUpdate;
  }

  void setInternalUpdate(boolean internalUpdate) {
    this.internalUpdate = internalUpdate;
  }

  boolean isSelectAll() {
    return selectAll;
  }

  void setSelectAll(boolean selectAll) {
    this.selectAll = selectAll;
  }

  String getSortOrder() {
    return sortOrder;
  }

  void setSortOrder(String sortOrder) {
    this.sortOrder = sortOrder;
  }

  int getCurrentPage() {
    return currentPage;
  }

  void setCurrentPage(int currentPage) {
    this.currentPage = currentPage;
  }

  void updatePageSize(int from, int to) {
    int startIndex = this.currentPage * from;
    this.pageSize = to;
    currentPage = startIndex / pageSize;
  }

  Row getFirstRow() {
    for (UIComponent c : getChildren()) {
      if (c instanceof Row)
        return (Row) c;
    }
    return null;
  }

  boolean hasInnerData() {
    for (UIComponent c : getChildren()) {
      if (c instanceof Row) {
        Row r = (Row) c;
        if (r.hasInnerAttribute())
          return true;
      }
    }
    return false;
  }

  boolean isRowsSelectionEnabled() {
    return getSelectedRowsExpression() != null;
  }

  void setSelectedRows(String[] select) {
    selectedRows.clear();
    if(select != null)
      Collections.addAll(this.selectedRows, select);
  }

  List<String> getSelectedRows() {
    return selectedRows;
  }

  boolean isRowSelected(String s) {
    return selectedRows.contains(s);
  }

  public int getRowsOnPage() {
    return rowsOnPage;
  }

  public void setRowsOnPage(int rowsOnPage) {
    this.rowsOnPage = rowsOnPage;
  }

  public void processUpdates(javax.faces.context.FacesContext context) {
    if (getSelectedRowsExpression() != null && !internalUpdate) {
      List<String> selected;
      if(selectAll && !disallowSelectAll) {
        DataTableModel model = getModel();
        selected = new LazySelectedList(model, selectedRows == null ? null : new HashSet<String>(selectedRows));
      }else {
        selected = new ArrayList<String>(selectedRows);
      }
      getSelectedRowsExpression().setValue(context.getELContext(), selected);
    }
    super.processUpdates(context);
  }


  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return renderer;
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[12];
    values[0] = super.saveState(context);
    values[1] = updateUsingSubmit;
    values[2] = autoUpdate;
    values[3] = currentPage;
    values[4] = pageSize;
    values[5] = sortOrder;
    values[6] = pageSizeRendered;
    values[7] = selectedRows;
    values[10] = selectAll;
    values[11] = disallowSelectAll;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    updateUsingSubmit = (Boolean) values[1];
    autoUpdate = (Integer) values[2];
    currentPage = (Integer) values[3];
    pageSize = (Integer) values[4];
    sortOrder = (String) values[5];
    pageSizeRendered = (Boolean) values[6];
    selectedRows = (List<String>) values[7];
    selectAll = (Boolean)values[10];
    disallowSelectAll = (Boolean)values[11];
  }

}