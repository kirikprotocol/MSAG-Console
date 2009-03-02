package ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist;

import com.eyeline.utils.tree.radix.StringsRTree;
import ru.sibinco.smsx.engine.service.group.datasource.DistrList;
import ru.sibinco.smsx.engine.service.group.datasource.DistrListDataSource;
import ru.sibinco.smsx.utils.DataSourceException;

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * User: artem
 * Date: 19.11.2008
 */
public class FileDistrListDataSource implements DistrListDataSource {

  private final MembersFile membersFile;
  private final SubmittersFile submittersFile;
  private final ListsFile listsFile;

  private final StringsRTree<DistrList> distrLists;
  private final Map<Long, DistrList> offsetsList;
  private final Lock lock = new ReentrantLock();

  public FileDistrListDataSource(File membersFile, File submittersFile, File listsFile) throws DataSourceException {
    try {
      this.membersFile = new MembersFile(membersFile);
      this.submittersFile = new SubmittersFile(submittersFile);
      this.listsFile = new ListsFile(listsFile);

      this.distrLists = new StringsRTree<DistrList>();
      this.offsetsList = new HashMap<Long, DistrList>();

      loadLists();
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  private static String getListName(String name, String owner) {
    return (owner != null) ? owner + '/' + name.toLowerCase() : name.toLowerCase();
  }

  private static String getStoreListName(String name, String owner) {
    if (owner != null) {
      if (owner.charAt(0) == '+')
        owner = owner.substring(1);
      return owner + '/' + name;
    }
    return name;
  }

  private static String getNameFromStoreListName(String storeListName) {
    int i=storeListName.indexOf('/');
    return (i == -1) ? storeListName : storeListName.substring(i+1);
  }

  private static String getOwnerFromStoreListName(String storeListName) {
    int i=storeListName.indexOf('/');
    return (i == -1) ? null : '+' + storeListName.substring(0, i);
  }

  private void loadLists() throws IOException {

    // Load lists
    listsFile.list(new ListsFile.Visitor() {
      public void list(String storeListName, String owner, int maxElements, long offset) {
        String name = getNameFromStoreListName(storeListName);
        FileDistrList dl = new FileDistrList(FileDistrListDataSource.this, offset, name, owner, maxElements);

        distrLists.put(getListName(name, owner), dl);
        offsetsList.put(offset, dl);
      }
    });

    // Load members
    membersFile.list(new MembersFile.Visitor() {
      public void member(String storeListName, String address, long offset) {
        String name = getNameFromStoreListName(storeListName);
        String owner = getOwnerFromStoreListName(storeListName);
        FileDistrList lst = (FileDistrList)getDistrList(name, owner);
        if (lst != null)
          lst.addMember(address, offset);
        else
          System.out.println("    WARNING: Found member " + address + " for unknown list " + getListName(name, owner));
      }
    });

    // Load submitters
    submittersFile.list(new SubmittersFile.Visitor() {
      public void submitter(String storeListName, String address, long offset) {
        String name = getNameFromStoreListName(storeListName);
        String owner = getOwnerFromStoreListName(storeListName);
        FileDistrList lst = (FileDistrList)getDistrList(name, owner);
        if (lst != null)
          lst.addSubmitter(address, offset);
        else
          System.out.println("    WARNING: Found submitter " + address + " for unknown list " + getListName(name, owner));
      }
    });
  }

  public DistrList createDistrList(String name, String owner, int maxElements) throws DataSourceException {
    try {
      lock.lock();
      FileDistrList list = (FileDistrList)distrLists.get(getListName(name, owner));
      if (list != null)
        throw new IllegalArgumentException("List: name=" + name + "; owner=" + owner + " already exists.");

      long offset = listsFile.addList(getStoreListName(name, owner), owner, maxElements);
      list = new FileDistrList(this, offset, name, owner, maxElements);

      distrLists.put(getListName(name, owner), list);
      offsetsList.put(offset, list);

      return list;
    } catch (IOException e) {
      throw new DataSourceException(e);
    } finally {
      lock.unlock();
    }
  }

  public void removeDistrList(String name, String owner) throws DataSourceException {
    try {
      lock.lock();

      String listName = getListName(name, owner);
      FileDistrList lst = (FileDistrList)distrLists.get(listName);
      if (lst != null) {
        lst.clear();
        listsFile.removeList(lst.offset);

        distrLists.remove(listName);
        offsetsList.remove(lst.offset);
      }

    } catch (IOException e) {
      throw new DataSourceException(e);
    } finally {
      lock.unlock();
    }
  }

  public void removeDistrLists(String owner) throws DataSourceException {
    try {
      lock.lock();
      for (DistrList l : getDistrLists(owner))
        removeDistrList(l.getName(), l.getOwner());
    } finally {
      lock.unlock();
    }
  }

  public DistrList getDistrList(String name, String owner) {
    try {
      lock.lock();
      return distrLists.get(getListName(name, owner));
    } finally {
      lock.unlock();
    }
  }

  public DistrList getDistrList(long id) throws DataSourceException {
    try {
      lock.lock();
      return offsetsList.get(id);
    } finally {
      lock.unlock();
    }
  }

  public Collection<DistrList> getDistrLists(String owner) throws DataSourceException {
    try {
      lock.lock();
      return owner == null ? distrLists.values() : distrLists.values(owner + '/');
    } finally {
      lock.unlock();
    }
  }

  public boolean containsDistrList(String name, String owner) throws DataSourceException {
    try {
      lock.lock();
      return distrLists.get(getListName(name, owner)) != null;
    } finally {
      lock.unlock();
    }
  }

  public int size() {
    try {
      lock.lock();
      return offsetsList.size();
    } finally {
      lock.unlock();
    }
  }

  long addMember(String listName, String owner, String member) throws DataSourceException {
    try {
      return membersFile.addMember(getStoreListName(listName, owner), member);
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  void removeMember(long offset) throws DataSourceException {
    try {
      membersFile.removeMember(offset);
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  long addSubmitter(String listName, String owner, String submitter) throws DataSourceException {
    try {
      return submittersFile.addSubmitter(getStoreListName(listName, owner), submitter);
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  void removeSubmitter(long offset) throws DataSourceException {
    try {
      submittersFile.removeSubmitter(offset);
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  void setMaxElements(long offset, int maxElements) throws DataSourceException {
    try {
      listsFile.alterList(offset, maxElements);
    } catch (IOException e) {
      throw new DataSourceException(e);
    }
  }

  public void close() {
    try {
      lock.lock();
      membersFile.close();
      submittersFile.close();
      listsFile.close();
    } finally {
      lock.unlock();
    }
  }

}
