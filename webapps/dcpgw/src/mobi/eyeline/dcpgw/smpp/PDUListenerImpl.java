package mobi.eyeline.dcpgw.smpp;

import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.dcp.DcpConnectionImpl;
import mobi.eyeline.dcpgw.model.Delivery;
import mobi.eyeline.dcpgw.model.Provider;
import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.DeliverSMResp;
import mobi.eyeline.smpp.api.pdu.Message;
import mobi.eyeline.smpp.api.pdu.PDU;
import mobi.eyeline.smpp.api.pdu.SubmitSMResp;
import mobi.eyeline.smpp.api.pdu.tlv.TLVString;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 30.09.11
 * Time: 21:33
 */
public class PDUListenerImpl implements PDUListener {

    private static Logger log = Logger.getLogger(PDUListenerImpl.class);

    private AtomicLong al = new AtomicLong(0);

    private long initial_message_id;
    private long limit;
    private long rang;
    private File message_id_rang_file;



    public PDUListenerImpl(){
        Config config = Config.getInstance();
        initial_message_id = config.getInitialMessageId();
        rang = config.getRang();
        limit = initial_message_id + rang;
        message_id_rang_file = config.getMessageIdRangFile();
    }

    @Override
    public boolean handlePDU(PDU pdu) {
        log.debug("handle " + pdu.getType()+": sn="+pdu.getSequenceNumber());
        switch (pdu.getType()) {
            case SubmitSM:{

                long message_id = initial_message_id + al.incrementAndGet();
                if (message_id == limit){
                    try {
                        PrintWriter pw = new PrintWriter(new FileWriter(message_id_rang_file));
                        limit = limit + rang;
                        pw.println(limit);
                        pw.flush();
                        pw.close();
                    } catch (IOException e) {
                        log.error("Couldn't write to file new initial message id rang.", e);
                    }
                }

                Message request = (Message) pdu;

                String connection_name = request.getConnectionName();
                int sequence_number = request.getSequenceNumber();
                String source_address = request.getSourceAddress().getAddress();
                String destination_address = request.getDestinationAddress().getAddress();
                String text = request.getMessage();
                RegDeliveryReceipt register_delivery = request.getRegDeliveryReceipt();

                log.debug("receive "+pdu.getType()+": id="+message_id+", sn="+sequence_number+", con="+connection_name
                          +", src="+source_address+", dest="+destination_address+", text="+text);

                Provider provider = Config.getInstance().getProvider(connection_name);
                if (provider == null) {
                    log.error("Couldn't find provider for connection "+connection_name);
                    return false;
                }
                log.debug("This connection name corresponds to the provider with name '"+provider.getName()+"'.");

                Delivery delivery = provider.getDelivery(source_address);

                if (delivery == null){
                    log.warn("Provider '"+provider.getName()+"' doesn't have delivery with service number "+source_address+".");
                    SubmitSMResp submitSMResp = new SubmitSMResp();
                    submitSMResp.setStatus(mobi.eyeline.smpp.api.types.Status.SUBMITFAIL);
                    submitSMResp.setSequenceNumber(sequence_number);
                    submitSMResp.setConnectionName(connection_name);
                    submitSMResp.setTLV(
                            new TLVString( (short)0x001D,
                                    "Provider "+provider.getName()+" doesn't have delivery with service number '"+source_address+"'.")
                    );
                    try{
                        Server.getInstance().send(submitSMResp);
                    } catch (SmppException e) {
                        log.error("Could not send response to client", e);
                        return false;
                    }
                }

                int delivery_id = delivery.getId();
                String informer_user = delivery.getUser();

                log.debug("id "+message_id+", delivery id "+delivery_id+", informer user "+informer_user);

                DcpConnectionImpl connection = Config.getInstance().getDCPConnection(informer_user);

                mobi.eyeline.informer.admin.delivery.Message informer_message =
                    getMessage(message_id, source_address, destination_address, text, connection_name, register_delivery);

                SubmitSMResp resp = getResponse(connection_name, sequence_number);

                try {
                    connection.addMessage(delivery_id, informer_message, message_id, resp);
                } catch (InterruptedException e) {
                    log.error(e);
                    return false;
                }

                break;
            }

            case DataSM: {

                long message_id = initial_message_id + al.incrementAndGet();
                if (message_id == limit){
                    try {
                        PrintWriter pw = new PrintWriter(new FileWriter(message_id_rang_file));
                        limit = limit + rang;
                        pw.println(limit);
                        pw.flush();
                        pw.close();
                    } catch (IOException e) {
                        log.error("Couldn't write to file new initial message id rang.", e);
                    }
                }

                Message request = (Message) pdu;

                String connection_name = request.getConnectionName();
                int sequence_number = request.getSequenceNumber();
                String source_address = request.getSourceAddress().getAddress();
                String destination_address = request.getDestinationAddress().getAddress();
                String text = request.getMessage();
                RegDeliveryReceipt register_delivery = request.getRegDeliveryReceipt();

                log.debug("receive "+pdu.getType()+": id="+message_id+", sn="+sequence_number+", con="+connection_name
                          +", src="+source_address+", dest="+destination_address+", text="+text);

                Provider provider = Config.getInstance().getProvider(connection_name);
                if (provider == null) {
                    log.error("Couldn't find provider for connection "+connection_name);
                    return false;
                }
                log.debug("This connection name corresponds to the provider with name '"+provider.getName()+"'.");

                Delivery delivery = provider.getDelivery(source_address);

                if (delivery == null){
                    log.warn("Provider '"+provider.getName()+"' doesn't have delivery with service number "+source_address+".");
                    SubmitSMResp submitSMResp = new SubmitSMResp();
                    submitSMResp.setStatus(mobi.eyeline.smpp.api.types.Status.SUBMITFAIL);
                    submitSMResp.setSequenceNumber(sequence_number);
                    submitSMResp.setConnectionName(connection_name);
                    submitSMResp.setTLV(
                            new TLVString( (short)0x001D,
                                    "Provider "+provider.getName()+" doesn't have delivery with service number '"+source_address+"'.")
                    );
                    try{
                        Server.getInstance().send(submitSMResp);
                    } catch (SmppException e) {
                        log.error("Could not send response to client", e);
                        return false;
                    }
                }

                int delivery_id = delivery.getId();
                String informer_user = delivery.getUser();

                log.debug("id "+message_id+", delivery id "+delivery_id+", informer user "+informer_user);

                DcpConnectionImpl connection = Config.getInstance().getDCPConnection(informer_user);

                mobi.eyeline.informer.admin.delivery.Message informer_message =
                    getMessage(message_id, source_address, destination_address, text, connection_name, register_delivery);

                SubmitSMResp resp = getResponse(connection_name, sequence_number);

                try {
                    connection.addMessage(delivery_id, informer_message, message_id, resp);
                } catch (InterruptedException e) {
                    log.error(e);
                    return false;
                }

                break;
            }

            case DeliverSMResp:{
                DeliverSMResp resp = (DeliverSMResp) pdu;
                return Server.getInstance().handle(resp);
            }

        }

        return true;
    }


    private mobi.eyeline.informer.admin.delivery.Message getMessage(long id,
                           String source_address,
                           String destination_address,
                           String text,
                           String connection_name,
                           RegDeliveryReceipt register_delivery
                           ){
        mobi.eyeline.informer.util.Address informer_destination_address = new mobi.eyeline.informer.util.Address(destination_address);

        mobi.eyeline.informer.admin.delivery.Message informer_message = mobi.eyeline.informer.admin.delivery.Message.newMessage(informer_destination_address, text);

        informer_message.setProperty("id", Long.toString(id));
        informer_message.setProperty("sa", source_address);
        informer_message.setProperty("con", connection_name);
        informer_message.setProperty("rd", String.valueOf(register_delivery.getValue()));
        return informer_message;
    }

    private SubmitSMResp getResponse(String connection_name, int sequence_number){
        SubmitSMResp resp = new SubmitSMResp();
        resp.setConnectionName(connection_name);
        resp.setSequenceNumber(sequence_number);
        return resp;
    }

}
