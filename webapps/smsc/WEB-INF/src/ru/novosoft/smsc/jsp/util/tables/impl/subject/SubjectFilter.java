package ru.novosoft.smsc.jsp.util.tables.impl.subject;

/*
 * Author: igork
 * Date: 13.06.2002
 * Time: 15:44:31
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.Filter;

import java.util.*;


public class SubjectFilter implements Filter
{
  private Set names = new HashSet();
  private List smes = new LinkedList();
  private MaskList masks = new MaskList();

  public SubjectFilter()
  {
  }

  public SubjectFilter(Set names, List smeIds, String masks)
  {
    this.names = names;
    this.smes = smeIds;
    this.masks = new MaskList(masks);
  }

  public boolean isEmpty()
  {
    return this.names.isEmpty() && this.smes.isEmpty() && this.masks.isEmpty();
  }

  private boolean isMaskAllowed(String maskStr)
  {
    final Mask mask;
    try {
      mask = new Mask(maskStr);
    } catch (AdminException e) {
      return false;
    }

    for (Iterator i = masks.iterator(); i.hasNext();) {
      final Mask m = (Mask) i.next();
      if (m.addressConfirm(mask))
        return true;
    }
    return false;
  }

  private boolean isAnyMaskAllowed(Vector item_masks)
  {
    for (Iterator i = item_masks.iterator(); i.hasNext();) {
      if (isMaskAllowed((String) i.next()))
        return true;
    }
    return false;
  }

  public boolean isItemAllowed(DataItem item)
  {
    if (isEmpty())
      return true;

    String item_name = (String) item.getValue("Name");
    String item_sme = (String) item.getValue("Default SME");
    Vector item_masks = (Vector) item.getValue("Masks");
    return (names.isEmpty() || names.contains(item_name))
            && (smes.isEmpty() || smes.contains(item_sme))
            && (masks.isEmpty() || isAnyMaskAllowed(item_masks));
  }

  public List getMaskStrings()
  {
    return masks.getNames();
  }

  public Set getNames()
  {
    return names;
  }

  public List getSmeIds()
  {
    return smes;
  }

  public void setMasks(String[] newMasks) throws AdminException
  {
    this.masks = new MaskList(newMasks);
  }

  public void setNames(String[] newNames)
  {
    this.names = new HashSet(Arrays.asList(newNames));
  }

  public void setSmes(String[] newSmes)
  {
    this.smes = Arrays.asList(newSmes);
  }
}
