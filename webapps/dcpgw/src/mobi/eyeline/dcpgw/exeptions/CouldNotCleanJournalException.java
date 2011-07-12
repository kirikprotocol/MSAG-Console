package mobi.eyeline.dcpgw.exeptions;

/**
 * Created by IntelliJ IDEA.
 * User: stepanov
 * Date: 11.07.11
 * Time: 15:32
 * To change this template use File | Settings | File Templates.
 */
public class CouldNotCleanJournalException extends Exception{

     public CouldNotCleanJournalException(String message){
         super(message);
     }

     public CouldNotCleanJournalException(String message, Throwable cause){
         super(message, cause);
     }

     public CouldNotCleanJournalException(Throwable cause){
         super(cause);
     }
}
