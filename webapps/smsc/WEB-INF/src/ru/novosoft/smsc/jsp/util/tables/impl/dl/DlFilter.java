package ru.novosoft.smsc.jsp.util.tables.impl.dl;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

/**
 * Created by igork
 * Date: 13.03.2003
 * Time: 22:07:01
 */
public class DlFilter implements Filter
{
  private String[] names = new String[0];
  private String[] owners = new String[0];

  public boolean isEmpty()
  {
    return (names == null || names.length == 0) && (owners == null || owners.length == 0);
  }

  public boolean isItemAllowed(DataItem item)
  {
    if (isEmpty())
      return true;
    if (names != null)
      for (int i = 0; i < names.length; i++) {
        if (((String) item.getValue("name")).matches(names[i] + ".*"))
          return true;
      }
    if (owners != null)
      for (int i = 0; i < owners.length; i++) {
        final Mask mask = (Mask) item.getValue("owner");
        if (mask != null && mask.getMask().matches(owners[i] + ".*"))
          return true;
      }
    return false;
  }

  private boolean isStringArrayContainsNonAlphanumeric(String[] strings)
  {
    for (int i = 0; i < strings.length; i++) {
      String str = strings[i];
      for (int j = 0; j < str.length(); j++) {
        final char ch = str.charAt(j);
        if (!Character.isLetterOrDigit(ch) && !Character.isSpaceChar(ch))
          return true;
      }
    }
    return false;
  }

  public boolean isNamesContainsRegexp()
  {
    return isStringArrayContainsNonAlphanumeric(names);
  }

  public boolean isOwnersContainsRegexp()
  {
    return isStringArrayContainsNonAlphanumeric(owners);
  }

  public String[] getNames()
  {
    return names;
  }

  public void setNames(String[] names)
  {
    this.names = names;
  }

  public String[] getOwners()
  {
    return owners;
  }

  public void setOwners(String[] owners)
  {
    this.owners = owners;
  }
}
