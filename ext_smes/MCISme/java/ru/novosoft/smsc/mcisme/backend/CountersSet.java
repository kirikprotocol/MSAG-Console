package ru.novosoft.smsc.mcisme.backend;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 17:59:37
 * To change this template use Options | File Templates.
 */
public class CountersSet
{
  public long missed = 0;
  public long delivered = 0;
  public long failed = 0;
  public long notified = 0;

  public CountersSet() {}
  public CountersSet(CountersSet set)
  {
    missed = set.missed;
    delivered = set.delivered;
    failed = set.failed;
    notified = set.notified;
  }
  public CountersSet(long m, long d, long f, long n)
  {
    missed = m; delivered = d;
    failed = f; notified = n;
  }

  public void increment(CountersSet set)
  {
    missed += set.missed;
    delivered += set.delivered;
    failed += set.failed;
    notified += set.notified;
  }

}
