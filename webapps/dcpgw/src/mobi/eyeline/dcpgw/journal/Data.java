package mobi.eyeline.dcpgw.journal;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 21.10.11
 * Time: 12:24
 */
abstract public class Data {

    protected static SimpleDateFormat sdf = new SimpleDateFormat("yy.MM.dd HH:mm:ss SSS");

    protected static final String sep=";";

    protected static final TimeZone STAT_TIMEZONE=TimeZone.getTimeZone("UTC");
    protected static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

    protected long message_id;

    protected Date submit_date;

    protected String connection_name;

    public void setMessageId(long message_id){
        this.message_id = message_id;
    }

    public long getMessageId(){
        return this.message_id;
    }

    public void setConnectionName(String connection_name){
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

}
