package ru.sibinco.smsx.engine.service.group.datasource.impl.distrlist;

import com.eyeline.utils.tree.radix.StringsRTree;

import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;
import java.lang.ref.Reference;
import java.util.Map;
import java.util.HashMap;
import java.util.Collection;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Lock;

import ru.sibinco.smsx.engine.service.group.datasource.DistrList;
import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 03.12.2009
 */
public class WeakDistrListCache {

  private static final Category log = Category.getInstance("GROUP");

  private final StringsRTree<DistrListReference> tree = new StringsRTree<DistrListReference>();

  private final Map<Integer, DistrListReference> hash = new HashMap<Integer, DistrListReference>();

  private final ReferenceQueue<DistrList> rqueue = new ReferenceQueue<DistrList>();

  private final Lock lock = new ReentrantLock();

  private void clearNullReferences() {
    Reference r;
    while ((r = rqueue.poll()) != null) {
      DistrListReference ref = (DistrListReference) r;
      DistrListReference ref1 = hash.get(ref.id);
      if (ref1 == ref) {
        if (log.isDebugEnabled())
          log.debug("Remove expired reference from cache: name=" + ref.name + "; owner=" + ref.owner);
        tree.remove(ref.owner + '_' + ref.name);
        hash.remove(ref.id);
      }
    }
  }

  public DistrList add(DistrList list) {
    try {
      lock.lock();
      clearNullReferences();

      DistrListReference ref = hash.get(list.getId());

      if (ref != null) {
        DistrList prevInst = ref.get();
        if (prevInst != null) {
          if (log.isDebugEnabled())
            log.debug("Group already exists in cache: name=" + list.getName() + "; owner=" + list.getOwner());
          return prevInst;
        }
      }

      ref = new DistrListReference(list, rqueue);
      if (log.isDebugEnabled())
        log.debug("Add group to cache: name=" + list.getName() + "; owner=" + list.getOwner());
      tree.put(list.getOwner() + '_' + list.getName(), ref);
      hash.put(list.getId(), ref);
      return list;

    } finally {
      lock.unlock();
    }
  }

  public DistrList get(String owner, String name) {
    try {
      lock.lock();
      clearNullReferences();
      DistrListReference ref = tree.get(owner + '_' + name);
      if (ref != null) {
        DistrList list = ref.get();
        if (log.isDebugEnabled() && list == null)
          log.debug("Reference to group was found in cache, but it is null reference: name=" + name + "; owner=" + owner);
        if (log.isDebugEnabled() && list != null)
          log.debug("Group was found in cache: name=" + name + "; owner=" + owner);
        return list;
      } else {
        if (log.isDebugEnabled())
          log.debug("Group was not found in cache: name=" + name + "; owner=" + owner);
        return null;
      }
    } finally {
      lock.unlock();
    }
  }

  public DistrList get(int id) {
    try {
      lock.lock();
      clearNullReferences();
      DistrListReference ref = hash.get(id);
      if (ref != null) {
        DistrList list = ref.get();
        if (log.isDebugEnabled() && list == null)
          log.debug("Reference to group was fount into cache, but it is null reference: id=" + id);
        if (log.isDebugEnabled() && list != null)
          log.debug("Group was found in cache: id=" + id);
      } else {
        if (log.isDebugEnabled())
          log.debug("Group was not found in cache: id=" + id);
      }
      return null;
    } finally {
      lock.unlock();
    }
  }

  public void remove(DistrList list) {
    try {
      lock.lock();
      clearNullReferences();
      if (log.isDebugEnabled())
        log.debug("Remove group from cache: name=" + list.getName() + "; owner=" + list.getOwner());
      tree.remove(list.getOwner() + '_' + list.getName());
      hash.remove(list.getId());
    } finally {
      lock.unlock();
    }
  }

  public void removeAll(Collection<DistrList> lists) {
    try {
      lock.lock();
      clearNullReferences();
      for (DistrList list : lists ) {
        if (log.isDebugEnabled())
          log.debug("Remove group from cache: name=" + list.getName() + "; owner=" + list.getOwner());
        tree.remove(list.getOwner() + '_' + list.getName());
        hash.remove(list.getId());
      }
    } finally {
      lock.unlock();
    }
  }

  private static class DistrListReference extends WeakReference<DistrList> {

    private final String owner;
    private final String name;
    private final int id;

    public DistrListReference(DistrList referent, ReferenceQueue<? super DistrList> q) {
      super(referent, q);
      this.owner = referent.getOwner();
      this.name = referent.getName();
      this.id = referent.getId();
    }
  }

}
