/*
 * Author: igork
 * Date: 15.05.2002
 * Time: 15:52:01
 */
package ru.sibinco.lib.backend.route;


/**
 * todo: remove it
 * 
 * @see ru.sibinco.lib.backend.util.SortedList
 * @see ru.sibinco.lib.backend.util.SortByPropertyComparator
 */
/*
public class SubjectList
{
  private Map subjects = new HashMap();
  //private SubjectDataSource dataSource = new SubjectDataSource();
  private Logger logger = Logger.getLogger(this.getClass().getName());

  public SubjectList(Element listElement, SmeManager smeManager)
      throws SibincoException
  {
    try {
      logger.debug("enter " + this.getClass().getName() + ".SubjectList(" + listElement + ", " + smeManager + ")");
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
        Sme defSme = (Sme) smeManager.getSmes().get(subjElem.getAttribute("defSme"));
        if (defSme == null)
          throw new SibincoException("Unknown Sme \"" + subjElem.getAttribute("defSme") + '"');

        String notes = "";
        NodeList notesList = subjElem.getElementsByTagName("notes");
        for (int j = 0; j < notesList.getLength(); j++)
          notes += Utils.getNodeText(notesList.item(j));

        try {
          add(new Subject(name, masks, defSme, notes));
        } catch (SibincoException e) {
          logger.warn("Cannot create source subject, skipped", e);
        }
      }
      logger.debug("exit " + this.getClass().getName() + ".SubjectList(" + listElement + ", " + smeManager + ")");
    } catch (Throwable ex) {
      logger.error("Couldn't parse subjects", ex);
      throw new SibincoException("Couldn't parse subjects", ex);
    }
  }

  public void add(Subject s)
      throws SibincoException
  {
    if (s == null)
      throw new NullPointerException("Source is null");
    if (subjects.containsKey(s.getName()))
      throw new SibincoException("Source \"" + s.getName() + "\" already contained");

    //dataSource.add(s);
    subjects.put(s.getName(), s);
  }

  public Subject remove(String subjectName)
  {
    Subject removed = (Subject) subjects.remove(subjectName);
/*
    if (removed != null)
      dataSource.remove(removed);
* /
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

/*
  public QueryResultSet query(SubjectQuery query)
  {
    dataSource.clear();
    for (Iterator i = subjects.values().iterator(); i.hasNext();) {
      Subject subject = (Subject) i.next();
      dataSource.add(subject);
    }
    return dataSource.query(query);
  }

* /
  public boolean contains(String subjectName)
  {
    return subjects.keySet().contains(subjectName);
  }
}
*/
