package ru.novosoft.smsc.web.util;

import javax.faces.context.FacesContext;
import javax.faces.model.DataModel;
import java.io.Serializable;
import java.util.List;

/**
 * Родительский класс для контроллеров, отображающих таблицу с сортировками и разбивкой на страницы
 * В качестве параметра используется класс, определяющий строки таблицы (implements Serializable ! )
 */
public abstract class AbstractTableController<T> implements Serializable{

  /**
   * Отображать или нет таблицу (для paginator необходимо вручную проверять этот параметр)
   **/
  protected boolean rendered = false;

  protected SortablePagedDataModel<T> model;

  private int pageSize = 20;

  /**
   * Название колонки для сортировки (см. метод sort)
   */
  private String _sortColumn;

  private static final String SELECT_COLUMN = "select_column";

  /**
   * Сортировка по возрастанию или убыванию (см. метод sort)
   */
  private boolean _asc = true;

  /**
   * Возвращает "модель" таблицы
   **/
  public DataModel getModel() {
    if (this.model == null) {
      this.model = new Model(pageSize);
    }
    return model;
  }

  /**
   * Кол-во строк на странице
   **/
  public int getPageSize() {
    return pageSize;
  }

  public void setPageSize(int pageSize) {
    this.pageSize = pageSize;
  }

  /**
   * Константа для названия переменной request, определяющей сортируемую колонку (см. метод sort)
   */
  public String getSelectColumn() {
    return SELECT_COLUMN;
  }

  public boolean is_asc() {
    return _asc;
  }

  public void set_asc(boolean _asc) {
    this._asc = _asc;
  }

  public String get_sortColumn() {
    return _sortColumn;
  }

  public void set_sortColumn(String _sortColumn) {
    this._sortColumn = _sortColumn;
  }

  class Model extends SortablePagedDataModel<T> {

    public Model(int pageSize) {
      super(pageSize);
    }

    void setPageSize(int pageSize) {
      this.pageSize = pageSize;
    }

    @Override
    public DataPage<T> fetchPage(int startRow, int pageSize) {
      try {
        return new DataPage<T>(startRow, getItems(_sortColumn, _asc, startRow, pageSize));
      } catch (Throwable e) {
        return null;
      }
    }

    @Override
    protected int getDatasetSize() {
      if (!rendered) {
        return 0;
      }
      return countItems();
    }
  }

  public String sort() {
    _sortColumn = FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap().get(SELECT_COLUMN);
    _asc = !_asc;
    return null;
  }

  public boolean isRendered() {
    return rendered;
  }

  public void setRendered(boolean rendered) {
    this.rendered = rendered;
  }

  public int getCountItems() {
    return countItems();
  }

  public String query() {
    rendered = true;
    return null;
  }

  /**
   * Метод для заполнения страницы таблицы
   **/
  protected abstract List<T> getItems(String sortField, boolean isAsc, int startRow, int size);

  /**
   * Метод возвращает общее кол-во строчек в таблице (для paginator-а)
   **/
  protected abstract int countItems();
}
