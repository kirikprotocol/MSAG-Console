package ru.novosoft.smsc.jsp.util.tables.impl.regions;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;
import ru.novosoft.smsc.admin.region.Region;

import java.util.Set;

/**
 * User: artem
 * Date: Aug 1, 2007
 */

public class RegionsDataItem extends AbstractDataItem {

  RegionsDataItem(int id, String name, int sendSpeed, Set subjects) {
    values.put("id", new Integer(id));
    values.put("name", name);
    values.put("bandWidth", new Integer(sendSpeed));
    values.put("subjects", subjects);
  }

  RegionsDataItem(Region region) {
    values.put("id", new Integer(region.getId()));
    values.put("name", region.getName());
    values.put("bandWidth", new Integer(region.getBandWidth()));
    values.put("subjects", region.getSubjects());
  }

  public String getName() {
    return (String)getValue("name");
  }

  public int getBandWidth() {
    return ((Integer)getValue("bandWidth")).intValue();
  }

  public Set getSubjects() {
    return (Set)getValue("subjects");
  }

  public int getId() {
    return ((Integer)values.get("id")).intValue();
  }

}
