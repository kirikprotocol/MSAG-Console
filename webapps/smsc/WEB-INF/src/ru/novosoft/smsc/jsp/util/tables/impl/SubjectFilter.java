/*
 * Author: igork
 * Date: 13.06.2002
 * Time: 15:44:31
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import java.util.*;


public class SubjectFilter implements Filter
{
  Set names = null;
  List smes = null;
  MaskList masks = null;
  boolean empty = true;

  public SubjectFilter()
  {
  }

  public SubjectFilter(Set names, List smeIds, String masks)
  {
    this.names = names;
    this.smes = smeIds;
    this.masks = new MaskList(masks);
    if (empty = (this.names.isEmpty() && this.smes.isEmpty() && this.masks.isEmpty()))
    {
      this.names = null;
      this.smes = null;
      this.masks = null;
    }
  }

  public boolean isEmpty()
  {
    return empty;
  }

  private boolean isMaskAllowed(String mask)
  {
    for (Iterator i = masks.getNames().iterator(); i.hasNext();)
    {
      if (mask.startsWith((String) i.next()))
        return true;
    }
    return false;
  }

  private boolean isAnyMaskAllowed(Vector item_masks)
  {
    for (Iterator i = item_masks.iterator(); i.hasNext();)
    {
      if (isMaskAllowed((String) i.next()))
        return true;
    }
    return false;
  }

  public boolean isItemAllowed(DataItem item)
  {
    if (empty)
      return true;

    String item_name = (String) item.getValue("Name");
    String item_sme = (String) item.getValue("Default SME");
    Vector item_masks = (Vector) item.getValue("Masks");
    return (names.isEmpty() || names.contains(item_name))
            && (smes.isEmpty() || smes.contains(item_sme))
            && (masks.isEmpty() || isAnyMaskAllowed(item_masks));
  }

  public Set getMaskStrings()
  {
    if (empty)
      return new HashSet();
    else
      return masks.getNames();
  }

  public Set getNames()
  {
    if (empty)
      return new HashSet();
    else
      return names;
  }

  public List getSmeIds()
  {
    if (empty)
      return new Vector();
    else
      return smes;
  }
}
