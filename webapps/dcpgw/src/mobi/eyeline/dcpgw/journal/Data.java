package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.dcpgw.FinalMessageState;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 10.07.11
 * Time: 19:42
 */
public class Data implements Cloneable{

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

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmssSSS");

    private static Calendar cal = Calendar.getInstance();

    private static final String sep=";";

    public Data(){

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

        cal.setTimeInMillis(first_sending_time);
        Date first_send_date = cal.getTime();

        cal.setTimeInMillis(last_resending_time);
        Date last_resending_date = cal.getTime();

        return "Data{message_id: " + message_id +
                ", first sending date: " + sdf.format(first_send_date) +
                ", last resending date: " + sdf.format(last_resending_date) +
                ", sa: " + source_address.getAddress() +
                ", da: " + destination_address.getAddress() +
                ", con: " + connection_name +
                ", submit date: " + sdf.format(submit_date)+
                ", done date: " + sdf.format(done_date)+
                ", nsms: " + nsms +
                ", sn: " + sequence_number +
                ", state: " + state +
                ", status: " + status +
                "}";
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

    public static Data parse(String line) throws ParseException, InvalidAddressFormatException {
        Data data = new Data();
        String[] ar = line.split(sep);

        Date date = sdf.parse(ar[0]);
        long first_sending_time = date.getTime();

        date = sdf.parse(ar[1]);
        long last_resending_time = date.getTime();

        long message_id = Long.parseLong(ar[2]);
        int sequence_number = Integer.parseInt(ar[3]);

        Address source_address = new Address(ar[4]);
        Address destination_address = new Address(ar[5]);

        String connection_name = ar[6];
        Date submit_date = sdf.parse(ar[7]);
        Date done_date = sdf.parse(ar[8]);

        FinalMessageState finalMessageState =  FinalMessageState.valueOf(ar[9]);

        int nsms = Integer.parseInt(ar[10]);

        Status status  =  Status.valueOf(ar[11]);

        data.setMessageId(message_id);
        data.setSourceAddress(source_address);
        data.setDestinationAddress(destination_address);
        data.setFirstSendingTime(first_sending_time);
        data.setLastResendTime(last_resending_time);
        data.setSubmitDate(submit_date);
        data.setDoneDate(done_date);
        data.setNsms(nsms);
        data.setFinalMessageState(finalMessageState);
        data.setConnectionName(connection_name);
        data.setSequenceNumber(sequence_number);
        data.setStatus(status);
        return data;
    }

    public static String format(Data data){
        cal.setTimeInMillis(data.getFirstSendingTime());
        Date first_sending_date = cal.getTime();
        cal.setTimeInMillis(data.getLastResendTime());
        Date last_resending_time = cal.getTime();
        return sdf.format(first_sending_date) + sep +
               sdf.format(last_resending_time) + sep +
               data.getMessageId() + sep +
               data.getSequenceNumber() + sep +
               data.getSourceAddress().getAddress() + sep +
               data.getDestinationAddress().getAddress()+ sep +
               data.getConnectionName() + sep +
               sdf.format(data.getSubmitDate()) + sep +
               sdf.format(data.getDoneDate()) + sep +
               data.getFinalMessageState() + sep +
               data.getNsms() + sep +
               data.getStatus();
    }

    public boolean equals(Data data){
        return (message_id == data.getMessageId() &&
                connection_name.equals(data.getConnectionName()) &&
                source_address.equals(data.getSourceAddress()) &&
                destination_address.equals(data.getDestinationAddress()) &&
                nsms == data.getNsms() &&
                submit_date.getTime() == data.getSubmitDate().getTime() &&
                done_date.getTime() == data.getDoneDate().getTime() &&
                first_sending_time == data.getFirstSendingTime() &&
                last_resending_time == data.getLastResendTime() &&
                sequence_number == data.getSequenceNumber() &&
                status == data.getStatus() &&
                state == data.getFinalMessageState()
        );
    }

    public Data clone() throws CloneNotSupportedException {
        Data d = (Data) super.clone();
        d.setMessageId(message_id);
        d.setStatus(status);
        d.setSequenceNumber(sequence_number);
        d.setConnectionName(connection_name);
        d.setDestinationAddress(destination_address);
        d.setSourceAddress(source_address);
        d.setFinalMessageState(state);
        d.setSubmitDate(submit_date);
        d.setDoneDate(done_date);
        d.setFirstSendingTime(first_sending_time);
        d.setLastResendTime(last_resending_time);
        d.setNsms(nsms);
        return d;
    }

}
