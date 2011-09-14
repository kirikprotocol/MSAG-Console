package mobi.eyeline.dcpgw;

import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.Message;
import mobi.eyeline.smpp.api.pdu.PDU;
import mobi.eyeline.smpp.api.pdu.SubmitSMResp;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.pdu.tlv.TLVString;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import mobi.eyeline.smpp.api.types.Status;
import org.apache.log4j.Logger;

import java.util.Hashtable;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 31.07.11
 * Time: 0:00
 */
public class PDUListenerImpl implements PDUListener {

    private static Logger log = Logger.getLogger(PDUListenerImpl.class);

    private AtomicLong al = new AtomicLong(0);

    private Hashtable<String, Provider> connection_provider_table;

    public PDUListenerImpl(Hashtable<String, Provider> connection_provider_table){
        this.connection_provider_table = connection_provider_table;
    }

    public void update(Hashtable<String, Provider> connection_provider_table){

    }

    @Override
    public boolean handlePDU(PDU pdu) {

        switch (pdu.getType()) {

            case SubmitSM:{

                long time = System.currentTimeMillis();
                long message_id = time + al.incrementAndGet();

                Message request = (Message) pdu;
                String connection_name = request.getConnectionName();

                int sequence_number = request.getSequenceNumber();

                log.debug("Handle pdu with type '"+pdu.getType()+"', sequence_number '"+sequence_number+"', connection_name '"+connection_name+"', set id '"+message_id+"'.");

                Provider provider = connection_provider_table.get(connection_name);
                log.debug("This connection name corresponds to the provider with name '"+provider.getName()+"'.");

                Address source_address = request.getSourceAddress();
                String service_number = source_address.getAddress();

                Address smpp_destination_address = request.getDestinationAddress();
                String destination_address_str = smpp_destination_address.getAddress();

                String text = request.getMessage();
                log.debug("Message: "+text);
                log.debug("Id '"+message_id+"', service number '"+service_number+"', destination address '"+destination_address_str+"', text '"+text+"'.");

                Delivery delivery = provider.getDelivery(service_number);
                if (delivery != null){

                    int delivery_id = delivery.getId();
                    String login= delivery.getUser();

                    log.debug("Message id '"+message_id+"', service number '"+service_number+"', delivery id '"+delivery_id+"', user '"+login+"'.");

                    Manager.getInstance().getSender(login).addMessage(message_id, service_number, destination_address_str, text, sequence_number, connection_name, delivery_id);

                    if (request.getRegDeliveryReceipt() == RegDeliveryReceipt.None ) log.warn("Received SubmitSM with RegDeliveryReceipt.None .");

                } else {
                    log.debug("Provider "+provider.getName()+" doesn't have delivery with service number '"+service_number+"'.");
                    SubmitSMResp submitSMResp = new SubmitSMResp();

                    submitSMResp.setStatus(Status.SUBMITFAIL);
                    submitSMResp.setSequenceNumber(sequence_number);
                    submitSMResp.setConnectionName(connection_name);
                    submitSMResp.setTLV(new TLVString( (short)0x001D, "Provider "+provider.getName()+" doesn't have delivery with service number '"+service_number+"'.") );
                    try {
                        Gateway.sendSubmitSMResp(submitSMResp);
                        log.debug("SUBMIT_SM_RESP sn: "+sequence_number+", con: "+connection_name+", status '"+submitSMResp.getStatus());
                    } catch (SmppException e) {
                        log.error("Could not send response to client", e);
                    }
                }

                break;
            }

            case DataSM: {

                long time = System.currentTimeMillis();
                long message_id = time + al.incrementAndGet();

                Message request = (Message) pdu;
                String connection_name = request.getConnectionName();

                int sequence_number = request.getSequenceNumber();

                log.debug("Handle pdu with type '"+pdu.getType()+"', sequence_number '"+sequence_number+"', connection_name '"+connection_name+"', set id '"+message_id+"'.");

                Provider provider = connection_provider_table.get(connection_name);
                log.debug("This connection name corresponds to the provider with name '"+provider.getName()+"'.");

                Address source_address = request.getSourceAddress();
                String service_number = source_address.getAddress();

                Address smpp_destination_address = request.getDestinationAddress();
                String destination_address_str = smpp_destination_address.getAddress();

                String text = request.getMessage();
                log.debug("Message: "+text);
                log.debug("Id '"+message_id+"', service number '"+service_number+"', destination address '"+destination_address_str+"', text '"+text+"'.");

                Delivery delivery = provider.getDelivery(service_number);
                if (delivery != null){

                    int delivery_id = delivery.getId();
                    String login= delivery.getUser();

                    log.debug("Message id '"+message_id+"', service number '"+service_number+"', delivery id '"+delivery_id+"', user '"+login+"'.");

                    Manager.getInstance().getSender(login).addMessage(message_id, service_number, destination_address_str, text, sequence_number, connection_name, delivery_id);

                    if (request.getRegDeliveryReceipt() == RegDeliveryReceipt.None ) log.warn("Received DataSM with RegDeliveryReceipt.None .");

                } else {
                    log.debug("Provider "+provider.getName()+" doesn't have delivery with service number '"+service_number+"'.");
                    SubmitSMResp submitSMResp = new SubmitSMResp();

                    submitSMResp.setStatus(Status.SUBMITFAIL);
                    submitSMResp.setSequenceNumber(sequence_number);
                    submitSMResp.setConnectionName(connection_name);
                    submitSMResp.setTLV(new TLVString( (short)0x001D, "Provider "+provider.getName()+" doesn't have delivery with service number '"+service_number+"'.") );
                    try {
                        Gateway.sendSubmitSMResp(submitSMResp);
                        log.debug("SUBMIT_SM_RESP sn: "+sequence_number+", con: "+connection_name+", status '"+submitSMResp.getStatus());
                    } catch (SmppException e) {
                        log.error("Could not send response to client", e);
                    }
                }

                break;
            }

            case DeliverSMResp:{
                log.debug("Handle pdu with type "+pdu.getType());
                Gateway.handleDeliverySMResp(pdu);
                break;
            }

        }

        return true;
    }

}
