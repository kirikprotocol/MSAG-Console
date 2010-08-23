package ru.novosoft.smsc.infosme.backend.config;

import java.util.List;
import java.util.ArrayList;

/**
 * User: artem
 * Date: 26.01.2009
 */
public class Changes {

  private final List added = new ArrayList();
  private final List modified = new ArrayList();
  private final List deleted = new ArrayList();
  private final List archivated = new ArrayList();

  public void added(Object o) {
    added.add(o);
  }

  public boolean isAdded(Object o) {
    return added.contains(o);
  }

  public void modified(Object o) {
    modified.add(o);
  }

  public boolean isModified(Object o) {
    return modified.contains(o);
  }

  public void deleted(Object o) {
    deleted.add(o);
  }

  public void archivated(Object o) {
    archivated.add(o);
  }

  public boolean isDeleted(Object o) {
    return deleted.contains(o);
  }

  public boolean isArchivated(Object o) {
    return archivated.contains(o);
  }

  public List getAdded() {
    return added;
  }

  public List getDeleted() {
    return deleted;
  }

  public List getArchivated() {
    return archivated;
  }

  public List getModified() {
    return modified;
  }
}
