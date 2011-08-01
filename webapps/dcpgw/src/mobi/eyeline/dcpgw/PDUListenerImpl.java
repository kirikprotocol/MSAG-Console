package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.journal.Status;
import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.pdu.DeliverSMResp;
import mobi.eyeline.smpp.api.pdu.Message;
import mobi.eyeline.smpp.api.pdu.PDU;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.types.EsmMessageType;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.apache.log4j.Logger;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Hashtable;
import java.util.concurrent.atomic.AtomicInteger;
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

    private AtomicInteger ai = new AtomicInteger(0);

    // Table used to map delivery identifier and user..
    private Hashtable<Integer, String> delivery_id_user_map;

    // Table used to map service number and delivery identifier.
    private Hashtable<String, Integer> service_number_delivery_id_map;

    private Calendar cal;

    private SimpleDateFormat sdf = new SimpleDateFormat("ddHHmmss");

    public PDUListenerImpl(){
        cal = Calendar.getInstance();
    }

    public void update(Hashtable<Integer, String> delivery_id_user_map, Hashtable<String, Integer> service_number_delivery_id_map){
        this.delivery_id_user_map = delivery_id_user_map;
        this.service_number_delivery_id_map = service_number_delivery_id_map;
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

                log.debug("Handle pdu with type '"+pdu.getType()+"', sequence_number '"+sequence_number+"', set id '"+message_id+"'.");

                Address source_address = request.getSourceAddress();
                String source_address_str = source_address.getAddress();

                Address smpp_destination_address = request.getDestinationAddress();
                String destination_address_str = smpp_destination_address.getAddress();

                String text = request.getMessage();
                log.debug("Message: "+text);
                log.debug("Id '"+message_id+"', source address '"+source_address_str+"', destination address '"+destination_address_str+"', text '"+text+"'.");

                int delivery_id = service_number_delivery_id_map.get(source_address_str);

                String login = delivery_id_user_map.get(delivery_id);
                log.debug("Id '"+message_id+"', service_number '"+source_address_str+"', delivery_id '"+delivery_id+"', user '"+login+"'.");

                Manager.getInstance().getSender(login).addMessage(message_id, destination_address_str, text, sequence_number, connection_name, delivery_id);

                if (request.getRegDeliveryReceipt() != RegDeliveryReceipt.None ) {

                    Message rcpt = request.getAnswer();
                    rcpt.setEsmMessageType(EsmMessageType.DeliveryReceipt);

                    Date date = cal.getTime();
                    int sn = Integer.parseInt(sdf.format(date))+ai.incrementAndGet();
                    rcpt.setSequenceNumber(sn);

                    Manager.getInstance().rememberReceiptMessage(message_id, rcpt);
                }

                break;
            }

            case DataSM: {



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
