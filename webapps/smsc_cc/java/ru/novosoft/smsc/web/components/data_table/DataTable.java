package ru.novosoft.smsc.web.components.data_table;

import ru.novosoft.smsc.web.components.EyelineComponent;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableRow;

import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class DataTable extends EyelineComponent {

  private DataTableModel model;
  private String sortOrder;
  private int currentPage;
  private int pageSize = 20;
  private Integer autoUpdate;
  private Boolean updateUsingSubmit;

  private List<String> selectedRows = new ArrayList<String>();
  private ValueExpression selectedRowsExpression;

  public DataTableModel getModel() {
    return model;
  }

  public void setModel(DataTableModel model) {
    this.model = model;
  }

  public String getSortOrder() {
    return sortOrder;
  }

  public void setSortOrder(String sortOrder) {
    this.sortOrder = sortOrder;
  }

  public Integer getAutoUpdate() {
    return autoUpdate;
  }

  public void setAutoUpdate(Integer autoUpdate) {
    this.autoUpdate = autoUpdate;
  }

  public int getCurrentPage() {
    return currentPage;
  }

  public void setCurrentPage(int currentPage) {
    this.currentPage = currentPage;
  }

  public int getPageSize() {
    return pageSize;
  }

  public void setPageSize(int pageSize) {
    this.pageSize = pageSize;
  }

  public void updatePageSize(int from, int to) {
    int startIndex = this.currentPage * from;
    this.pageSize = to;
    currentPage = startIndex / pageSize;
  }

  public boolean isRowSelection() {
    return selectedRowsExpression != null;
  }

  public void clearSelectedRows() {
    selectedRows.clear();
  }

  public void addSelectedRow(String rowId) {
    selectedRows.add(rowId);
  }

  public List<String> getSelectedRows() {
    return selectedRows;
  }

  public Boolean isUpdateUsingSubmit() {
    return updateUsingSubmit;
  }

  public void setUpdateUsingSubmit(boolean updateUsingSubmit) {
    this.updateUsingSubmit = updateUsingSubmit;
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
        DataTableRow tableRow = ((Row) c).getRow();
        if (tableRow != null && (tableRow.getInnerData() != null || tableRow.getInnerRows() != null))
          return true;
      }
    }
    return false;
  }

  public void setSelectedRowsExpression(ValueExpression selectedRowsExpression) {
    this.selectedRowsExpression = selectedRowsExpression;
  }

  public void processUpdates(javax.faces.context.FacesContext context) {
    if (selectedRowsExpression != null && selectedRows != null && !selectedRows.isEmpty())
      selectedRowsExpression.setValue(context.getELContext(), new ArrayList<String>(selectedRows));
    super.processUpdates(context);
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[11];
    values[0] = super.saveState(context);
    values[1] = updateUsingSubmit;
    values[2] = selectedRowsExpression;
    values[3] = autoUpdate;
    values[4] = currentPage;
    values[5] = pageSize;
    values[9] = sortOrder;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    updateUsingSubmit = (Boolean) values[1];
    selectedRowsExpression = (ValueExpression)values[2];
    autoUpdate = (Integer) values[3];
    currentPage = (Integer) values[4];
    pageSize = (Integer) values[5];
    sortOrder = (String) values[9];
  }

}
