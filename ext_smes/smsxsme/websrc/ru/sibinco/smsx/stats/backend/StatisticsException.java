package ru.sibinco.smsx.stats.backend;

/**
 * @author Aleksandr Khalitov
 */
public class StatisticsException extends Exception{

  private Code code = Code.COMMON;

  public StatisticsException(Code code) {
    this.code = code;
  }

  public Code getCode() {
    return code;
  }

  public StatisticsException(String message) {
    super(message);
  }

  public StatisticsException(String message, Throwable cause) {
    super(message, cause);
  }

  public StatisticsException(Throwable cause) {
    super(cause);
  }

  public static class Code {

    public static final Code COMMON = new Code();
    public static final Code INTERRUPTED = new Code();

    private Code() {}

  }
}
