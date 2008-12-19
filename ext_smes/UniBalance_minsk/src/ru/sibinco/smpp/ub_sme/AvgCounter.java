package ru.sibinco.smpp.ub_sme;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 19.12.2008
 * Time: 11:44:36
 * To change this template use File | Settings | File Templates.
 */
public class AvgCounter {

  private int count;
  private long avgTime;

  public synchronized void add(long time){
    count++;
    avgTime =((count-1)* avgTime + time)/count;
  }

  public synchronized long getAvg(){
    long res=avgTime;
    avgTime=0;
    count=0;
    return res;
  }

}
