package ru.novosoft.smsc.admin.route;

import java.util.Set;
import java.util.HashSet;
import java.util.Iterator;
import java.io.Writer;
import java.io.PrintWriter;

/**
 * User: artem
 * Date: Aug 2, 2007
 */

public class ChildSubjectsList {
  private final Set subjects;

  public ChildSubjectsList() {
    subjects = new HashSet();
  }

  public ChildSubjectsList(String[] subjects) {
    this.subjects = new HashSet();
    for (int i=0; i < subjects.length; i++)
      this.subjects.add(subjects[i]);
  }

  public void addSubject(String subject) {
    subjects.add(subject);
  }

  public void addSubject(Subject subject) {
    subjects.add(subject.getName());
  }

  public String[] getNames() {
    String[] result = new String[subjects.size()];
    int i=0;
    for (Iterator iter = subjects.iterator(); iter.hasNext(); i++)
      result[i] = (String)iter.next();
    return result;
  }

  void store(PrintWriter out) {
    for (Iterator iter = subjects.iterator(); iter.hasNext();)
      out.println("    <subject id=\"" + iter.next() + "\"/>");
  }
}
