package mobi.eyeline.smsquiz.quizmanager;

public class Result {

  public static enum ReplyRull {
    OK, REPEAT, DOESNT_EXIST
  }

  private String text;
  private ReplyRull replyRull;
  private String sourceAddress;

  public Result(String text, ReplyRull replyRull, String sourceAddress) {
    this.text = text;
    this.replyRull = replyRull;
    this.sourceAddress = sourceAddress;
  }

  public String getText() {
    return text;
  }

  public ReplyRull getReplyRull() {
    return replyRull;
  }

  public String getSourceAddress() {
    return sourceAddress;
  }

}
 
