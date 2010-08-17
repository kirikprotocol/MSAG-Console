package ru.novosoft.smsc.web.components.paged_table;

import javax.faces.component.UICommand;
import javax.faces.context.FacesContext;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class PagedTable extends UICommand {

  private List<Column> columns = new ArrayList<Column>();
  private PagedTableModel model;
  private String sortOrder;
  private boolean hasFilter;
  private int currentPage;
  private int pageSize = 20;
  private Integer autoUpdate;
  private boolean allowCheckBoxes = true;
  private List<Integer> selectedRows = new ArrayList<Integer>();

  private String selectedColumn;
  private int selectedRow;

  public String getFamily() {
    return "Eyeline";
  }

  public String getRendererType() {
    return "pagedTable";
  }

  public void addColumn(Column c) {
    if (!columns.contains(c))
      columns.add(c);
  }

  public List<Column> getColumns() {
    return columns;
  }

  public PagedTableModel getModel() {
    return model;
  }

  public void setModel(PagedTableModel model) {
    this.model = model;
  }

  public String getSortOrder() {
    return sortOrder;
  }

  public void setSortOrder(String sortOrder) {
    this.sortOrder = sortOrder;
  }

  public boolean isHasFilter() {
    return hasFilter;
  }

  public void setHasFilter(boolean hasFilter) {
    this.hasFilter = hasFilter;
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

  public boolean isAllowCheckBoxes() {
    return allowCheckBoxes;
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

  public String getSelectedColumn() {
    return selectedColumn;
  }

  public void setSelectedColumn(String selectedColumn) {
    this.selectedColumn = selectedColumn;
  }

  public int getSelectedRow() {
    return selectedRow;
  }

  public void setSelectedRow(int selectedRow) {
    this.selectedRow = selectedRow;
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[10];
    values[0] = super.saveState(context);
    values[1] = model;
    values[2] = hasFilter;
    values[3] = autoUpdate;
    values[4] = currentPage;
    values[5] = pageSize;
    values[6] = selectedRows;
    values[7] = selectedColumn;
    values[8] = selectedRow;
    values[9] = columns;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    model = (PagedTableModel) values[1];
    hasFilter = (Boolean) values[2];
    autoUpdate = (Integer)values[3];
    currentPage = (Integer)values[4];
    pageSize = (Integer)values[5];
    selectedRows = (List<Integer>)values[6];
    selectedColumn = (String)values[7];
    selectedRow = (Integer)values[8];
    columns = (List<Column>)values[9];
  }
}
