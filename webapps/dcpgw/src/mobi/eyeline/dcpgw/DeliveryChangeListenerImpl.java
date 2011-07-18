package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeMessageStateEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListener;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.Message;
import org.apache.log4j.Logger;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 14.07.11
 * Time: 15:07
 */
public class DeliveryChangeListenerImpl implements DeliveryChangeListener {

    private static Logger log = Logger.getLogger(DeliveryChangeListenerImpl.class);

    private SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmm");

    @Override
    public void messageStateChanged(ChangeMessageStateEvent e) throws AdminException {
        log.debug(e);
        Properties p = e.getProperties();
        if (p!=null){
            if (p.containsKey("id")){
                String s = p.getProperty("id");
                long message_id = Long.parseLong(s);

                Message rcpt = Manager.getInstance().getReceiptMessage(message_id);

                if (rcpt != null){

                    Date submit_time = Manager.getInstance().getSubmitTime(message_id);
                    Date done_date = e.getEventDate();

                    MessageState messageState = e.getMessageState();

                    FinalMessageStates finalMessageStates;
                    if (messageState == MessageState.Delivered){
                        finalMessageStates = FinalMessageStates.DELIVERED;
                    } else if (messageState == MessageState.Expired){
                        finalMessageStates = FinalMessageStates.EXPIRED;
                    } else {
                        finalMessageStates = FinalMessageStates.UNKNOWN;
                    }

                    String receipt_message = "id:" + message_id +
                                             " sub:001 dlvrd:001 submit date:" + sdf.format(submit_time) +
                                             " done date:" + sdf.format(done_date) +
                                             " stat:"+finalMessageStates +
                                             " err:000 Text:";
                    log.debug("Receipt message: " + receipt_message);
                    rcpt.setMessage(receipt_message);

                    try {
                        Gateway.sendDeliveryReceipt(message_id, rcpt);
                        // todo remove receipt
                    } catch (SmppException e1) {
                        log.error("Couldn't send delivery receipt to the client.", e1);
                        // todo ?
                    } catch (CouldNotWriteToJournalException e1) {
                        log.error(e);
                    }
                } else {

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
