package mobi.eyeline.smsquiz.quizmanager;


import java.util.LinkedList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;

import org.apache.log4j.Logger;
import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;

/**
 * Created by IntelliJ IDEA.
 * User: alkhal
 * Date: 28.11.2008
 * Time: 15:03:20
 * To change this template use File | Settings | File Templates.
 */
public class Quizes {

  private static final Logger logger = Logger.getLogger(Quizes.class);

  LinkedList<Quiz> qInternal = new LinkedList<Quiz>();

  Lock lock = new ReentrantLock();


  public Quiz getQuizByFile(String filename) throws QuizException{
    if(filename==null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }

    try{
      lock.lock();

      for(Quiz q : qInternal) {
        if(q.getFileName().equals(filename)) {
          return q;
        }
      }
      return null;

    } finally{
      lock.unlock();
    }
  }

  public Quiz getQuizByDestination(String dest) throws QuizException{
    if(dest==null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    try{
      lock.lock();
      for(Quiz q : qInternal) {
        if(q.getDestAddress().equals(dest)) {
          return q;
        }
      }
      return null;

    } finally{
      lock.unlock();
    }
  }

  public void add(Quiz quiz) throws QuizException{
    if(quiz==null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    try{
      lock.lock();
      if(logger.isInfoEnabled()) {
        logger.info("Adding quiz to quizes: "+quiz);
      }
      qInternal.add(quiz);

    } finally{
      lock.unlock();
    }

  }

  public void remove(Quiz quiz) throws QuizException{
    if(quiz==null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    try{
      lock.lock();
      if(logger.isInfoEnabled()) {
        logger.info("Removing quiz from quizes: "+quiz);  
      }
      qInternal.remove(quiz);

    }finally{
      lock.unlock();
    }

  }

  public void visit(Visitor visitor) throws QuizException{
    if(visitor==null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    try{
      lock.lock();
      for (Quiz quiz : qInternal) {
        visitor.visit(quiz);
      }
    } finally{
      lock.unlock();
    }
  }

  public interface Visitor {
    public void visit(Quiz quiz) throws QuizException;
  }
}
