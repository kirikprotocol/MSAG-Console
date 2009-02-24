package mobi.eyeline.smsquiz.quizes;

import java.util.List;
import java.util.LinkedList;

/**
 * author: alkhal
 * Date: 22.01.2009
 */
public class QuizState {

  private String name;

  public static final QuizState NEW = new QuizState("NEW");
  public static final QuizState GENERATION = new QuizState("GENERATION");
  public static final QuizState ACTIVE = new QuizState("ACTIVE");
  public static final QuizState FINISHED = new QuizState("FINISHED");
  public static final QuizState FINISHED_ERROR = new QuizState("FINISHED_ERROR");
  public static final QuizState EXPORTING = new QuizState("EXPORTING");
  public static final QuizState UNKNOWN = new QuizState("UNKNOWN");

  private QuizState(String name) {
    this.name = name;
  }

  public String getName() {
    return name;
  }

  public static QuizState getQuizStateByName(String name) {
    if(name.equalsIgnoreCase(NEW.getName())) {
      return NEW;
    }
    if(name.equalsIgnoreCase(GENERATION.getName())) {
      return GENERATION;
    }
    if(name.equalsIgnoreCase(ACTIVE.getName())) {
      return ACTIVE;
    }
    if(name.equalsIgnoreCase(FINISHED_ERROR.getName())) {
      return FINISHED_ERROR;
    }
    if(name.equalsIgnoreCase(FINISHED.getName())) {
      return FINISHED;
    }
    if(name.equalsIgnoreCase(EXPORTING.getName())) {
      return FINISHED;
    }
    if(name.equalsIgnoreCase(UNKNOWN.getName())) {
      return UNKNOWN;
    }
    return null;
  }
  public static List getQuizStateStringList() {
    LinkedList list = new LinkedList();
    list.add(NEW.getName());
    list.add(GENERATION.getName());
    list.add(ACTIVE.getName());
    list.add(FINISHED.getName());
    list.add(FINISHED_ERROR.getName());
    list.add(EXPORTING.getName());
    return list;
  }
}
