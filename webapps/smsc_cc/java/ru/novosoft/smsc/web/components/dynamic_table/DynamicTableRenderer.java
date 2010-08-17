package ru.novosoft.smsc.web.components.dynamic_table;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.context.ResponseWriter;
import javax.faces.render.Renderer;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

/**
 * @author Artem Snopkov
 */
public class DynamicTableRenderer extends Renderer {

  public void decode(FacesContext context, UIComponent component) {

    System.out.println("DYNAMIC TABLE DECODE");

    DynamicTable c = (DynamicTable) component;

    TableModel newModel = new TableModel();
    String paramPrefix = c.getId() + '_';
    String newCellPrefix = c.getId() + "_newcell";
    String totalCellPrefix = c.getId() + "_total";

    Map<String, String> parametersMap = context.getExternalContext().getRequestParameterMap();

    ArrayList<String> dtParams = new ArrayList<String>();
    for (String paramName : parametersMap.keySet()) {
      if (paramName.startsWith(paramPrefix) && !paramName.startsWith(newCellPrefix) && !paramName.startsWith(totalCellPrefix)) {
        dtParams.add(paramName.substring(paramPrefix.length()));
      }
    }

    TreeMap<Integer, TableRow> rows = new TreeMap<Integer, TableRow>();

    for (String p : dtParams) {
      int i = p.indexOf('_');
      Integer rowNum = Integer.parseInt(p.substring(0, i));
      String columnName = p.substring(i + 1);

      TableRow row = rows.get(rowNum);
      if (row == null) {
        row = new TableRow();
        rows.put(rowNum, row);
      }
      row.setValue(columnName, parametersMap.get(paramPrefix + p));
    }

    for (TableRow row : rows.values())
      newModel.addRow(row);

    c.setModel(newModel);
  }

  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    ResponseWriter w = context.getResponseWriter();
    DynamicTable table = (DynamicTable) component;
    w.append("\n<table class=\"properties_list\" cellspacing=0 id=\"" + component.getId() + "\" name=\"" + component.getId() + "\" width=\"30%\"></table>\n");

    w.append("<script language=\"javascript\" type=\"text/javascript\">\n");
    
    int i = 0;
    for (Column c : table.getColumns()) {
      if (c instanceof TextColumn) {
        w.append("var column" + i + "= new TextColumn('" + c.getName() + "'," + ((TextColumn) c).isAllowEditAfterAdd() + ");\n");

      } else if (c instanceof SelectColumn) {
        StringBuilder values = new StringBuilder();
        values.append("new Array(");
        List valuesList = ((SelectColumn) c).getValues();
        for (int j = 0; j < valuesList.size(); j++) {
          if (j > 0)
            values.append(',');
          values.append("'").append(valuesList.get(j)).append("'");
        }
        values.append(')');

        w.append("var column" + i + "= new SelectColumn('" + c.getName() + "'," + values.toString() + ',' + ((SelectColumn) c).isAllowEditAfterAdd() + ',' + ((SelectColumn) c).isUniqueValues() + ");\n");
      }
      i++;
    }

    StringBuilder columnsArray = new StringBuilder();
    columnsArray.append("new Array(");
    for (int j = 0; j < i; j++) {
      if (j > 0)
        columnsArray.append(',');
      columnsArray.append("column").append(j);
    }
    columnsArray.append(')');

    w.append("var dtable=new DynamicTable('" + context.getExternalContext().getRequestContextPath() + "','" + component.getId() + "'," + columnsArray.toString() + ");\n");

    TableModel m = table.getModel();
    for (TableRow row : m.getRows()) {
      StringBuilder valuesArray = new StringBuilder();
      valuesArray.append("new Array(");
      for (int j = 0; j < table.getColumns().size(); j++) {
        if (j > 0)
          valuesArray.append(',');
        valuesArray.append("'").append(row.getValue(table.getColumns().get(j).getName())).append("'");
      }
      valuesArray.append(")");
      w.append("dtable.addRow(").append(valuesArray).append(");\n");
    }

    w.append("</script>");
  }
}
