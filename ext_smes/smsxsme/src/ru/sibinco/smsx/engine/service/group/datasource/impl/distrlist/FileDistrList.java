package ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist;

import ru.sibinco.smsx.engine.service.group.datasource.DistrList;
import ru.sibinco.smsx.utils.DataSourceException;

import java.util.Collection;
import java.util.Collections;
import java.util.Vector;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * User: artem
 * Date: 19.11.2008
 */
class FileDistrList implements DistrList {

  private final FileDistrListDataSource ds;

  private final String name;
  private final String owner;
  final long offset;
  private int maxElements;

  private IndexedList members;

  private IndexedList submitters;

  public FileDistrList(FileDistrListDataSource ds, long offset, String name, String owner, int maxElements) {
    this.ds = ds;
    this.name = name;
    this.offset = offset;
    this.owner = owner;
    this.maxElements = maxElements;
  }

  public int getMaxElements() {
    return maxElements;
  }

  public void setMaxElements(int maxElements) throws DataSourceException {
    ds.setMaxElements(offset, maxElements);
    this.maxElements = maxElements;
  }

  public int getId() {
    return (int)offset;
  }

  public String getName() {
    return name;
  }

  public String getOwner() {
    return owner;
  }

  public void clear() throws DataSourceException {
    if (members != null) {
      for (int i=0; i<members.index.size(); i++)
        ds.removeMember(members.index.get(i));
    }

    if (submitters != null) {
      for (int i=0; i<submitters.index.size(); i++)
        ds.removeSubmitter(submitters.index.get(i));
    }

    members = null;
    submitters = null;
  }

  public Collection<String> members() throws DataSourceException {
    if (members == null)
      return Collections.emptyList();
    return members.values;
  }

  void addMember(String member, long offset) {
    if (members == null)
      members = new IndexedList();
    members.add(member, offset);
  }

  public void addMember(String member) throws DataSourceException {
    if (members == null || !members.contains(member)) {
      long offset = ds.addMember(name, owner, member);
      addMember(member, offset);
    }
  }

  public boolean removeMember(String member) throws DataSourceException {
    if (members != null) {
      long offset = members.remove(member);
      if (offset >= 0) {
        ds.removeMember(offset);
        if (members.isEmpty()) members = null;
        return true;
      }
    }
    return false;
  }

  public boolean containsMember(String member) throws DataSourceException {
    return members != null && members.contains(member);
  }

  public int membersSize() throws DataSourceException {
    return members == null ? 0 : members.size();
  }


  public Collection<String> submitters() throws DataSourceException {
    if (submitters == null)
      return Collections.emptyList();
    return submitters.values;
  }

  void addSubmitter(String submitter, long offset) {
    if (submitters == null)
      submitters = new IndexedList();
    submitters.add(submitter, offset);
  }

  public void addSubmitter(String submitter) throws DataSourceException {
    if (submitters == null || !submitters.contains(submitter)) {
      long offset = ds.addSubmitter(name, owner, submitter);
      addSubmitter(submitter,  offset);
    }
  }

  public boolean removeSubmitter(String submitter) throws DataSourceException {
    if (submitters != null) {
      long offset = submitters.remove(submitter);
      if (offset >= 0) {
        ds.removeSubmitter(offset);
        if (submitters.isEmpty()) submitters = null;
        return true;
      }
    }
    return false;
  }

  public boolean containsSubmitter(String address) throws DataSourceException {
    return submitters != null && submitters.contains(address);
  }

  public int submittersSize() throws DataSourceException {
    return submitters == null ? 0 : submitters.size();
  }

  public void copyFrom(DistrList list) throws DataSourceException {
    for (String s : list.members())
      addMember(s);
    for (String s : list.submitters())
      addSubmitter(s);
  }


  private static class IndexedList {
    private final Vector<String> values = new Vector<String>();
    private final Vector<Long> index = new Vector<Long>();

    private final Lock lock = new ReentrantLock();

    public void add(String value, long index) {
      try {
        lock.lock();
        this.values.add(value);
        this.index.add(index);
      } finally {
        lock.unlock();
      }
    }

    public long remove(String value) {
      try {
        lock.lock();
        int id = values.indexOf(value);
        if (id != -1) {
          values.remove(id);
          return index.remove(id);
        }
        return -1;
      } finally {
        lock.unlock();
      }
    }

    public boolean contains(String value) {
      try {
        lock.lock();
        return values.contains(value);
      } finally {
        lock.unlock();
      }
    }

    public boolean isEmpty() {
      return values.isEmpty();
    }

    public int size() {
      return values.size();
    }
  }
}
