package mobi.eyeline.dcpgw.exeptions;

/**
 * Created by IntelliJ IDEA.
 * User: stepanov
 * Date: 11.07.11
 * Time: 16:24
 * To change this template use File | Settings | File Templates.
 */
public class CouldNotLoadJournalException extends Exception{

     public CouldNotLoadJournalException(String message){
         super(message);
     }

     public CouldNotLoadJournalException(String message, Throwable cause){
         super(message, cause);
     }

     public CouldNotLoadJournalException(Throwable cause){
         super(cause);
     }
}
