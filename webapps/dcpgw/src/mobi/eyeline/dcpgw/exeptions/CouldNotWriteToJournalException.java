package mobi.eyeline.dcpgw.exeptions;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 11.07.11
 * Time: 14:40
 */
public class CouldNotWriteToJournalException extends Exception{

     public CouldNotWriteToJournalException(String message){
         super(message);
     }

     public CouldNotWriteToJournalException(String message, Throwable cause){
         super(message, cause);
     }

     public CouldNotWriteToJournalException(Throwable cause){
         super(cause);
     }
}
