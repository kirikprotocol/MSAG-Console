package ru.sibinco.scag.backend.protocol.tables.impl.alias;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 19:30:25
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.scag.backend.protocol.tables.Filter;
import ru.sibinco.scag.backend.protocol.tables.DataItem;
import ru.sibinco.lib.backend.route.MaskList;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.SibincoException;

import java.util.Iterator;


public class AliasFilter implements Filter
{
  public static final byte HIDE_NOFILTER = 0;
  public static final byte HIDE_SHOW_HIDE = 1;
  public static final byte HIDE_SHOW_NOHIDE = 2;
  public static final byte HIDE_UNKNOWN = -1;

  private MaskList aliases = null;
  private MaskList addresses = null;
  private byte hide = HIDE_NOFILTER;


  public AliasFilter()
  {
    aliases = new MaskList();
    addresses = new MaskList();
    hide = HIDE_NOFILTER;
  }

  public AliasFilter(String[] alias_masks, String[] address_masks, byte hide_option) throws SibincoException
  {
    setAliases(alias_masks);
    setAddresses(address_masks);
    setHide(hide_option);
  }

  public boolean isEmpty()
  {
    return aliases.isEmpty() && addresses.isEmpty() && hide == HIDE_NOFILTER;
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
    if (isEmpty())
      return true;

    return isMaskAllowed(addresses, (String) item.getValue("Address")) && isMaskAllowed(aliases, (String) item.getValue("Alias")) && (hide == HIDE_NOFILTER || (hide == HIDE_SHOW_HIDE && ((Boolean) item.getValue("Hide")).booleanValue()) || (hide == HIDE_SHOW_NOHIDE && !((Boolean) item.getValue("Hide")).booleanValue()));
  }

  public String[] getAliases()
  {
    return (String[]) aliases.getNames().toArray(new String[0]);
  }

  public String[] getAddresses()
  {
    return (String[]) addresses.getNames().toArray(new String[0]);
  }

  public byte getHide()
  {
    return hide;
  }

  public void setAliases(String[] newAliasStrings) throws SibincoException
  {
    aliases = new MaskList(newAliasStrings);
  }

  public void setAddresses(String[] newAddressStrings) throws SibincoException
  {
    addresses = new MaskList(newAddressStrings);
  }

  public void setHide(byte newHideOption)
  {
    hide = newHideOption;
  }
}
