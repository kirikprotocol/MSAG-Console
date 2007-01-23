package ru.novosoft.smsc.jsp.util.helper.dynamictable;

import ru.novosoft.smsc.jsp.util.helper.Validation;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.RowControlButtonColumn;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.TextColumn;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * User: artem
 * Date: 16.01.2007
 */

public class ListPropertiesHelper extends DynamicTableHelper{

  private final Column props;
  private final Column delColumn;

  public ListPropertiesHelper(String name, String uid, int width, Validation validation, boolean allowEditPropsAfterAdd) {
    super(name, uid);
    props = new TextColumn(this, name, name, width, validation, allowEditPropsAfterAdd);
    delColumn = new RowControlButtonColumn(this, "", "delColumn");
    addColumn(props);
    addColumn(delColumn);
  }

  public ListPropertiesHelper(String name, String uid, int width, Validation validation, boolean allowEditPropsAfterAdd, String[] properties) {
    this(name, uid, width, validation, allowEditPropsAfterAdd);

    setProps(properties);
  }

  protected void fillTable() {
  }

  public void setProps(String[] properties) {
    for (int i=0; i<properties.length - 1; i++) {
      final Row row = createNewRow();
      row.addValue(props, properties[i]);
    }

    if (properties.length > 0)
      getLastRow().addValue(props, properties[properties.length - 1]);
  }

  public List getPropsAsList() {
    final ArrayList result = new ArrayList();
    for (Iterator iter = getRows(); iter.hasNext();)
      result.add(((Row)iter.next()).getValue(props));
    return result;
  }

  public String[] getPropsAsArray() {
    final String[] result = new String[getRowsCount()+1];
    int i=0;
    for (Iterator iter = getRows(); iter.hasNext(); i++)
      result[i] =(String)((Row)iter.next()).getValue(props);
    result[getRowsCount()] = (String)getLastRow().getValue(props);
    return result;
  }
}
