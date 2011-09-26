package mobi.eyeline.dcpgw.dcp;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryState;
import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.Message;

import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 25.09.11
 * Time: 18:20
 */
public class PseudoDcpConnectionImpl implements DcpConnection{
    @Override
    public long[] addDeliveryMessages(int deliveryId, List<Message> messages) throws AdminException {
        return new long[0];
    }

    @Override
    public void changeDeliveryState(int deliveryId, DeliveryState state) throws AdminException {

    }

    @Override
    public DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException {
        DeliveryStatistics deliveryStatistics = new DeliveryStatistics();
        DeliveryState deliveryState = new DeliveryState();
        deliveryStatistics.setDeliveryState(deliveryState);
        deliveryState.setStatus(DeliveryStatus.Finished);
        return deliveryStatistics;
    }

    @Override
    public void close() {

    }
}
