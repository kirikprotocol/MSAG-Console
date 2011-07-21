package mobi.eyeline.dcpgw.journal;

import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 10.07.11
 * Time: 19:42
 */
public class Data {

    private long message_id;

    // Time when the delivery receipt has been sent for the first time.
    private long first_sending_time;

    // Time when the delivery recepit was the last time resend.
    private long last_resending_time;

    public Data(long message_id, long first_sending_time, long last_resending_time){
        this.message_id = message_id;
        this.first_sending_time = first_sending_time;
        this.last_resending_time = last_resending_time;
    }

    public void setMessageId(long message_id){
        this.message_id = message_id;
    }

    public long getMessageId(){
        return this.message_id;
    }

    public void setFirstSendingTime(long time){
        this.first_sending_time = time;
    }

    public long getFirstSendingTime(){
        return first_sending_time;
    }

    public void setLastResendTime(long last_resending_time){
        this.last_resending_time = last_resending_time;
    }

    public long getLastResendTime(){
        return last_resending_time;
    }

    public String toString(){

        Calendar cal = Calendar.getInstance();

        cal.setTimeInMillis(first_sending_time);
        Date first_send_date = cal.getTime();

        cal.setTimeInMillis(last_resending_time);
        Date last_resending_date = cal.getTime();

        DateFormat df = DateFormat.getDateTimeInstance();

        return "message_id: "+message_id+", first sending date: "+df.format(first_send_date)+", last resending date: "+df.format(last_resending_date);
    }

}
