package mobi.eyeline.smsquiz.quizmanager;



public interface QuizManager {
    
    public void start();
	public void stop();
	public Result handleSms(String address, String oa, String text);
}
 
