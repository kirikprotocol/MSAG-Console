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
  private long time;
  private long startTime;
  private long minAvgCountTime;

  public AvgCounter(long minAvgCountTime) {
    this.minAvgCountTime = minAvgCountTime;
    reset();
  }

  public synchronized void add(long time){
    this.time = this.time + time;
    count++;
  }

  public synchronized long getAvg(){
    if(System.currentTimeMillis() - startTime < minAvgCountTime){
      return -1;
    }
    if(count==0){
      return 0;
    }
    long res = time / count;
    time = 0;
    count = 0;
    return res;
  }

  public synchronized void reset(){
    time = 0;
    count = 0;
    startTime=System.currentTimeMillis();
  }

}
