/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 20, 2003
 * Time: 2:23:08 PM
 */
package ru.novosoft.smsc.admin.dl;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.AdminException;

public class DistributionList
{
  private String name;
  private Mask owner;
  private int maxElements;

  public DistributionList()
  {
    name = "";
    owner = null;
    maxElements = 0;
  }
/*
    public DistributionList(String name, int maxElements) {
        this.name = name; this.owner = null;
        this.maxElements = maxElements;
    }
*/
  public DistributionList(String name, String owner, int maxElements) throws AdminException
  {
    this.name = name;
    this.maxElements = maxElements;
    setOwner(owner);
  }

  public String getName()
  {
    return name;
  }

  public void setName(String name)
  {
    this.name = name;
  }

  public String getOwner()
  {
    return owner != null ? owner.getMask() : null;
  }

  public String getNormalizedOwner()
  {
    return owner != null ? owner.getNormalizedMask() : null;
  }

  public void setOwner(String owner) throws AdminException
  {
    this.owner = owner != null && owner.trim().length() > 0 ? new Mask(owner) : null;
  }

  public boolean isSys()
  {
    return owner == null;
  }

  public int getMaxElements()
  {
    return maxElements;
  }

  public void setMaxElements(int maxElements)
  {
    this.maxElements = maxElements;
  }
}
