package ru.novosoft.smsc.infosme.backend;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 13:17:26
 * To change this template use Options | File Templates.
 */
public class CountersSet
{
  public long generated = 0;
  public long delivered = 0;
  public long retried = 0;
  public long failed = 0;

  CountersSet()
  {
  }

  CountersSet(CountersSet set)
  {
    generated = set.generated;
    delivered = set.delivered;
    retried = set.retried;
    failed = set.failed;
  }

  CountersSet(long g, long d, long r, long f)
  {
    generated = g;
    delivered = d;
    retried = r;
    failed = f;
  }

  void increment(CountersSet set)
  {
    generated += set.generated;
    delivered += set.delivered;
    retried += set.retried;
    failed += set.failed;
  }

}
