/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 15:13:49
 */
package ru.novosoft.smsc.admin.route;

import java.util.*;


public class MaskList
{
  private Map masks = new HashMap();

  public MaskList()
  {
  }

  public MaskList(String masks)
  {
    for (StringTokenizer tokenizer = new StringTokenizer(masks); tokenizer.hasMoreTokens();) {
      add(new Mask(tokenizer.nextToken()));
    }
  }

  public MaskList(Mask mask)
  {
    add(mask);
  }

  public Mask add(Mask newMask)
  {
    if (newMask == null)
      throw new NullPointerException("Mask is null");

    return (Mask) masks.put(newMask.getMask(), newMask);
  }

  public Mask get(String key)
  {
    return (Mask) masks.get(key);
  }

  public int size()
  {
    return masks.size();
  }

  public Iterator iterator()
  {
    return masks.values().iterator();
  }

  public String getText()
  {
    String result = "";
    for (Iterator i = masks.keySet().iterator(); i.hasNext();) {
      result += (String) i.next() + "<br>";
    }
    return result;
  }

  public Set getNames()
  {
    return masks.keySet();
  }
}
