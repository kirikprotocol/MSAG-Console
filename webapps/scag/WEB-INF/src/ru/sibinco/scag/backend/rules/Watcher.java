package ru.sibinco.scag.backend.rules;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 19.12.2006
 * Time: 16:00:23
 * To change this template use File | Settings | File Templates.
 */
public class Watcher extends Thread {
private RuleManager ruleManager;
private long timeout;
private HashMap ruleIdMap = new HashMap();
private boolean isStopped = false;

public Watcher(RuleManager ruleManager, long timeout) {
  this.timeout = timeout;
  this.ruleManager = ruleManager;
  this.start();
}

public synchronized void run() {
  while (!isStopped) {
    //System.out.println("!!!!!!!!!!!!!Before wait!!!!!!!!!!!");
    try {
      wait(timeout);
    } catch (InterruptedException ie) {
      ie.printStackTrace();
    }
    //System.out.println("!!!!!!!!!!!!!After wait!!!!!!!!!!!");
    long curTime = System.currentTimeMillis();
    synchronized(ruleIdMap) {
        for (Iterator i = ruleIdMap.entrySet().iterator();i.hasNext();) {
          Map.Entry entry = (Map.Entry)i.next();
          long time = ((Long)entry.getValue()).longValue();
          //System.out.println("ruleId = "+(String)entry.getKey()+" time = "+ new java.util.Date(time)+ " curTime = "+new java.util.Date(curTime)+" dif = "+(curTime - time));
          if (curTime - time > timeout) {
            ruleManager.unlockRule((String)entry.getKey());
            i.remove();
          }
        }
    }
  }
}

public void put(String ruleId, Long time) {
  synchronized(ruleIdMap) {
    ruleIdMap.put(ruleId, time);
  }
}

public void stopW() {
  isStopped = true;
}
}

