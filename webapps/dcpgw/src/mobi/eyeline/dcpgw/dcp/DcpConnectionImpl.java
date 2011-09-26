package mobi.eyeline.dcpgw.dcp;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryState;
import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.admin.delivery.protogen.DcpClient;
import mobi.eyeline.informer.admin.delivery.protogen.protocol.*;

import java.util.List;

import static mobi.eyeline.informer.admin.delivery.DcpConverter.convert;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 25.09.11
 * Time: 15:22
 */
public class DcpConnectionImpl implements DcpConnection{

    private DcpClient client;
    private String user;

    public DcpConnectionImpl(String host, int port, final String user, String password) throws AdminException {
        this.user = user;
        this.client = new DcpClient(host, port, user, password);
    }

    public void close() {
        client.close();
    }

    public long[] addDeliveryMessages(int deliveryId, List<Message> messages) throws AdminException {
        AddDeliveryMessages req = new AddDeliveryMessages();
        req.setDeliveryId(deliveryId);
        req.setMessages(convert(messages));
        AddDeliveryMessagesResp resp = client.send(req);
        return resp.getMessageIds();
    }

    public void changeDeliveryState(int deliveryId, DeliveryState state) throws AdminException {
        ChangeDeliveryState req = new ChangeDeliveryState();
        req.setDeliveryId(deliveryId);
        req.setState(convert(state));
        client.send(req);
    }

    public DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException {
        GetDeliveryState req = new GetDeliveryState();
        req.setDeliveryId(deliveryId);
        GetDeliveryStateResp resp;
        resp = client.send(req);
        return convert(resp.getStats(), resp.getState());
    }

    public String toString(){
        return "DcpConnection{user="+user+"}";
    }



}
