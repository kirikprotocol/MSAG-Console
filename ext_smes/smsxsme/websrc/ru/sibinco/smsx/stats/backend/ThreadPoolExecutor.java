package ru.sibinco.smsx.stats.backend;

import org.apache.log4j.Category;

import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
class ThreadPoolExecutor {

  private static final Category logger = Category.getInstance(ThreadPoolExecutor.class);

  private final Worker[] workers;

  private final List tasks = new LinkedList();

  ThreadPoolExecutor(int size) {
    workers = new Worker[size];
    for(int i =0; i<size;i++) {
      workers[i] = new Worker();
      workers[i].start();
    }
  }

  private synchronized Runnable getTask() {
    if(tasks.isEmpty()) {
      return null;
    }
    return (Runnable)tasks.remove(0);
  }

  public synchronized void execute(Runnable runnable) {
    tasks.add(runnable);
  }

  public synchronized void shutdown() {
    tasks.clear();
    for(int i=0; i<workers.length; i++) {
      workers[i].shutdown();
    }
  }

  private class Worker extends Thread {

    private boolean started = true;

    public void run() {
      do {
        Runnable task = getTask();
        if(task != null) {
          try{
            task.run();
          }catch (Exception e){
            logger.error(e, e);
          }
        }
        try {
          Thread.sleep(2000);
        } catch (InterruptedException ignored) {}
      } while (started);
    }

    public void shutdown() {
      started = false;
      interrupt();
    }

  }
}


