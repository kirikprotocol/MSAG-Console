/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 15:13:49
 */
package ru.sibinco.lib.backend.route;

import org.apache.log4j.Logger;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.SortedList;

import java.io.PrintWriter;
import java.util.*;


public class MaskList
{
  private Map masks = new HashMap();
  private Logger logger = Logger.getLogger(this.getClass().getName());

  public MaskList()
  {
  }

  public MaskList(String masks)
  {
    logger.debug("enter " + this.getClass().getName() + ".MaskList(" + masks + ")");
    for (StringTokenizer tokenizer = new StringTokenizer(masks); tokenizer.hasMoreTokens();) {
      final String mask = tokenizer.nextToken();
      try {
        add(new Mask(mask));
      } catch (SibincoException e) {
        logger.error("Couldn't load mask \"" + mask + "\", skipped", e);
      }
    }
    logger.debug("exit " + this.getClass().getName() + ".MaskList(" + masks + ")");
  }

  public MaskList(String[] masks) throws SibincoException
  {
    logger.debug("enter " + this.getClass().getName() + ".MaskList(" + masks + ")");
    for (int i = 0; i < masks.length; i++) {
      String mask = masks[i].trim();
      try {
        add(new Mask(mask));
      } catch (SibincoException e) {
        logger.error("Couldn't load mask \"" + mask + "\", skipped", e);
        throw e;
      }
    }
    logger.debug("exit " + this.getClass().getName() + ".MaskList(" + masks + ")");
  }

  public MaskList(Collection masks) throws SibincoException
  {
    logger.debug("enter " + this.getClass().getName() + ".MaskList(" + masks + ")");
    for (Iterator i = masks.iterator(); i.hasNext();) {
      String mask = ((String) i.next()).trim();
      try {
        add(new Mask(mask));
      } catch (SibincoException e) {
        logger.error("Couldn't load mask \"" + mask + "\", skipped", e);
        throw e;
      }
    }
    logger.debug("exit " + this.getClass().getName() + ".MaskList(" + masks + ")");
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

  public SortedList getNames()
  {
    return new SortedList(masks.keySet());
  }

  public PrintWriter store(PrintWriter out)
  {
    for (Iterator i = iterator(); i.hasNext();) {
      ((Mask) i.next()).store(out);
    }
    return out;
  }

  public boolean isEmpty()
  {
    return masks.size() == 0;
  }

  public void clear()
  {
    masks.clear();
  }

  public static String[] normalizeMaskList(String[] masks) throws SibincoException
  {
    return (String[]) new MaskList(masks).getNames().toArray(new String[0]);
  }

  public boolean contains(Mask mask)
  {
    Mask m = get(mask.getMask());
    return m != null && m.equals(mask);
  }
}
