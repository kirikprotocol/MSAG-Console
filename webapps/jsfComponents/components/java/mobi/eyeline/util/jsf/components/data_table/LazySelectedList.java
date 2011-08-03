package mobi.eyeline.util.jsf.components.data_table;

import mobi.eyeline.util.jsf.components.data_table.model.DataTableModel;
import mobi.eyeline.util.jsf.components.data_table.model.ModelException;
import mobi.eyeline.util.jsf.components.data_table.model.ModelWithObjectIds;

import java.util.*;

/**
* User: artem
* Date: 02.08.11
*/
class LazySelectedList implements List<String> {

  private final DataTableModel model;

  private List<String> selected;

  private final Set<String> unselected;

  LazySelectedList(DataTableModel model, Set<String> unselected) {
    this.model = model;
    this.unselected = unselected;
  }

  private void load() {
    List<String> selected = new LinkedList<String>();
    int i = 0;
    List rows;
    ModelWithObjectIds ident = (ModelWithObjectIds)model;
    try {
      while(!(rows =  model.getRows(10000*i, 10000, null)).isEmpty()) {
        for (Object o : rows) {
          String id = ident.getId(o);
          if(unselected == null || !unselected.contains(id)) {
            selected.add(id);
          }
        }
        if(rows.size()<10000) {
          break;
        }
        i++;
      }
    } catch (ModelException e) {
      throw new RuntimeException(e);
    }
    this.selected = selected;
  }

  public int size() {
    if(selected == null) {
      load();
    }
    return selected.size();
  }

  public boolean isEmpty() {
    if(selected == null) {
      load();
    }
    return selected.isEmpty();
  }

  public boolean contains(Object o) {
    if(selected == null) {
      load();
    }
    return selected.contains(o);
  }

  public Iterator<String> iterator() {
    if(selected == null) {
      load();
    }
    return selected.iterator();
  }

  public Object[] toArray() {
    if(selected == null) {
      load();
    }
    return selected.toArray();
  }

  public <T> T[] toArray(T[] a) {
    if(selected == null) {
      load();
    }
    return selected.toArray(a);
  }

  public boolean add(String s) {
    if(selected == null) {
      load();
    }
    return selected.add(s);
  }

  public boolean remove(Object o) {
    if(selected == null) {
      load();
    }
    return selected.remove(o);
  }

  public boolean containsAll(Collection<?> c) {
    if(selected == null) {
      load();
    }
    return selected.containsAll(c);
  }

  public boolean addAll(Collection<? extends String> c) {
    if(selected == null) {
      load();
    }
    return selected.addAll(c);
  }

  public boolean addAll(int index, Collection<? extends String> c) {
    if(selected == null) {
      load();
    }
    return selected.addAll(index, c);
  }

  public boolean removeAll(Collection<?> c) {
    if(selected == null) {
      load();
    }
    return selected.removeAll(c);
  }

  public boolean retainAll(Collection<?> c) {
    if(selected == null) {
      load();
    }
    return selected.retainAll(c);
  }

  public void clear() {
    if(selected == null) {
      load();
    }
    selected.clear();
  }

  @Override
  public boolean equals(Object o) {
    if(selected == null) {
      load();
    }
    return selected.equals(o);
  }

  @Override
  public int hashCode() {
    if(selected == null) {
      load();
    }
    return selected.hashCode();
  }

  public String get(int index) {
    if(selected == null) {
      load();
    }
    return selected.get(index);
  }

  public String set(int index, String element) {
    if(selected == null) {
      load();
    }
    return selected.set(index, element);
  }

  public void add(int index, String element) {
    if(selected == null) {
      load();
    }
    selected.add(index, element);
  }

  public String remove(int index) {
    if(selected == null) {
      load();
    }
    return selected.remove(index);
  }

  public int indexOf(Object o) {
    if(selected == null) {
      load();
    }
    return selected.indexOf(o);
  }

  public int lastIndexOf(Object o) {
    if(selected == null) {
      load();
    }
    return selected.lastIndexOf(o);
  }

  public ListIterator<String> listIterator() {
    if(selected == null) {
      load();
    }
    return selected.listIterator();
  }

  public ListIterator<String> listIterator(int index) {
    if(selected == null) {
      load();
    }
    return selected.listIterator(index);
  }

  public List<String> subList(int fromIndex, int toIndex) {
    if(selected == null) {
      load();
    }
    return selected.subList(fromIndex, toIndex);
  }

  @Override
  public String toString() {
    if(selected == null) {
      load();
    }
    return selected.toString();
  }
}
