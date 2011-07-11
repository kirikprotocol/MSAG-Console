package mobi.eyeline.util.jsf.components.data_table.model;

import java.util.List;

/**
 * Интерфейс, предоставляющий данные для заполнения DataTable
 *
 * @author Artem Snopkov
 */
public interface DataTableModel {

  /**
   * Возвращает список объектов согласно указанным критериям
   *
   * @param startPos  начальная позиция
   * @param count     количество элементов
   * @param sortOrder порядок сортировки
   * @return список экземпляров DataTableRow согласно указанным критериям
   * @throws ModelException ошибка при составлении списка
   */
  List getRows(int startPos, int count, DataTableSortOrder sortOrder) throws ModelException;

  /**
   * Возвращает общее число элементов в таблице
   *
   * @return общее число элементов в таблице
   * @throws ModelException ошибка при вычислении размера
   */
  int getRowsCount() throws ModelException;
}
