package ru.sibinco.calendarsme.engine.calendar;

import ru.sibinco.calendarsme.engine.calendar.CalendarMessage;

import java.util.*;

/**
 * User: artem
 * Date: Jul 25, 2006
 */

final class CalendarMessagesList {
  private static final org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(CalendarMessagesList.class);

  private final SortedSet list;
  private int currentSize = 0;
  private final int maxSize;
  private final Date maxDate = new Date();

  private final Object fullNotifier = new Object();
  private final List newMessageNotifiers = new ArrayList();

  public CalendarMessagesList(final int maxSize) {
    this.list = new TreeSet(new CalendarMessagesComparator());
    this.maxSize = maxSize;
  }

  public void addNewMessageNotifier(final Object obj) {
    if (obj == null)
      return;

    newMessageNotifiers.add(obj);
  }

  public boolean canAdd(final CalendarMessage obj) {
    synchronized(maxDate) {
      if (obj.getSendDate().after(maxDate))
        return false;
    }
    return true;
  }

  public boolean add(final CalendarMessage obj) {
    synchronized(maxDate) {
      if (obj.getSendDate().after(maxDate))
        return false;
    }

    if (currentSize >= maxSize) {
      try {
        synchronized (fullNotifier) {
          fullNotifier.wait();
        }
      } catch (InterruptedException e) {
        Log.error(e);
      }
    }

    synchronized(list) {
      list.add(obj);
      currentSize++;
      for (Iterator iter = newMessageNotifiers.iterator(); iter.hasNext();) {
        final Object notifier = iter.next();
        synchronized(notifier) {
          notifier.notify();
        }
      }
    }
    return true;
  }

  public CalendarMessage getFirst() {
    synchronized(list) {
      return (CalendarMessage)list.first();
    }
  }

  public CalendarMessage getLast() {
    synchronized(list) {
      return (CalendarMessage)list.last();
    }
  }

  public void remove(final CalendarMessage msg) {
    synchronized(list) {
      list.remove(msg);
      currentSize--;
      synchronized (fullNotifier) {
        fullNotifier.notify();
      }
    }
  }

  public boolean isEmpty() {
    synchronized(list) {
      return list.isEmpty();
    }
  }

  public int size() {
    synchronized(list) {
      return list.size();
    }
  }

  public int getMaxSize() {
    return maxSize;
  }

  public int getCurrentSize() {
    return currentSize;
  }

  public void setCurrentSize(int currentSize) {
    this.currentSize = currentSize;
  }

  public void setMaxDate(Date maxDate) {
    synchronized(maxDate) {
      this.maxDate.setTime(maxDate.getTime());
    }
  }


  class CalendarMessagesComparator implements Comparator {
    public int compare(Object o1, Object o2) {
      final CalendarMessage msg1 = (CalendarMessage)o1;
      final CalendarMessage msg2 = (CalendarMessage)o2;

      if (msg1.getSendDate().before(msg2.getSendDate()))
        return -1;

      if (msg1.getSendDate().after(msg2.getSendDate()))
        return 1;

      return 0;
    }
  }
}
