/*
 * Author: igork
 * Date: 15.05.2002
 * Time: 15:52:01
 */
package ru.novosoft.smsc.admin.route;

import org.apache.log4j.Category;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.subject.SubjectDataSource;
import ru.novosoft.smsc.jsp.util.tables.impl.subject.SubjectQuery;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.xml.Utils;

import java.io.PrintWriter;
import java.util.*;


public class SubjectList
{
  private Map subjects = new HashMap();
  private SubjectDataSource dataSource = new SubjectDataSource();
  private Category logger = Category.getInstance(this.getClass());

  public SubjectList(Element listElement, SmeManager smeManager)
          throws AdminException
  {
    try {
    NodeList subjList = listElement.getElementsByTagName("subject_def");
    for (int i = 0; i < subjList.getLength(); i++) {
      Element subjElem = (Element) subjList.item(i);
      String name = subjElem.getAttribute("id");
      NodeList masksList = subjElem.getElementsByTagName("mask");
      String[] masks = new String[masksList.getLength()];
      for (int j = 0; j < masksList.getLength(); j++) {
        Element maskElem = (Element) masksList.item(j);
        masks[j] = maskElem.getAttribute("value").trim();
      }
      SME defSme = smeManager.get(subjElem.getAttribute("defSme"));
      if (defSme == null)
        throw new AdminException("Unknown SME \"" + subjElem.getAttribute("defSme") + '"');

      String notes = "";
      NodeList notesList = subjElem.getElementsByTagName("notes");
      for (int j=0; j < notesList.getLength(); j++)
        notes += Utils.getNodeText(notesList.item(j));

      try {
        add(new Subject(name, masks, defSme, notes));
      } catch (AdminException e) {
        logger.warn("source skipped", e);
      }
    }
    } catch (Throwable ex) {
      logger.error("Couldn't parse subjects", ex);
      throw new AdminException("Couldn't parse subjects");
    }
  }

  public void add(Subject s)
          throws AdminException
  {
    if (s == null)
      throw new NullPointerException("Source is null");
    if (subjects.containsKey(s.getName()))
      throw new AdminException("Source \"" + s.getName() + "\" already contained");

    dataSource.add(s);
    subjects.put(s.getName(), s);
  }

  public Subject remove(String subjectName)
  {
    Subject removed = (Subject) subjects.remove(subjectName);
    if (removed != null)
      dataSource.remove(removed);
    return removed;
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

  public List getNames()
  {
    return new SortedList(subjects.keySet());
  }

  public PrintWriter store(PrintWriter out)
  {
    for (Iterator i = iterator(); i.hasNext();) {
      ((Subject) i.next()).store(out);
    }
    return out;
  }

  public QueryResultSet query(SubjectQuery query)
  {
    dataSource.clear();
    for (Iterator i = subjects.values().iterator(); i.hasNext();) {
      Subject subject = (Subject) i.next();
      dataSource.add(subject);
    }
    return dataSource.query(query);
  }

  public boolean contains(String subjectName)
  {
    return subjects.keySet().contains(subjectName);
  }
}
