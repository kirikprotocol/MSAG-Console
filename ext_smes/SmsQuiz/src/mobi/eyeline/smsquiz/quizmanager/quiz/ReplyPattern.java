package mobi.eyeline.smsquiz.quizmanager.quiz;

import mobi.eyeline.smsquiz.quizmanager.quiz.Quiz;

import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class ReplyPattern {


    private Pattern pattern;
	 
	private String category;

    private String answer;

    public ReplyPattern (String p, String category, String answer) {
        this.category = category;
        this.answer = answer;
        pattern = Pattern.compile(p);
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
 
