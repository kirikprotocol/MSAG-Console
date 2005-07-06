/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 20, 2003
 * Time: 2:23:08 PM
 */
package ru.novosoft.smsc.admin.dl;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

public class DistributionList extends AbstractDataItem
{
  public DistributionList()
  {
    values.put("name", "");
    values.put("owner", null);
    values.put("maxElements", new Integer(0));
  }

  public DistributionList(String name, String owner, int maxElements) throws AdminException
  {
    values.put("name", name);
    values.put("maxElements", new Integer(maxElements));
    setOwner(owner);
  }

  public String getName()
  {
    return (String) values.get("name");
  }

  public void setName(String name)
  {
    values.put("name", name == null ? "" : name);
  }

  public String getOwner()
  {
    return values.get("owner") != null ? ((Mask)values.get("owner")).getMask() : "";
  }

  public String getNormalizedOwner()
  {
    return values.get("owner") != null ? ((Mask)values.get("owner")).getNormalizedMask() : null;
  }

  public void setOwner(String owner) throws AdminException
  {
    values.put("owner", owner != null && owner.trim().length() > 0 ? new Mask(owner) : null);
  }

  public boolean isSys()
  {
    return values.get("owner") == null;
  }

  public int getMaxElements()
  {
    return ((Integer)values.get("maxElements")).intValue();
  }

  public void setMaxElements(int maxElements)
  {
    values.put("maxElements", new Integer(maxElements));
  }
}
