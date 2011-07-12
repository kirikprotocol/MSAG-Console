package mobi.eyeline.dcpgw.journal;

import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Set;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 10.07.11
 * Time: 19:42
 */
public class DeliverSMData {

    private long message_id;

    private long time;

    public DeliverSMData(long message_id, long time){
        this.message_id = message_id;
        this.time = time;
    }

    public void setMessageId(long message_id){
        this.message_id = message_id;
    }

    public long getMessageId(){
        return this.getMessageId();
    }

    public void setTime(long time){
        this.time = time;
    }

    public long getTime(){
        return time;
    }

    public String toString(){
        Calendar cal = Calendar.getInstance();
        cal.setTimeInMillis(time);
        Date date = cal.getTime();
        DateFormat df = DateFormat.getDateTimeInstance();
        return "message_id="+message_id+", date="+df.format(date);
    }

}
