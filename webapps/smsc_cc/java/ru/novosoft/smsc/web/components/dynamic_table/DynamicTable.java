package ru.novosoft.smsc.web.components.dynamic_table;

import ru.novosoft.smsc.web.components.EyelineComponent;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;

import javax.faces.context.FacesContext;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class DynamicTable extends EyelineComponent {

  private DynamicTableModel model;
  private List<Column> columns = new ArrayList<Column>();
  private int width; 

  public DynamicTableModel getModel() {
    return model;
  }

  public void setModel(DynamicTableModel value) {
    this.model = value;
  }

  void addColumn(Column column) {
    columns.add(column);
  }

  List<Column> getColumns() {
    return columns;
  }

  int getWidth() {
    return width;
  }

  void setWidth(int width) {
    this.width = width;
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[5];
    values[0] = super.saveState(context);
    values[1] = model;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    model = (DynamicTableModel) values[1];
  }
}
