package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class Subject implements Serializable {

  private static final transient ValidationHelper vh = new ValidationHelper(Subject.class);

  private String name;
  private String notes = "";
  private String defaultSmeId;
  private List<Address> masks;
  private List<String> children;

  public Subject() {
  }

  public Subject(String name) throws AdminException {
    vh.checkNotEmpty("name", name);
    this.name = name;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) throws AdminException {
    vh.checkNotEmpty("name", name);
    this.name = name;
  }

  public String getDefaultSmeId() {
    return defaultSmeId;
  }

  public void setDefaultSmeId(String defaultSmeId) {
    this.defaultSmeId = defaultSmeId;
  }

  public List<Address> getMasks() {
    return masks;
  }

  public void setMasks(List<Address> masks) throws AdminException {
    vh.checkNoNulls("masks", masks);
    this.masks = new ArrayList<Address>(masks);
  }

  public List<String> getChildren() {
    if (children == null)
      return null;
    return new ArrayList<String>(children);
  }

  public void setChildren(List<String> children) throws AdminException {
    vh.checkNoNulls("children", children);
    this.children = children;
  }

  public String getNotes() {
    return notes;
  }

  public void setNotes(String notes) {
    this.notes = notes;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Subject subject = (Subject) o;

    if (name != null ? !name.equals(subject.name) : subject.name != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    return name != null ? name.hashCode() : 0;
  }
}
