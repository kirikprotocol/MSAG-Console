package ru.novosoft.smsc.admin.stat;

import java.util.Collection;
import java.util.TreeMap;

/**
 * author: Aleksandr Khalitov
 */
class ExtendedCountersSet extends CountersSet {

  private TreeMap<Integer, ErrorCounterSet> errors = new TreeMap<Integer, ErrorCounterSet>();

  ExtendedCountersSet() {
  }

  ExtendedCountersSet(long accepted, long rejected, long delivered,
                             long failed, long rescheduled, long temporal, long i, long o) {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
  }

  void incError(int errcode, long count) {
    ErrorCounterSet set = errors.get(errcode);
    if (set == null)
      errors.put(errcode, new ErrorCounterSet(errcode, count));
    else
      set.increment(count);
  }

  void addAllErr(Collection<ErrorCounterSet> err) {
    for (ErrorCounterSet anErr : err) {
      if (anErr != null) errors.put(anErr.getErrcode(), anErr);
    }
  }

  Collection<ErrorCounterSet> getErrors() {
    return errors.values();
  }
}

