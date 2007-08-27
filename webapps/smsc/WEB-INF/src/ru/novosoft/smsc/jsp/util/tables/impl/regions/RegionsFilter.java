package ru.novosoft.smsc.jsp.util.tables.impl.regions;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

/**
 * User: artem
 * Date: Aug 1, 2007
 */

public class RegionsFilter implements Filter{

  private final String regionName;

  public RegionsFilter(String regionName) {
    this.regionName = regionName;
  }

  public boolean isEmpty() {
    return regionName == null || regionName.length()==0;
  }

  public boolean isItemAllowed(DataItem item) {
    return regionName == null || ((RegionsDataItem)item).getName().startsWith(regionName);
  }
}
