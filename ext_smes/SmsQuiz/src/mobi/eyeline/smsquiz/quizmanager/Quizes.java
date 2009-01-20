package mobi.eyeline.smsquiz.quizmanager;


import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;
import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;
import org.apache.log4j.Logger;

import java.util.LinkedList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * autor: alkhal
 */

public class Quizes {

  private static final Logger logger = Logger.getLogger(Quizes.class);

  private final LinkedList<Quiz> qInternal;

  private final Lock lock;

  private final AbstractDynamicMBean monitor;

  public Quizes() {
    qInternal = new LinkedList<Quiz>();
    lock = new ReentrantLock();
    monitor = new QuizesMBean(this);
  }

  public Quiz getQuizByFile(String filename) {
    if (filename == null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }

    try {
      lock.lock();

      for (Quiz q : qInternal) {
        if (q.getFileName().equals(filename)) {
          return q;
        }
      }
      return null;

    } finally {
      lock.unlock();
    }
  }

  public void add(Quiz quiz) {
    if (quiz == null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    try {
      lock.lock();
      qInternal.add(quiz);

    } finally {
      lock.unlock();
    }

  }

  public void remove(Quiz quiz) {
    if (quiz == null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    try {
      lock.lock();
      qInternal.remove(quiz);

    } finally {
      lock.unlock();
    }

  }

  public void visit(Visitor visitor) throws QuizException {
    if (visitor == null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    try {
      lock.lock();
      for (Quiz quiz : qInternal) {
        visitor.visit(quiz);
      }
    } finally {
      lock.unlock();
    }
  }

  public int countQuizes() {
    return qInternal.size();
  }

  public String getQuizesToString(String separator) {
    try {
      lock.lock();
      StringBuilder sb = new StringBuilder();
      for (Quiz quiz : qInternal) {
        sb.append(quiz).append(separator);
      }
      return sb.toString();
    } finally {
      lock.unlock();
    }
  }

  public interface Visitor {
    public void visit(Quiz quiz) throws QuizException;
  }

  public AbstractDynamicMBean getMonitor() {
    return monitor;
  }

}
