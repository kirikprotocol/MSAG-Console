package ru.novosoft.smsc.admin.route;

/*
 * Author: igork
 * Date: 15.05.2002
 * Time: 15:52:01
 */

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

        NodeList childList = subjElem.getElementsByTagName("subject");
        String[] childs = new String[childList.getLength()];
        for (int j = 0; j < childList.getLength(); j++) {
          Element childElem = (Element) childList.item(j);
          childs[j] = childElem.getAttribute("id").trim();
        }

        SME defSme = smeManager.get(subjElem.getAttribute("defSme"));
        if (defSme == null)
          throw new AdminException("Unknown SME \"" + subjElem.getAttribute("defSme") + '"');

        String notes = "";
        NodeList notesList = subjElem.getElementsByTagName("notes");
        for (int j = 0; j < notesList.getLength(); j++)
          notes += Utils.getNodeText(notesList.item(j));

        try {
          add(new Subject(name, masks, defSme, notes, childs));
        } catch (AdminException e) {
          logger.warn("source skipped", e);
        }
      }
    } catch (Throwable ex) {
      ex.printStackTrace();
      logger.error("Couldn't parse subjects", ex);
      throw new AdminException("Couldn't parse subjects");
    }
  }

  public synchronized void add(Subject s)
          throws AdminException
  {
    if (s == null)
      throw new NullPointerException("Source is null");
    if (subjects.containsKey(s.getName()))
      throw new AdminException("Source \"" + s.getName() + "\" already contained");

    dataSource.add(s);
    subjects.put(s.getName(), s);
  }

  public synchronized Subject remove(String subjectName)
  {
    Subject removed = (Subject) subjects.remove(subjectName);
    if (removed != null)
      dataSource.remove(removed);
    return removed;
  }

  public synchronized int size()
  {
    return subjects.size();
  }

  public synchronized Subject get(String subjectName)
  {
    return (Subject) subjects.get(subjectName);
  }

  public synchronized Iterator iterator()
  {
    return subjects.values().iterator();
  }

  public synchronized boolean isEmpty()
  {
    return subjects.isEmpty();
  }

  public synchronized List getNames()
  {
    return new SortedList(subjects.keySet());
  }

  public synchronized void check() throws AdminException {
    // Try to find circles in subjects graph
    // Construct graph matrix
    final int[][] matrix = new int[subjects.size()][subjects.size()];
    for (int i=0; i < matrix.length; i++) {
      for (int j = 0; j < matrix.length; j++)
        matrix[i][j] = 0;
    }
    final ArrayList subjectNames = new ArrayList();
    for (Iterator i = iterator(); i.hasNext();)
      subjectNames.add(((Subject) i.next()).getName());

    // Fill graph matrix

    for (Iterator iter=iterator(); iter.hasNext();) {
      Subject subj = (Subject)iter.next();
      int i = subjectNames.indexOf(subj.getName());
      String[] childs = subj.getChildSubjects().getNames();
      for (int j = 0; j < childs.length; j++) {
        int k = subjectNames.indexOf(childs[j]);
        matrix[i][k] = 1;
      }
    }

    // Find circles

    // Find non circled elements
    boolean removed = true;
    final Set removedVertexes = new HashSet();
    while (removed) {
      removed = false;
      for (int i=0; i < matrix.length; i++) {
        if (removedVertexes.contains(new Integer(i))) // vertex was removed
          continue;

        // Check outgoing links
        boolean found = false;
        for (int j = 0; j < matrix.length && !found; j++)
          found = matrix[i][j] > 0;

        if (found) {
          // Check incoming links
          found = false;
          for (int j = 0; j < matrix.length && !found; j++)
            found = matrix[j][i] > 0;

          if (found) // Outgoing and incoming links was found
            continue;
        }

        // Vertex has just outgoing or just incoming links. Remove it.
        for (int j = 0; j < matrix.length; j++) {
          matrix[i][j] = -1;
          matrix[j][i] = -1;
        }
        removedVertexes.add(new Integer(i));
        removed = true;
      }
    }

    for (int i=0; i < matrix.length; i++) {
      for (int j = 0; j < matrix.length; j++)
        if (matrix[i][j] > 0) {
          final StringBuffer buffer = new StringBuffer();
          printCircle(matrix, subjectNames, i, j, buffer, new HashSet());
          throw new AdminException("Circle was found in subjects: " + buffer);
        }
    }
  }

  private void printCircle(int[][] matrix, ArrayList subjectNames, int i, int j, StringBuffer buffer, Set steps) {
    if (steps.contains(new Integer(i)))
      return;

    steps.add(new Integer(i));
    buffer.append(buffer.length() > 0 ? " -> " : "").append(subjectNames.get(i));

    for (int k =0; k < matrix.length; k++) {
      if (matrix[j][k] > 0) {
        printCircle(matrix, subjectNames, j, k, buffer, steps);
        return;
      }
    }
  }

  public synchronized PrintWriter store(PrintWriter out)
  {
    for (Iterator i = iterator(); i.hasNext();) {
      ((Subject) i.next()).store(out);
    }
    return out;
  }

  public synchronized QueryResultSet query(SubjectQuery query)
  {
    dataSource.clear();
    for (Iterator i = subjects.values().iterator(); i.hasNext();) {
      Subject subject = (Subject) i.next();
      dataSource.add(subject);
    }
    return dataSource.query(query);
  }

  public synchronized boolean contains(String subjectName)
  {
    return subjects.keySet().contains(subjectName);
  }
}
