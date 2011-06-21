package mobi.eyeline.informer.web.components.data_table;

import mobi.eyeline.informer.web.components.EyelineComponent;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;

import javax.el.ValueExpression;
import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import java.util.*;

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
  private Map<Integer, ValueExpression> rows = new HashMap<Integer, ValueExpression>();

  private ValueExpression selectedRowsExpression;

  private ValueExpression modelExpression;

  private boolean pageSizeRendered = true;

  private List<String> selectedRows = new LinkedList<String>();

  private boolean selectAll;

  private boolean internalUpdate;

  private boolean showSelectedOnly;

  private boolean disallowSelectAll;

  public boolean isShowSelectedOnly() {
    return showSelectedOnly;
  }

  public void setShowSelectedOnly(boolean showSelectedOnly) {
    this.showSelectedOnly = showSelectedOnly;
  }

  boolean isInternalUpdate() {
    return internalUpdate;
  }

  void setInternalUpdate(boolean internalUpdate) {
    this.internalUpdate = internalUpdate;
  }

  public boolean isSelectAll() {
    return selectAll;
  }

  public void setSelectAll(boolean selectAll) {
    this.selectAll = selectAll;
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

  public boolean isPageSizeRendered() {
    return pageSizeRendered;
  }

  public void setPageSizeRendered(boolean pageSizeRendered) {
    this.pageSizeRendered = pageSizeRendered;
  }

  public void clearSelectedRows() {
    selectedRows.clear();
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
        Row r = (Row) c;
        if (r.hasInnerData() || r.hasInnerRows())
          return true;
      }
    }
    return false;
  }

  public ValueExpression getRowValueExpr(int rowNumber) {
    ValueExpression e = rows.get(rowNumber);
    if (e == null) {
      e = new ConstantExpression(null);
      rows.put(rowNumber, e);
    }
    return e;
  }

  public ValueExpression getModelExpression() {
    return modelExpression;
  }

  public void setModelExpression(ValueExpression modelExpression) {
    this.modelExpression = modelExpression;
  }

  public void processDecodes(javax.faces.context.FacesContext context) {
    DataTableRenderer.decodeTable(context, this);
    super.processDecodes(context);
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
    values[8] = selectedRowsExpression;
    values[9] = modelExpression;
    values[10] = selectAll;
    values[11] = disallowSelectAll;
    return (values);
  }

  public void updateSelected(String[] select) {
    if(select != null) {
      Collections.addAll(this.selectedRows, select);
    }else {
      selectedRows.clear();
    }
  }

  public boolean isSelection() {
    return selectedRowsExpression != null;
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
    selectedRowsExpression = (ValueExpression) values[8];
    modelExpression = (ValueExpression)values[9];
    selectAll = (Boolean)values[10];
    disallowSelectAll = (Boolean)values[11];
  }

  public List<String> getSelectedRows() {
    return selectedRows;
  }

  public void setSelectedRows(List<String> selectedRows) {
    this.selectedRows = selectedRows == null ? null : new ArrayList<String>(selectedRows);
  }

  public void processUpdates(javax.faces.context.FacesContext context) {
    if (selectedRowsExpression != null && !internalUpdate) {
      List<String> selected;
      if(selectAll && !disallowSelectAll) {
        DataTableModel model = (DataTableModel)modelExpression.getValue(context.getELContext());
        selected = new LazySelectedList(model, selectedRows == null ? null : new HashSet<String>(selectedRows));
      }else {
        selected = new ArrayList<String>(selectedRows);
      }
      selectedRowsExpression.setValue(context.getELContext(), selected);
    }
    super.processUpdates(context);
  }


  public boolean isSelected(String s) {
    return selectedRows.contains(s);
  }

  public ValueExpression getSelectedRowsExpression() {
    return selectedRowsExpression;
  }

  public void setSelectedRowsExpression(ValueExpression selectedRowsExpression) {
    this.selectedRowsExpression = selectedRowsExpression;
  }

  public boolean isDisallowSelectAll() {
    return disallowSelectAll;
  }

  public void setDisallowSelectAll(boolean disallowSelectAll) {
    this.disallowSelectAll = disallowSelectAll;
  }

  private static class LazySelectedList implements List<String>{

    private final DataTableModel model;

    private List<String> selected;

    private final Set<String> unselected;

    private LazySelectedList(DataTableModel model, Set<String> unselected) {
      this.model = model;
      this.unselected = unselected;
    }

    private void load() {
      List<String> selected = new LinkedList<String>();
      int i = 0;
      List rows;
      while(!(rows =  model.getRows(10000*i, 10000, null)).isEmpty()) {
        for (Object o : rows) {
          String id = model.getId(o);
          if(unselected == null || !unselected.contains(id)) {
            selected.add(id);
          }
        }
        if(rows.size()<10000) {
          break;
        }
        i++;
      }
      this.selected = selected;
    }

    @Override
    public int size() {
      if(selected == null) {
        load();
      }
      return selected.size();
    }

    @Override
    public boolean isEmpty() {
      if(selected == null) {
        load();
      }
      return selected.isEmpty();
    }

    @Override
    public boolean contains(Object o) {
      if(selected == null) {
        load();
      }
      return selected.contains(o);
    }

    @Override
    public Iterator<String> iterator() {
      if(selected == null) {
        load();
      }
      return selected.iterator();
    }

    @Override
    public Object[] toArray() {
      if(selected == null) {
        load();
      }
      return selected.toArray();
    }

    @Override
    public <T> T[] toArray(T[] a) {
      if(selected == null) {
        load();
      }
      return selected.toArray(a);
    }

    @Override
    public boolean add(String s) {
      if(selected == null) {
        load();
      }
      return selected.add(s);
    }

    @Override
    public boolean remove(Object o) {
      if(selected == null) {
        load();
      }
      return selected.remove(o);
    }

    @Override
    public boolean containsAll(Collection<?> c) {
      if(selected == null) {
        load();
      }
      return selected.containsAll(c);
    }

    @Override
    public boolean addAll(Collection<? extends String> c) {
      if(selected == null) {
        load();
      }
      return selected.addAll(c);
    }

    @Override
    public boolean addAll(int index, Collection<? extends String> c) {
      if(selected == null) {
        load();
      }
      return selected.addAll(index, c);
    }

    @Override
    public boolean removeAll(Collection<?> c) {
      if(selected == null) {
        load();
      }
      return selected.removeAll(c);
    }

    @Override
    public boolean retainAll(Collection<?> c) {
      if(selected == null) {
        load();
      }
      return selected.retainAll(c);
    }

    @Override
    public void clear() {
      if(selected == null) {
        load();
      }
      selected.clear();
    }

    @Override
    public boolean equals(Object o) {
      if(selected == null) {
        load();
      }
      return selected.equals(o);
    }

    @Override
    public int hashCode() {
      if(selected == null) {
        load();
      }
      return selected.hashCode();
    }

    @Override
    public String get(int index) {
      if(selected == null) {
        load();
      }
      return selected.get(index);
    }

    @Override
    public String set(int index, String element) {
      if(selected == null) {
        load();
      }
      return selected.set(index, element);
    }

    @Override
    public void add(int index, String element) {
      if(selected == null) {
        load();
      }
      selected.add(index, element);
    }

    @Override
    public String remove(int index) {
      if(selected == null) {
        load();
      }
      return selected.remove(index);
    }

    @Override
    public int indexOf(Object o) {
      if(selected == null) {
        load();
      }
      return selected.indexOf(o);
    }

    @Override
    public int lastIndexOf(Object o) {
      if(selected == null) {
        load();
      }
      return selected.lastIndexOf(o);
    }

    @Override
    public ListIterator<String> listIterator() {
      if(selected == null) {
        load();
      }
      return selected.listIterator();
    }

    @Override
    public ListIterator<String> listIterator(int index) {
      if(selected == null) {
        load();
      }
      return selected.listIterator(index);
    }

    @Override
    public List<String> subList(int fromIndex, int toIndex) {
      if(selected == null) {
        load();
      }
      return selected.subList(fromIndex, toIndex);
    }

    @Override
    public String toString() {
      if(selected == null) {
        load();
      }
      return selected.toString();
    }
  }



}
