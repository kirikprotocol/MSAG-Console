/*
 * Author: igork
 * Date: 15.05.2002
 * Time: 15:52:01
 */
package ru.novosoft.smsc.admin.route;

import java.util.Map;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;


public class SubjectList
{
  private Map subjects = new HashMap();

  public SubjectList()
  {
  }

  public void add(Subject s)
  {
    if (s == null)
      throw new NullPointerException("Source is null");
    if (subjects.containsKey(s.getName()))
      throw new IllegalArgumentException("Source already contained");

    subjects.put(s.getName(), s);
  }

  public int size()
  {
    return subjects.size();
  }

  public Subject get(String subjectName)
  {
    return (Subject) subjects.get(subjectName);
  }

  public Iterator iterator()
  {
    return subjects.values().iterator();
  }

  public boolean isEmpty()
  {
    return subjects.isEmpty();
  }

  public Set getNames()
  {
    return subjects.keySet();
  }
}
