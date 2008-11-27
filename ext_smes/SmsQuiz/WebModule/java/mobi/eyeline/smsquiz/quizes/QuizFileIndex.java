package mobi.eyeline.smsquiz.quizes;

import java.io.File;

/**
 * author: alkhal
 * Date: 27.11.2008
 */
public class QuizFileIndex {

  private QuizFileIndex() {}

  private static int index = 0;

  public static int getUniqIndex(String quizDir) {
    do{
      index++;
    }
    while(new File(quizDir+File.separator+index+".xml").exists());
    
    return index;
  }

}
