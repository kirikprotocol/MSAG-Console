package ru.sibinco.lib.bean;

import java.util.List;


/**
 * Created by igork
 * Date: 05.03.2004
 * Time: 15:55:42
 */
public interface TabledBean
{
  List getTabledItems();

  String getSort();

  void setSort(String sort);

  int getPageSize();

  void setPageSize(int pageSize);

  int getStartPosition();

  void setStartPosition(int startPosition);

  int getTotalSize();

  String[] getChecked();

  void setChecked(String[] checked);

  boolean isChecked(String id);
}
