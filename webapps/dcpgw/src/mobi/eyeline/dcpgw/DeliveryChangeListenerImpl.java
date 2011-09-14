package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.exeptions.CouldNotReadMessageStateException;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeMessageStateEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListener;
import mobi.eyeline.informer.util.Address;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.Message;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import org.apache.log4j.Logger;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Hashtable;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 14.07.11
 * Time: 15:07
 */
public class DeliveryChangeListenerImpl implements DeliveryChangeListener {

    private static Logger log = Logger.getLogger(DeliveryChangeListenerImpl.class);

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmss");

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

                Date submit_date;
                try {
                    submit_date = sdf.parse(p.getProperty("sd"));
                } catch (ParseException e1) {
                    log.error(e1);
                    throw new CouldNotReadMessageStateException("could.not.read.message.state", e1);
                }

                Date done_date = e.getEventDate();

                MessageState messageState = e.getMessageState();

                int nsms = e.getNsms();

                try {
                        Gateway.sendDeliveryReceipt(message_id, submit_date, done_date, connection_name, source_address, destination_address, nsms, messageState);
                    } catch (SmppException e1) {
                        log.error("Couldn't send delivery receipt to the client.", e1);
                        // todo ?
                    } catch (CouldNotWriteToJournalException e1) {
                        log.error(e);
                    }

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
