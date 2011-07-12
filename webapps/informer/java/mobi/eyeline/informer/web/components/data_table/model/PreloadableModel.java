package mobi.eyeline.informer.web.components.data_table.model;

/**
 * Модель, требующая предварительной загрузки данных перед вызовом метода getRows().
 * author: Aleksandr Khalitov
 */
public interface PreloadableModel extends DataTableModel {

  /**
   * Предварительная загрузка записей
   * @param startPos с какой записи требуется загрузить
   * @param count сколько записейц требуется загрузить
   * @param sortOrder сортировка записей (возможно null)
   * @return возвращает объект LoadListener, если процесс загрузки начался либо в процессе, null если данные уже загружены и могут быть показаны, используя метод {@link PreloadableModel#getRows(int, int, DataTableSortOrder)}
   * @throws ModelException ошибка загрузки
   */
  public LoadListener prepareRows(int startPos, int count, DataTableSortOrder sortOrder) throws ModelException;


}
