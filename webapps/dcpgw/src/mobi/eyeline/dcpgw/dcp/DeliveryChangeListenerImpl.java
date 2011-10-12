package mobi.eyeline.dcpgw.dcp;

import mobi.eyeline.dcpgw.exeptions.CouldNotReadMessageStateException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.smpp.FinalMessageState;
import mobi.eyeline.dcpgw.smpp.Server;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeMessageStateEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListener;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import org.apache.log4j.Logger;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 30.09.11
 * Time: 21:40
 */
public class DeliveryChangeListenerImpl implements DeliveryChangeListener {

    private static Logger log = Logger.getLogger(DeliveryChangeListenerImpl.class);

    private SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmssSSS");

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
                    data.setSubmitDate(submit_date);
                    data.setFinalMessageState(state);
                    data.setNsms(nsms);
                    data.setStatus(Data.Status.INIT);
                    data.setInitTime(System.currentTimeMillis());

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
