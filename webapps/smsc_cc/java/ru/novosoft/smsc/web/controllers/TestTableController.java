package ru.novosoft.smsc.web.controllers;
import ru.novosoft.smsc.web.util.AbstractTableController;

import java.io.Serializable;
import java.util.*;

/**
 * author: alkhal
 */
public class TestTableController extends AbstractTableController<TestTableController.Item>{

  private static TreeSet<Item> byNamesAsc = new TreeSet<Item>();
  private static TreeSet<Item> byNamesDesc = new TreeSet<Item>();
  private static TreeSet<Item> byValuesAsc = new TreeSet<Item>();
  private static TreeSet<Item> byValuesDesc = new TreeSet<Item>();

  static {
    Item item;
    for(int i=100; i<130;i++) {
      byNamesAsc.add(new Item("name"+i,"value"+i){
        @Override
        public int compareTo(Object o) {
          return name.compareTo(((Item)o).name);
        }
      });
      byNamesDesc.add(new Item("name"+i,"value"+i){
        @Override
        public int compareTo(Object o) {
          return ((Item)o).name.compareTo(name);
        }
      });
      byValuesAsc.add(new Item("name"+i,"value"+i){
        @Override
        public int compareTo(Object o) {
          return value.compareTo(((Item)o).value);
        }
      });
      byValuesDesc.add(new Item("name"+i,"value"+i){
        @Override
        public int compareTo(Object o) {
          return ((Item)o).value.compareTo(value);
        }
      });
    }
  }

  private String namePrefix;

  public TestTableController() {
  }

  private int itemsSize;

  @Override
  protected List<Item> getItems(String sortField, boolean isAsc, int startRow, int size) {
    Set<Item> set;
    if(sortField != null && sortField.equals("name")) {  //sortField - id параметра в commandLink заголовка столбца
      if(isAsc) {
        set = new TreeSet<Item>(byNamesAsc);
      }else {
        set = new TreeSet<Item>(byNamesDesc);
      }
    }else {
      if(isAsc) {
        set = new TreeSet<Item>(byValuesAsc);
      }else {
        set = new TreeSet<Item>(byValuesDesc);
      }
    }

    // simple filter
    if(namePrefix != null && namePrefix.length()>0) {
      Iterator<Item> iter = set.iterator();
      while(iter.hasNext()) {
        Item i = iter.next();
        if(!i.getName().startsWith(namePrefix)) {
          iter.remove();
        }
      }
    }

    List<Item> result = new LinkedList<Item>();
    Iterator<Item> iter = set.iterator();
    int i=0;
    while(iter.hasNext()) {
      Item item = iter.next();
      if(i>=startRow && i < (startRow+size)) {
        result.add(item);
      }
      i++;
    }
    return result;
  }

  @Override
  protected int countItems() {      //Общее кол-во строк в таблице, удовлетворяющих фильтру

    int count = byNamesAsc.size();
    // simple filter
    if(namePrefix != null && namePrefix.length()>0) {
      for (Item aByNamesAsc : byNamesAsc) {
        if (!aByNamesAsc.getName().startsWith(namePrefix)) {
          count--;
        }
      }
    }
    
    return count;
  }

  public String getNamePrefix() {
    return namePrefix;
  }

  public void setNamePrefix(String namePrefix) {
    this.namePrefix = namePrefix;
  }

  public static class Item implements Serializable, Comparable {

    protected String name;

    protected String value;

    public Item() {
    }

    public Item(String name, String value) {
      this.name = name;
      this.value = value;
    }

    public String getName() {
      return name;
    }

    public void setName(String name) {
      this.name = name;
    }

    public String getValue() {
      return value;
    }

    public void setValue(String value) {
      this.value = value;
    }

    public int compareTo(Object o) {
      return 0;
    }
  }


}
