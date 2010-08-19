package ru.novosoft.smsc.web.components.data_table;

/**
 * @author Artem Snopkov
 */
public interface DataTableRow {

  public Object getData(String columnId);

  public Object getInnerData();

}
