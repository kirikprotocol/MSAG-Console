/*
 * Author: igork
 * Date: 15.05.2002
 * Time: 15:52:01
 */
package ru.novosoft.smsc.admin.route;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;

import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;


public class SubjectList
{
  private Map subjects = new HashMap();

  public SubjectList(Element listElement, SMEList smes)
          throws AdminException
  {
    NodeList subjList = listElement.getElementsByTagName("subject_def");
    for (int i = 0; i < subjList.getLength(); i++)
    {
      Element subjElem = (Element) subjList.item(i);
      String name = subjElem.getAttribute("id");
      String masks = "";
      NodeList masksList = subjElem.getElementsByTagName("mask");
      for (int j = 0; j < masksList.getLength(); j++)
      {
        Element maskElem = (Element) masksList.item(j);
        masks += maskElem.getAttribute("value") + "\n";
      }
      SME defSme = smes.get(subjElem.getAttribute("defSme"));
      if (defSme == null)
        throw new AdminException("Unknown SME \"" + subjElem.getAttribute("defSme") + '"');
      add(new Subject(name, masks, defSme));
    }
  }

  public void add(Subject s)
  {
    if (s == null)
      throw new NullPointerException("Source is null");
    if (subjects.containsKey(s.getName()))
      throw new IllegalArgumentException("Source already contained");

    subjects.put(s.getName(), s);
  }

  public Subject remove(String subjectName)
  {
    return (Subject) subjects.remove(subjectName);
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

  public PrintWriter store(PrintWriter out)
  {
    for (Iterator i = iterator(); i.hasNext();)
    {
      ((Subject) i.next()).store(out);
    }
    return out;
  }
}
