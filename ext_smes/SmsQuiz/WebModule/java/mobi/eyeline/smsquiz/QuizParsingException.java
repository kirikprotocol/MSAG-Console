package mobi.eyeline.smsquiz;

/**
 * author: alkhal
 * Date: 10.11.2008
 */
public class QuizParsingException extends Exception{

  public QuizParsingException(String message) {
    super(message);
  }

  public QuizParsingException(String message, Exception e) {
    super(message,e);
  }

  public QuizParsingException(Throwable e) {
    super(e);
  }

}
