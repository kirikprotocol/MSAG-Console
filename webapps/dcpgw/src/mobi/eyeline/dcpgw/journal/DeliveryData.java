package mobi.eyeline.dcpgw.journal;

import mobi.eyeline.dcpgw.smpp.FinalMessageState;
import mobi.eyeline.informer.util.Functions;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;

import java.text.ParseException;
import java.util.Calendar;
import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 10.07.11
 * Time: 19:42
 */
public class DeliveryData extends Data implements Cloneable{

    private Long init_time;

    // Time when the delivery recepit was the last time resend.
    private Long last_send_time;

    private Address source_address, destination_address;

    private Integer sequence_number;

    private Date done_date;

    private FinalMessageState state;

    private int nsms;

    private Status status;

    private static Calendar cal = Calendar.getInstance();

    public void setLastSendTime(Long last_resending_time){
        this.last_send_time = last_resending_time;
    }

    public Long getLastSendTime(){
        return last_send_time;
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


    public void setSequenceNumber(Integer sequence_number){
        this.sequence_number = sequence_number;
    }

    public Integer getSequenceNumber(){
        return sequence_number;
    }

    public String toString(){

        cal.setTimeInMillis(init_time);
        Date init_date = cal.getTime();

        Date last_resending_date = null;
        if (last_send_time != null){
            cal.setTimeInMillis(last_send_time);
            last_resending_date = cal.getTime();
        }

        return "DeliveryData{message_id: " + message_id +
                ", init date: " + sdf.format(init_date) +
                ", last send date: " + (last_resending_date != null ? sdf.format(last_resending_date) : "N/A") +
                ", sa: " + source_address.getAddress() +
                ", da: " + destination_address.getAddress() +
                ", con: " + connection_name +
                ", submit date: " + sdf.format(submit_date)+
                ", done date: " + sdf.format(done_date)+
                ", nsms: " + nsms +
                ", sn: " + (sequence_number != null ? sequence_number : "N/A") +
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

    public static DeliveryData parse(String line) throws ParseException, InvalidAddressFormatException {
        DeliveryData data = new DeliveryData();
        String[] ar = line.split(sep);

        Date date = sdf.parse(ar[0]);
        date = Functions.convertTime(date, STAT_TIMEZONE, LOCAL_TIMEZONE);
        long init_time = date.getTime();

        Long last_resending_time = null;
        if (!ar[1].equals("N/A")){
            date = sdf.parse(ar[1]);
            date = Functions.convertTime(date, STAT_TIMEZONE, LOCAL_TIMEZONE);
            last_resending_time = date.getTime();
        }

        long message_id = Long.parseLong(ar[2]);

        Integer sequence_number = null;
        if (!ar[3].equals("N/A")){
            sequence_number = Integer.parseInt(ar[3]);
        }

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
        data.setInitTime(init_time);
        data.setLastSendTime(last_resending_time);
        data.setSubmitDate(submit_date);
        data.setDoneDate(done_date);
        data.setNsms(nsms);
        data.setFinalMessageState(finalMessageState);
        data.setConnectionName(connection_name);
        data.setSequenceNumber(sequence_number);
        data.setStatus(status);
        return data;
    }

    public static String format(DeliveryData data){

        Date init_date = Functions.convertTime(new Date(data.getInitTime()), LOCAL_TIMEZONE, STAT_TIMEZONE);

        Date last_resending_date = null;
        Long last_resending_time = data.getLastSendTime();
        if (last_resending_time != null){
            last_resending_date =  Functions.convertTime(new Date(data.getLastSendTime()), LOCAL_TIMEZONE, STAT_TIMEZONE);
        }

        return sdf.format(init_date) + sep +
               (last_resending_date != null ? sdf.format(last_resending_date): "N/A") + sep +
               data.getMessageId() + sep +
               (data.getSequenceNumber() != null ? data.getSequenceNumber() : "N/A") + sep +
               data.getSourceAddress().getAddress() + sep +
               data.getDestinationAddress().getAddress()+ sep +
               data.getConnectionName() + sep +
               sdf.format(data.getSubmitDate()) + sep +
               sdf.format(data.getDoneDate()) + sep +
               data.getFinalMessageState() + sep +
               data.getNsms() + sep +
               data.getStatus();
    }

    public boolean equals(DeliveryData data){

        boolean b2;
        if (last_send_time != null){
            b2 = data.getLastSendTime() != null && last_send_time.equals(data.getLastSendTime());
        } else {
            b2 = data.getLastSendTime() == null;
        }

        boolean b3;
        if (sequence_number != null){
            b3 = data.getSequenceNumber() != null && sequence_number.equals(data.getSequenceNumber());
        } else {
            b3 = data.getSequenceNumber() == null;
        }

        return (message_id == data.getMessageId() &&
                connection_name.equals(data.getConnectionName()) &&
                source_address.equals(data.getSourceAddress()) &&
                destination_address.equals(data.getDestinationAddress()) &&
                nsms == data.getNsms() &&
                submit_date.getTime() == data.getSubmitDate().getTime() &&
                done_date.getTime() == data.getDoneDate().getTime() &&
                init_time.equals(data.getInitTime()) &&
                b2  &&
                b3  &&
                status == data.getStatus() &&
                state == data.getFinalMessageState()
        );
    }

    public DeliveryData clone() throws CloneNotSupportedException {
        DeliveryData d = (DeliveryData) super.clone();
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
        d.setLastSendTime(last_send_time);
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
        INIT,
        DELETED,
        PERM_ERROR,
        TEMP_ERROR,
        RESEND

    }
}
