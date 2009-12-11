package ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist;

import ru.sibinco.smsx.engine.service.group.datasource.DistrList;
import ru.sibinco.smsx.utils.DataSourceException;

import java.util.Collection;
import java.util.ArrayList;
import java.util.HashSet;

/**
 * User: artem
 * Date: 02.12.2009
 */
public class DBDistrList implements DistrList {

  private final int id;
  private String name;
  private String owner;
  private int maxElements;
  private Collection<String> members;
  private boolean membersHaveLoaded = false;
  private Collection<String> submitters;
  private boolean submitterHaveLoaded = false;

  private final DBDistrListDataSource ds;

  public DBDistrList(int id, String name, String owner, int maxElements, DBDistrListDataSource ds) {
    this.id = id;
    this.name = name;
    this.owner = owner;
    this.maxElements = maxElements;
    this.ds = ds;
  }

  public int getId() {
    return id;
  }

  public String getName() {
    return name;
  }

  public String getOwner() {
    return owner;
  }

  public int getMaxElements() {
    return maxElements;
  }

  public void setMaxElements(int maxElements) throws DataSourceException {
    ds.setMaxElements(id, maxElements);
    this.maxElements = maxElements;
  }

  private synchronized void loadMembers() throws DataSourceException {
    if (!membersHaveLoaded) {
      members = new HashSet<String>(ds.getMembers(id));
      membersHaveLoaded = true;
    }
  }

  public Collection<String> members() throws DataSourceException {
    loadMembers();
    return new ArrayList<String>(members);
  }

  public void addMember(String member) throws DataSourceException {
    loadMembers();
    if (!members.contains(member)) {
      ds.addMember(id, member);
      members.add(member);
    }
  }

  public boolean removeMember(String member) throws DataSourceException {
    loadMembers();
    if (members.contains(member)) {
      ds.removeMember(id, member);
      members.remove(member);
      return true;
    }
    return false;
  }

  public boolean containsMember(String member) throws DataSourceException {
    loadMembers();
    return members.contains(member);
  }

  public int membersSize() throws DataSourceException {
    loadMembers();
    return members.size();    
  }

  private synchronized void loadSubmitters() throws DataSourceException {
    if (!submitterHaveLoaded) {
      submitters = new HashSet<String>(ds.getSubmitters(id));
      submitterHaveLoaded = true;
    }
  }

  public Collection<String> submitters() throws DataSourceException {
    loadSubmitters();
    return new ArrayList<String>(submitters);
  }

  public void addSubmitter(String submitter) throws DataSourceException {
    loadSubmitters();
    if (!submitters.contains(submitter)) {
      ds.addSubmitter(id, submitter);
      submitters.add(submitter);
    }
  }

  public boolean removeSubmitter(String submitter) throws DataSourceException {
    loadSubmitters();
    if (submitters.contains(submitter)) {
      ds.removeSubmitter(id, submitter);
      submitters.remove(submitter);
      return true;
    }
    return false;
  }

  public boolean containsSubmitter(String address) throws DataSourceException {
    loadSubmitters();
    return submitters.contains(address);
  }

  public int submittersSize() throws DataSourceException {
    loadSubmitters();
    return submitters.size();    
  }

  public synchronized  void copyFrom(DistrList list) throws DataSourceException {
    ds.copyMembersAndSubmitters(id, list);
    submitterHaveLoaded = true;
    submitters = new HashSet<String>(list.submitters());
    membersHaveLoaded = true;
    members = new HashSet<String>(list.members());
  }
}
