package mobi.eyeline.smsquiz.quizmanager;

public class Result {

    public static enum ReplyRull{OK, REPEAT, DOESNT_EXIST}

    private String text;
    private ReplyRull replyRull;

    public Result(String text, ReplyRull replyRull) {
        this.text = text;
        this.replyRull = replyRull;
    }

    public String getText() {
        return text;
    }

    public ReplyRull getReplyRull() {
        return replyRull;
    }



	 
}
 
