package mobi.eyeline.util.jsf.components.dynamic_table;

import mobi.eyeline.util.jsf.components.HtmlWriter;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableModel;
import mobi.eyeline.util.jsf.components.dynamic_table.model.DynamicTableRow;

import javax.faces.component.UIComponent;
import javax.faces.context.FacesContext;
import javax.faces.convert.ConverterException;
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
    DynamicTable c = (DynamicTable)component;

    DynamicTableModel newModel = new DynamicTableModel();
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

    TreeMap<Integer, DynamicTableRow> rows = new TreeMap<Integer, DynamicTableRow>();

    for (String p : dtParams) {
      int i = p.indexOf('_');
      Integer rowNum = Integer.parseInt(p.substring(0, i));
      String columnName = p.substring(i + 1);

      DynamicTableRow row = rows.get(rowNum);
      if (row == null) {
        row = new DynamicTableRow();
        rows.put(rowNum, row);
      }
      row.setValue(columnName, parametersMap.get(paramPrefix + p));
    }

    for (DynamicTableRow row : rows.values())
      newModel.addRow(row);

    c.setSubmittedValue(newModel);
  }

  @Override
  public Object getConvertedValue(FacesContext context, UIComponent component, Object submitted) throws ConverterException {
    return submitted;
  }


  public void encodeEnd(FacesContext context, UIComponent component) throws IOException {
    HtmlWriter w = new HtmlWriter(context.getResponseWriter());
    DynamicTable table = (DynamicTable) component;

    List<Column> columns = table.getColumns();

    String classStr = "eyeline_list";
    if (table.getTableClass() != null)
      classStr += " " + table.getTableClass();

    w.a("\n<table class=\"" + classStr + "\" id=\"" + component.getId() + "\" name=\"" + component.getId() + "\">");
//    w.a("<colgroup>");
//    for (Column c : columns)
//      w.a("<col width=\"").a(c.getWidth() != null ? c.getWidth() : ((100/columns.size()) + "%")).a("\"/>");
//    w.a("<col width=\"1px\"/>");
//    w.a("</colgroup>");

    w.a("<thead>");
    w.a("<tr>");
    for (Column c : table.getColumns()) {
      w.a("<th");
      if (c.getColumnClass() != null)
        w.a(" class=\"" + c.getColumnClass() + "\"");
      if(c.getTitle() == null || c.getTitle().length() == 0) {
        w.a(" style=\"display:none\"");
      }
      w.a(" >"+(c.getTitle()==null ? "":c.getTitle())+"</td>");
    }
    w.a("<th style=\"display:none\">&nbsp;</th>\n");
    w.a("</tr>");
    w.a("</thead>");
    w.a("<tbody></tbody>");
    w.a("</table>");

    w.a("<script language=\"javascript\" type=\"text/javascript\">\n");

    int i = 0;
    for (Column c : table.getColumns()) {
      String columnClass = c.getColumnClass();
      if (columnClass == null)
        columnClass="";

      if (c instanceof TextColumn) {
        w.a("var column" + i + "= new TextColumn('" + c.getName() + "','" + columnClass + "'," + ((TextColumn) c).isAllowEditAfterAdd() + "," + ((TextColumn) c).isAllowEmpty() + ");\n");

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

        w.a("var column" + i + "= new SelectColumn('" + c.getName() + "'," + values.toString() + ",'" + columnClass + "'," + ((SelectColumn) c).isAllowEditAfterAdd() + ',' + ((SelectColumn) c).isUniqueValues() + ");\n");
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

    w.a("var dtable=new DynamicTable('" + context.getExternalContext().getRequestContextPath() + "','" + component.getId() + "'," + columnsArray.toString() + ");\n");

    DynamicTableModel m = (DynamicTableModel)table.getSubmittedValue();
    if (m == null)
      m = (DynamicTableModel)table.getValue();


    for (DynamicTableRow row : m.getRows()) {
      StringBuilder valuesArray = new StringBuilder();
      valuesArray.append("new Array(");
      for (int j = 0; j < table.getColumns().size(); j++) {
        if (j > 0)
          valuesArray.append(',');
        String jsValue = row.getValue(table.getColumns().get(j).getName()).toString();
        jsValue = jsValue.replaceAll("'","\\\\'");
        jsValue = jsValue.replaceAll("\n","\\\\n'");
        valuesArray.append("'").append(jsValue).append("'");
      }
      valuesArray.append(")");
      w.a("dtable.addRow(").a(valuesArray.toString()).a(");\n");
    }

    w.a("</script>");
  }
}
