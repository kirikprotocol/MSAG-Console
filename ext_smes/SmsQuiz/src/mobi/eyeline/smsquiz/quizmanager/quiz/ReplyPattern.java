package mobi.eyeline.smsquiz.quizmanager.quiz;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

class ReplyPattern {


  private Pattern pattern;

  private String category;

  private String answer;

  public ReplyPattern(String p, String category, String answer) {
    this.category = category;
    this.answer = answer;
    pattern = Pattern.compile(p, Pattern.CASE_INSENSITIVE);
  }

  public boolean matches(String str) {
    Matcher matcher = pattern.matcher(str);
    return matcher.matches();
  }

  public String getCategory() {
    return category;
  }

  public String getAnswer() {
    return answer;
  }
}
 
