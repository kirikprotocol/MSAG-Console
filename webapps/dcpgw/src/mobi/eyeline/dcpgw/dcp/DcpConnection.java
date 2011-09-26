package mobi.eyeline.dcpgw.dcp;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryState;
import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
import mobi.eyeline.informer.admin.delivery.Message;

import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitrry Nikolaevich
 * Date: 25.09.11
 * Time: 15:19
 */
public interface DcpConnection {

    public long[] addDeliveryMessages(int deliveryId, List<Message> messages) throws AdminException;

    public void changeDeliveryState(int deliveryId, DeliveryState state) throws AdminException;

    public DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException;

    public void close();

}
