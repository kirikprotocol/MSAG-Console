package mobi.eyeline.dcpgw.dcp;

import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.exeptions.CouldNotReadMessageStateException;
import mobi.eyeline.dcpgw.exeptions.CouldNotWriteToJournalException;
import mobi.eyeline.dcpgw.journal.DeliveryData;
import mobi.eyeline.dcpgw.journal.Journal;
import mobi.eyeline.dcpgw.journal.SubmitData;
import mobi.eyeline.dcpgw.model.Delivery;
import mobi.eyeline.dcpgw.model.Provider;
import mobi.eyeline.dcpgw.smpp.Connection;
import mobi.eyeline.dcpgw.smpp.FinalMessageState;
import mobi.eyeline.dcpgw.smpp.Server;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.MessageState;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeDeliveryStatusEvent;
import mobi.eyeline.informer.admin.delivery.changelog.ChangeMessageStateEvent;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangeListener;
import mobi.eyeline.informer.util.Functions;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
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
            if (!p.containsKey("id")){
                log.warn("Couldn't find message identifier in the final log string.");
                return;
            }

            String s = p.getProperty("id");
            long message_id = Long.parseLong(s);

            // Check does connection exist.
            String connection_name = p.getProperty("con");
            Connection connection = Server.getInstance().getConnection(connection_name);
            if (connection == null){
                if (!Server.getInstance().connectionRemoved(connection_name)) {
                    log.error("Couldn't send "+message_id+"_DeliverSM, couldn't "+connection_name+"smpp_connection ");
                } else {
                    log.debug("Couldn't send "+message_id+"_DeliverSM, "+connection_name+"_smpp_connection was deleted or disabled.");
                }
                return;
            }

            // Check whether we need to send a delivery report
            MessageState messageState = e.getMessageState();

            RegDeliveryReceipt rdr = RegDeliveryReceipt.valueOf(Integer.valueOf(p.getProperty("rd")));
            if (rdr == RegDeliveryReceipt.None ){
                log.debug(message_id+"_message has registered delivery parameter "+rdr+", remove delivery receipt");
                return;
            } else if (rdr == RegDeliveryReceipt.Success){
                if (messageState != MessageState.Delivered){
                    log.debug(message_id+"_message has registered delivery parameter "+rdr+", but message state is "+messageState+", remove delivery receipt");

                    try {
                        SubmitData sdata = connection.removeSubmitSMData(message_id);
                        sdata.setSubmitDate(new Date(System.currentTimeMillis()));
                        sdata.setStatus(SubmitData.Status.RECEIVE_DELIVERY_RECEIPT);
                        Journal.getInstance().write(sdata);
                    } catch (CouldNotWriteToJournalException e1) {
                        log.error("Couldn't write submit data to journal.", e1);
                    }

                    return;
                }
            } else if (rdr == RegDeliveryReceipt.Failure){
                if (messageState == MessageState.Delivered){
                    log.debug(message_id+"_message has registered delivery parameter "+rdr+", but message state is "+messageState+", remove delivery receipt");

                    try {
                        SubmitData sdata = connection.removeSubmitSMData(message_id);
                        sdata.setSubmitDate(new Date(System.currentTimeMillis()));
                        sdata.setStatus(SubmitData.Status.RECEIVE_DELIVERY_RECEIPT);
                        Journal.getInstance().write(sdata);
                    } catch (CouldNotWriteToJournalException e1) {
                        log.error("Couldn't write submit data to journal.", e1);
                    }

                    return;
                }
            }

            mobi.eyeline.smpp.api.pdu.data.Address source_address, destination_address;
            try {
                source_address  = new mobi.eyeline.smpp.api.pdu.data.Address(p.getProperty("sa"));
                destination_address = new mobi.eyeline.smpp.api.pdu.data.Address(e.getAddress().getAddress());
            } catch (InvalidAddressFormatException e1) {
                log.error(e);
                throw new CouldNotReadMessageStateException("could.not.read.message.state", e1);
            }

            int nsms = e.getNsms();

            FinalMessageState state;
            if (messageState == MessageState.Delivered) {
                state = FinalMessageState.DELIVRD;
            } else if (messageState == MessageState.Expired) {
                state = FinalMessageState.EXPIRED;
            } else {
                state = FinalMessageState.UNKNOWN;
            }

            DeliveryData data = new DeliveryData();
            data.setMessageId(message_id);
            data.setConnectionName(connection_name);
            data.setSourceAddress(destination_address);
            data.setDestinationAddress(source_address);

            Date done_date = Functions.convertTime(e.getEventDate(), LOCAL_TIMEZONE, STAT_TIMEZONE);
            data.setDoneDate(done_date);

            data.setFinalMessageState(state);
            data.setNsms(nsms);
            data.setStatus(DeliveryData.Status.INIT);
            data.setInitTime(System.currentTimeMillis());

            SubmitData sdata = connection.removeSubmitSMData(message_id);
            if (sdata == null){
                log.error("Couldn't find submit data for receipt with message id "+data.getMessageId());
                return;
            }

            SubmitData.Status status = sdata.getStatus();

            Date submit_date = Functions.convertTime(sdata.getSubmitDate(), LOCAL_TIMEZONE, STAT_TIMEZONE);
            data.setSubmitDate(submit_date);

            try {
                sdata.setSubmitDate(new Date(System.currentTimeMillis()));
                sdata.setStatus(SubmitData.Status.RECEIVE_DELIVERY_RECEIPT);
                Journal.getInstance().write(sdata);
            } catch (CouldNotWriteToJournalException e1) {
                log.error("Couldn't write to submit date journal.", e1);
            }

            if (status == SubmitData.Status.NOT_SEND_RESPONSE){
                log.debug("Received "+message_id+"_delivery_receipt, couldn't send SubmitSMResp for this "+message_id+"message, remove receipt");
                return;
            }

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

        }
    }

    @Override
    public void deliveryStateChanged(ChangeDeliveryStatusEvent e) throws AdminException {
        log.debug(e);
    }

}
