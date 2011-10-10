package mobi.eyeline.dcpgw.exeptions;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 11.07.11
 * Time: 16:24
 */
public class CouldNotLoadJournalException extends Exception{

     public CouldNotLoadJournalException(String message){
         super(message);
     }

     public CouldNotLoadJournalException(Throwable cause){
         super(cause);
     }
}
