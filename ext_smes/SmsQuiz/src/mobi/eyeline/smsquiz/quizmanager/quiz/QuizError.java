package mobi.eyeline.smsquiz.quizmanager.quiz;

/**
 * author: alkhal
 */
public class QuizError {


  public static final QuizError UNKNOWN = new QuizError("0");

  public static final QuizError INTERNAL_ERROR = new QuizError("1");

  public static final QuizError CREATE_ERROR = new QuizError("2");

  public static final QuizError QUIZES_CONFLICT = new QuizError("3");

  public static final QuizError DISTR_ERROR = new QuizError("4");


  private final String code;

  private QuizError(String code) {
    this.code = code;
  }

  public String getCode() {
    return code;
  }
}
