package mobi.eyeline.util.jsf.components.dynamic_table;

import mobi.eyeline.util.jsf.components.base.UIInputImpl;

import javax.faces.context.FacesContext;
import javax.faces.render.Renderer;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class DynamicTable extends UIInputImpl {

  private List<Column> columns = new ArrayList<Column>();

  void addColumn(Column column) {
    columns.add(column);
  }

  List<Column> getColumns() {
    return columns;
  }

  @Override
  public String getFamily() {
    return "Eyeline";
  }

  @Override
  protected Renderer getRenderer(FacesContext context) {
    return new DynamicTableRenderer();
  }

}
