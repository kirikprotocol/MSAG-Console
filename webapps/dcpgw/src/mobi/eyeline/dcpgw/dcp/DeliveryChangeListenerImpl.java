package mobi.eyeline.dcpgw.dcp;

import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.exeptions.CouldNotReadMessageStateException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.model.Delivery;
import mobi.eyeline.dcpgw.model.Provider;
import mobi.eyeline.dcpgw.smpp.FinalMessageState;
import mobi.eyeline.dcpgw.smpp.Server;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeMessageStateEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListener;
import mobi.eyeline.informer.util.Functions;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import org.apache.log4j.Logger;

import java.util.Date;
import java.util.Properties;
import java.util.TimeZone;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 30.09.11
 * Time: 21:40
 */
public class DeliveryChangeListenerImpl implements DeliveryChangeListener {

    private static Logger log = Logger.getLogger(DeliveryChangeListenerImpl.class);

    private static final TimeZone STAT_TIMEZONE=TimeZone.getTimeZone("UTC");
    private static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

    @Override
    public void messageStateChanged(ChangeMessageStateEvent e) throws AdminException {
        log.debug(e);
        Properties p = e.getProperties();
        if (p!=null){
            if (p.containsKey("id")){
                String s = p.getProperty("id");
                long message_id = Long.parseLong(s);

                mobi.eyeline.smpp.api.pdu.data.Address source_address, destination_address;
                try {
                    source_address  = new mobi.eyeline.smpp.api.pdu.data.Address(p.getProperty("sa"));
                    destination_address = new mobi.eyeline.smpp.api.pdu.data.Address(e.getAddress().getAddress());
                } catch (InvalidAddressFormatException e1) {
                    log.error(e);
                    throw new CouldNotReadMessageStateException("could.not.read.message.state", e1);
                }

                String connection_name = p.getProperty("con");

                Date done_date = Functions.convertTime(e.getEventDate(), LOCAL_TIMEZONE, STAT_TIMEZONE);

                MessageState messageState = e.getMessageState();

                int nsms = e.getNsms();

                FinalMessageState state;
                if (messageState == MessageState.Delivered) {
                    state = FinalMessageState.DELIVRD;
                } else if (messageState == MessageState.Expired) {
                    state = FinalMessageState.EXPIRED;
                } else {
                    state = FinalMessageState.UNKNOWN;
                }

                Data data = new Data();
                data.setMessageId(message_id);
                data.setConnectionName(connection_name);
                data.setSourceAddress(destination_address);
                data.setDestinationAddress(source_address);
                data.setDoneDate(done_date);
                data.setFinalMessageState(state);
                data.setNsms(nsms);
                data.setStatus(Data.Status.INIT);
                data.setInitTime(System.currentTimeMillis());

                Provider provider = Config.getInstance().getProvider(connection_name);
                if (provider == null){
                    log.debug("Couldn't find provider for message_id "+message_id+" with connection "+connection_name+".");
                    return;
                }

                Delivery delivery = provider.getDelivery(source_address.getAddress());
                if (delivery == null){
                    log.debug("Couldn't find delivery for message_id "+message_id+" with source address "+source_address.getAddress()+".");
                    return;
                }

                Server.getInstance().send(data);

            } else {
                log.warn("Couldn't find message identifier in the final log string.");
            }
        }
    }

    @Override
    public void deliveryStateChanged(ChangeDeliveryStatusEvent e) throws AdminException {
        log.debug(e);
    }

}
