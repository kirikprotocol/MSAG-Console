package mobi.eyeline.smsquiz.quizes;

/**
 * author: alkhal
 * Date: 10.11.2008
 */


public class AnswerCategory {

  public AnswerCategory(String pattern, String name, String answer) {
    this.name = name;
    this.pattern = pattern;
    this.answer = answer;
  }

  private String pattern;

  private String name;

  private String answer;


  public String getPattern() {
    return pattern;
  }

  public String getName() {
    return name;
  }

  public String getAnswer() {
    return answer;
  }

  public void setAnswer(String answer) {
    this.answer = answer;
  }

  public String toString() {
    StringBuffer buffer = new StringBuffer();
    buffer.append("name=").append(name).append(",").append("pattern=").append(pattern)
        .append(",").append("answer=").append(answer);
    return buffer.toString();
  }
}

