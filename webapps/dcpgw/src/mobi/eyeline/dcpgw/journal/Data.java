package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.dcpgw.smpp.FinalMessageState;
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

    private Long init_time;

    // Time when the delivery receipt has been sent for the first time.
    private Long first_sending_time;

    // Time when the delivery recepit was the last time resend.
    private Long last_resending_time;

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

    public Long getFirstSendingTime(){
        return first_sending_time;
    }

    public void setLastResendTime(long last_resending_time){
        this.last_resending_time = last_resending_time;
    }

    public Long getLastResendTime(){
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

        cal.setTimeInMillis(init_time);
        Date init_date = cal.getTime();

        Date first_sending_date = null;
        if (first_sending_time != null){
            cal.setTimeInMillis(first_sending_time);
            first_sending_date = cal.getTime();
        }

        Date last_resending_date = null;
        if (last_resending_time != null){
            cal.setTimeInMillis(last_resending_time);
            last_resending_date = cal.getTime();
        }

        return "Data{message_id: " + message_id +
                ", init date: " + sdf.format(init_date) +
                ", first sending date: " + (first_sending_date != null ? sdf.format(first_sending_date) : "N/A") +
                ", last resending date: " + (last_resending_date != null ? sdf.format(last_resending_date) : "N/A") +
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
        long init_time = date.getTime();

        date = sdf.parse(ar[1]);
        long first_sending_time = date.getTime();

        date = sdf.parse(ar[2]);
        long last_resending_time = date.getTime();

        long message_id = Long.parseLong(ar[3]);
        int sequence_number = Integer.parseInt(ar[4]);

        Address source_address = new Address(ar[5]);
        Address destination_address = new Address(ar[6]);

        String connection_name = ar[7];
        Date submit_date = sdf.parse(ar[8]);
        Date done_date = sdf.parse(ar[9]);

        FinalMessageState finalMessageState =  FinalMessageState.valueOf(ar[10]);

        int nsms = Integer.parseInt(ar[11]);

        Status status  =  Status.valueOf(ar[12]);

        data.setMessageId(message_id);
        data.setSourceAddress(source_address);
        data.setDestinationAddress(destination_address);
        data.setInitTime(init_time);
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
        cal.setTimeInMillis(data.getInitTime());
        Date init_date = cal.getTime();

        Date first_sending_date = null;
        Long first_sending_time = data.getFirstSendingTime();
        if (first_sending_time != null){
            cal.setTimeInMillis(data.getFirstSendingTime());
            first_sending_date = cal.getTime();
        }

        Date last_resending_date = null;
        Long last_resending_time = data.getLastResendTime();
        if (last_resending_time != null){
            cal.setTimeInMillis(last_resending_time);
            last_resending_date = cal.getTime();
        }
        return sdf.format(init_date) + sep +
               (first_sending_date  != null ? sdf.format(first_sending_date) : "N/A") + sep +
               (last_resending_date != null ? sdf.format(last_resending_date): "N/A") + sep +
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
                init_time.equals(data.getInitTime()) &&
                first_sending_time.equals(data.getFirstSendingTime()) &&
                last_resending_time.equals(data.getLastResendTime()) &&
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
        d.setInitTime(init_time);
        d.setFirstSendingTime(first_sending_time);
        d.setLastResendTime(last_resending_time);
        d.setNsms(nsms);
        return d;
    }

    public Long getInitTime() {
        return init_time;
    }

    public void setInitTime(long initTime) {
        this.init_time = initTime;
    }

    public static enum Status {

        SEND,
        DONE,
        EXPIRED_TIMEOUT,
        EXPIRED_MAX_TIMEOUT,
        NOT_SEND,
        INIT

    }
}
