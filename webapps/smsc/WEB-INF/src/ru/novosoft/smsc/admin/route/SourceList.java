/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:45:20
 */
package ru.novosoft.smsc.admin.route;

import java.util.*;


public class SourceList
{
  Map sources = new HashMap();

  public SourceList()
  {
  }

  public void add(Source s)
  {
    if (s == null)
      throw new NullPointerException("Source is null");
    if (sources.containsKey(s.getName()))
      throw new IllegalArgumentException("Source already contained");

    sources.put(s.getName(), s);
  }

  public int size()
  {
    return sources.size();
  }

  public Source get(String sourceName)
  {
    return (Source) sources.get(sourceName);
  }

  public Iterator iterator()
  {
    return sources.values().iterator();
  }

  public boolean isEmpty()
  {
    return sources.isEmpty();
  }

  public Set getNames()
  {
    return sources.keySet();
  }

  public Source remove(String name)
  {
    return (Source) sources.remove(name);
  }

  public boolean removeAll(SourceList srcs)
  {
    return sources.keySet().removeAll(srcs.getNames());
  }

  public void addAll(SourceList srcs)
  {
    sources.putAll(srcs.sources);
  }

  public boolean retainAll(SourceList srcs)
  {
    return sources.keySet().retainAll(srcs.getNames());
  }

  public Set getSubjectNames()
  {
    Set result = new HashSet();
    for (Iterator i = sources.values().iterator(); i.hasNext();) {
      Source s = (Source) i.next();
      if (s.isSubject())
        result.add(s.getName());
    }

    return result;
  }

  public Set getMaskNames()
  {
    Set result = new HashSet();
    for (Iterator i = sources.values().iterator(); i.hasNext();) {
      Source s = (Source) i.next();
      if (!s.isSubject())
        result.addAll(s.getMasks().getNames());
    }

    return result;
  }
}
