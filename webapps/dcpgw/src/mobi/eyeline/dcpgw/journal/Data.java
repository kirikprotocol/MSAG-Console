package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.dcpgw.FinalMessageState;
import mobi.eyeline.smpp.api.pdu.data.Address;

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

    private Address source_address, destination_address;

    private String connection_name;

    private int sequence_number;

    private Date done_date;

    private Date submit_date;

    private FinalMessageState state;

    private int nsms;

    private Status status;

    public Data(){

    }

    public Data(long message_id, Address source_address, Address destination_address, String connection_name){
        this.message_id = message_id;
        this.source_address = source_address;
        this.destination_address = destination_address;
        this.connection_name = connection_name;
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

    public void setSourceAddress(Address source_address){
        this.source_address = source_address;
    }

    public Address getSourceAddress(){
        return source_address;
    }

    public void setDestinationAddress(Address destination_address){
        this.destination_address = destination_address;
    }

    public Address getDestinationAddress(){
        return destination_address;
    }

    public void setConnectionName(String connection_name){
        this.connection_name = connection_name;
    }

    public String getConnectionName(){
        return connection_name;
    }

    public void setSequenceNumber(int sequence_number){
        this.sequence_number = sequence_number;
    }

    public int getSequenceNumber(){
        return sequence_number;
    }

    public String toString(){

        Calendar cal = Calendar.getInstance();

        cal.setTimeInMillis(first_sending_time);
        Date first_send_date = cal.getTime();

        cal.setTimeInMillis(last_resending_time);
        Date last_resending_date = cal.getTime();

        DateFormat df = DateFormat.getDateTimeInstance();

        return "Data{message_id: "+message_id+", first sending date: "+df.format(first_send_date)+", last resending date: "+df.format(last_resending_date)
                + "sa: "+source_address.getAddress()+", da: "+destination_address.getAddress()+", connection name: "+connection_name+"}";
    }

    public Date getDoneDate() {
        return done_date;
    }

    public void setDoneDate(Date done_date) {
        this.done_date = done_date;
    }

    public Date getSubmitDate() {
        return submit_date;
    }

    public void setSubmitDate(Date submit_date) {
        this.submit_date = submit_date;
    }

    public void setFinalMessageState(FinalMessageState state){
        this.state = state;
    }

    public FinalMessageState getFinalMessageState(){
        return state;
    }

    public int getNsms() {
        return nsms;
    }

    public void setNsms(int nsms) {
        this.nsms = nsms;
    }

    public void setStatus(Status status){
        this.status = status;
    }

    public Status getStatus(){
        return status;
    }

}
