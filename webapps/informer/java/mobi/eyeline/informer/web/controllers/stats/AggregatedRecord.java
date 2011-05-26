package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.web.components.data_table.model.DataTableRow;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;

import java.io.PrintWriter;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 29.10.2010
 * Time: 14:26:08
 */
public abstract class AggregatedRecord implements DataTableRow {
  protected TreeMap<Object, AggregatedRecord> innerRowsMap = null;
  protected List<AggregatedRecord> innerRows = null;
  protected boolean isParent;

  abstract Object getAggregationKey();

  abstract void add(AggregatedRecord other);

  abstract Comparator getRecordsComparator(DataTableSortOrder sortOrder);

  abstract void printCSVheader(PrintWriter writer, boolean detalised);

  abstract void printWithChildrenToCSV(PrintWriter writer, boolean detalised);


  protected void addChild(AggregatedRecord newChild) {
    Object key = newChild.getAggregationKey();
    AggregatedRecord oldChild = innerRowsMap.get(key);
    if (oldChild == null) {
      innerRowsMap.put(key, newChild);
    } else {
      oldChild.add(newChild);
    }
  }

  public Map<Object, AggregatedRecord> getInnerRowsMap() {
    return innerRowsMap;
  }

  protected void addChildren(AggregatedRecord other) {
    if (!isParent) return;
    for (AggregatedRecord newChild : other.getInnerRowsMap().values()) {
      addChild(newChild);
    }
  }

  public String getId() {
    return null;
  }

  public Object getData() {
    return this;
  }

  public Object getInnerData() {
    return null;
  }

  public List getInnerRows() {
    if (!isParent) return null;
    if (innerRows == null) {
      innerRows = new ArrayList(innerRowsMap.values());
    }
    return innerRows;
  }

  public boolean isParent() {
    return isParent;
  }
}
