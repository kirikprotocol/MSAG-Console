package ru.novosoft.smsc.jsp.smsc.subjects;

import ru.novosoft.smsc.jsp.smsc.SmscBean;

import java.util.Set;
import java.util.List;
import java.util.Collection;

/**
 * User: artem
 * Date: Aug 2, 2007
 */

public class SubjectBody extends SmscBean {

  protected String name = null;
  protected String notes = "";
  protected String defSme = null;
  protected String[] checkedSources = null;
  protected String[] srcMasks = null;
  protected Set checkedSourcesSet = null;

  public Collection getAllSubjects() {
    return routeSubjectManager.getSubjects().getNames();
  }


  public List getPossibleSmes() {
    return smeManager.getSmeNames();
  }

  public boolean isSrcChecked(final String srcName) {
    return checkedSourcesSet.contains(srcName);
  }

  public String[] getCheckedSources() {
    return checkedSources;
  }

  public void setCheckedSources(String[] checkedSources) {
    this.checkedSources = checkedSources;
  }

  public String[] getSrcMasks() {
    return srcMasks;
  }

  public void setSrcMasks(String[] srcMasks) {
    this.srcMasks = srcMasks;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getNotes() {
    return notes;
  }

  public void setNotes(String notes) {
    this.notes = notes;
  }

  public String getDefSme() {
    return defSme;
  }

  public void setDefSme(String defSme) {
    this.defSme = defSme;
  }
}
