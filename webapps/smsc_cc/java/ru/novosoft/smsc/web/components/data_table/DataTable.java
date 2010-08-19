package ru.novosoft.smsc.web.components.data_table;

import javax.faces.component.UIComponentBase;
import javax.faces.context.FacesContext;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class DataTable extends UIComponentBase {

  private DataTableModel model;
  private String sortOrder;
  private int currentPage;
  private int pageSize = 20;
  private Integer autoUpdate;
  private boolean rowSelection;
  private boolean updateUsingSubmit;
  private List<Integer> selectedRows = new ArrayList<Integer>();

  private int currentRowNum;
  private DataTableRow currentRow;

  public String getFamily() {
    return "Eyeline";
  }

  public String getRendererType() {
    return "dataTable";
  }

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

  public void setRowSelection(boolean rowSelection) {
    this.rowSelection = rowSelection;
  }

  public boolean isRowSelection() {
    return rowSelection;
  }

  public void clearSelectedRows() {
    selectedRows.clear();
  }

  public void addSelectedRow(int rowNum) {
    selectedRows.add(rowNum);
  }

  public List<Integer> getSelectedRows() {
    return selectedRows;
  }

  public int getCurrentRowNum() {
    return currentRowNum;
  }

  public void setCurrentRowNum(int currentRowNum) {
    this.currentRowNum = currentRowNum;
  }

  public DataTableRow getCurrentRow() {
    return currentRow;
  }

  public void setCurrentRow(DataTableRow currentRow) {
    this.currentRow = currentRow;
  }

  public boolean isUpdateUsingSubmit() {
    return updateUsingSubmit;
  }

  public void setUpdateUsingSubmit(boolean updateUsingSubmit) {
    this.updateUsingSubmit = updateUsingSubmit;
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[11];
    values[0] = super.saveState(context);
    values[1] = updateUsingSubmit;
    values[3] = autoUpdate;
    values[4] = currentPage;
    values[5] = pageSize;
    values[6] = selectedRows;
    values[9] = sortOrder;
    values[10] = rowSelection;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    updateUsingSubmit = (Boolean)values[1];
    autoUpdate = (Integer) values[3];
    currentPage = (Integer) values[4];
    pageSize = (Integer) values[5];
    selectedRows = (List<Integer>) values[6];
    sortOrder = (String) values[9];
    rowSelection = (Boolean) values[10];
  }

}
