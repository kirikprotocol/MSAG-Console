package mobi.eyeline.informer.web.components.data_table.model;

import java.util.List;

/**
 * Интерфейс, предоставляющий данные для заполнения DataTable 
 * @author Artem Snopkov
 */
public interface DataTableModel {

  /**
   * Возвращает список объектов согласно указанным критериям
   * @param startPos начальная позиция
   * @param count количество элементов
   * @param sortOrder порядок сортировки
   * @return список экземпляров DataTableRow согласно указанным критериям
   */
  List getRows(int startPos, int count, DataTableSortOrder sortOrder);

  /**
   * Возвращает общее число элементов в таблице
   * @return общее число элементов в таблице
   */
  int getRowsCount() ;
}
