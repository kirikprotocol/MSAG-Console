package mobi.eyeline.informer.web.components.dynamic_table;

import mobi.eyeline.informer.web.components.EyelineComponent;
import mobi.eyeline.informer.web.components.dynamic_table.model.DynamicTableModel;

import javax.el.ValueExpression;
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
  private ValueExpression valueExpression;

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

  public void processUpdates(javax.faces.context.FacesContext context) {
    if (valueExpression != null)
      valueExpression.setValue(context.getELContext(), model);
    super.processUpdates(context);
  }

  public void setValueExpression(ValueExpression valueExpression) {
    this.valueExpression = valueExpression;
  }

  public Object saveState(FacesContext context) {
    Object[] values = new Object[5];
    values[0] = super.saveState(context);
    values[1] = model;
    values[2] = valueExpression;
    return (values);
  }

  public void restoreState(FacesContext context, Object state) {
    Object[] values = (Object[]) state;
    super.restoreState(context, values[0]);
    model = (DynamicTableModel) values[1];
    valueExpression = (ValueExpression)values[2];
  }
}
