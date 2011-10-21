package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.informer.util.Functions;

import java.text.ParseException;
import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 21.10.11
 * Time: 12:10
 */
public class SubmitSMData extends Data {

    private Status status;

    public long getMessageId() {
        return message_id;
    }

    public void setMessageId(long message_id) {
        this.message_id = message_id;
    }

    public void setConnectionName(String connection_name) {
        this.connection_name = connection_name;
    }

    public String getConnectionName(){
        return connection_name;
    }

    public Date getSubmitDate() {
        return submit_date;
    }

    public void setSubmitDate(Date submit_date) {
        this.submit_date = submit_date;
    }

    public void setStatus(Status status) {
        this.status = status;
    }

    public Status getStatus(){
        return status;
    }

    public static enum Status {

        SEND_RESPONSE,
        NOT_SEND_RESPONSE,
        RECEIVE_DELIVERY_RECEIPT

    }

    public static String format(SubmitSMData data){
        Date date = Functions.convertTime(data.getSubmitDate(), LOCAL_TIMEZONE, STAT_TIMEZONE);
        return data.getMessageId() + sep + data.connection_name + sep + sdf.format(data.getSubmitDate()) + sep + data.getStatus();
    }

    public static SubmitSMData parse(String s) throws ParseException {
        SubmitSMData data = new SubmitSMData();
        String[] ar = s.split(sep);

        long message_id = Long.parseLong(ar[0]);
        data.setMessageId(message_id);

        String connection_name = ar[1];
        data.setConnectionName(connection_name);

        Date submit_date = sdf.parse(ar[2]);
        submit_date = Functions.convertTime(submit_date, STAT_TIMEZONE, LOCAL_TIMEZONE);
        data.setSubmitDate(submit_date);

        Status status  =  Status.valueOf(ar[3]);
        data.setStatus(status);

        return data;
    }


}
