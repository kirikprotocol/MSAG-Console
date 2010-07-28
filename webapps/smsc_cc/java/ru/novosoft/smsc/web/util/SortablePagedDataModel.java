package ru.novosoft.smsc.web.util;

/**
 * This is the base class for datamodels returned by the beans, which want their
 * contents be sortable and pageable in the most efficient way. Subclasses can get
 * the sortColumn and ascending properties during resultset retrieval.
 *
 * @author szolotaryov
 *         <p/>
 *         Oct 10, 2007
 */
public abstract class SortablePagedDataModel<T> extends PagedListDataModel<T> {

  private static final long serialVersionUID = -777764054824546815L;
  private String sortColumn;
  private boolean sortAscending;

  public SortablePagedDataModel(int pageSize) {
    super(pageSize);
  }

  public final String getSortColumn() {
    return sortColumn;
  }

  public final void setSortColumn(String sortColumn) {
    this.sortColumn = sortColumn;
  }

  public final boolean isSortAscending() {
    return sortAscending;
  }

  public final void setSortAscending(boolean sortAscending) {
    this.sortAscending = sortAscending;
  }

}
