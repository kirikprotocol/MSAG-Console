/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 17:14:45
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;


public class AliasFilter implements Filter
{
  private boolean empty = true;
  private MaskList aliases = null;
  private MaskList addresses = null;


  public AliasFilter()
  {
    empty = true;
  }

  public AliasFilter(String alias_masks, String address_masks)
  {
    aliases = new MaskList(alias_masks);
    addresses = new MaskList(address_masks);
    if (empty = (aliases.isEmpty() && addresses.isEmpty()))
    {
      aliases = null;
      addresses = null;
    }
  }

  public boolean isEmpty()
  {
    return empty;
  }

  protected static boolean isMaskAllowed(MaskList masks, String addr)
  {
    if (masks.isEmpty())
      return true;

    for (Iterator i = masks.iterator(); i.hasNext();) {
      if (addr.startsWith(((Mask) i.next()).getMask()))
        return true;
    }
    return false;
  }

  public boolean isItemAllowed(DataItem item)
  {
    if (empty)
      return true;

    return isMaskAllowed(addresses, (String) item.getValue("Address"))
            && isMaskAllowed(aliases, (String) item.getValue("Alias"));
  }

  public Set getAliasStrings()
  {
    if (empty)
      return new HashSet();
    else
      return aliases.getNames();
  }

  public Set getAddressStrings()
  {
    if (empty)
      return new HashSet();
    else
      return addresses.getNames();
  }
}
