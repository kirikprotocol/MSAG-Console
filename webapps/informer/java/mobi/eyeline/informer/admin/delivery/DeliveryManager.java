package mobi.eyeline.informer.admin.delivery;

/**
 * @author Aleksandr Khalitov
 */
public class DeliveryManager {

  private final DcpConnectionFactory connectionFactory;
  
  public DeliveryManager(String host, int port) {
    this.connectionFactory = new DcpConnectionFactory(host, port);
  }

  protected DeliveryManager(DcpConnectionFactory factory) {
    this.connectionFactory = factory;
  }


  //  public CreateDeliveryResp send(CreateDelivery req) throws AdminException {
//    return client.send(req);
//  }
//
//  public CountDeliveriesResp send(CountDeliveries req) throws AdminException {
//    return client.send(req);
//  }
//
//  public FailResponse send(UserAuth req) throws AdminException {
//    return client.send(req);
//  }
//
//  public GetUserStatsResp send(GetUserStats req) throws AdminException {
//    return client.send(req);
//  }
//
//  public FailResponse send(ModifyDelivery req) throws AdminException {
//    return client.send(req);
//  }
//
//  public FailResponse send(DropDelivery req) throws AdminException {
//    return client.send(req);
//  }
//
//  public FailResponse send(ChangeDeliveryState req) throws AdminException {
//    return client.send(req);
//  }
//
//  public FailResponse send(DropDeliverymessages req) throws AdminException {
//    return client.send(req);
//  }
//
//  public GetDeliveryGlossaryResp send(GetDeliveryGlossary req) throws AdminException {
//    return client.send(req);
//  }
//
//  public FailResponse send(ModifyDeliveryGlossary req) throws AdminException {
//    return client.send(req);
//  }
//
//  public GetDeliveryStateResp send(GetDeliveryState req) throws AdminException {
//    return client.send(req);
//  }
//
//  public GetDeliveryInfoResp send(GetDeliveryInfo req) throws AdminException {
//    return client.send(req);
//  }
//
//  public GetDeliveriesListResp send(GetDeliveriesList req) throws AdminException {
//    return client.send(req);
//  }
//
//  public GetDeliveriesListNextResp send(GetDeliveriesListNext req) throws AdminException {
//    return client.send(req);
//  }
//
//  public RequestMessagesStateResp send(RequestMessagesState req) throws AdminException {
//    return client.send(req);
//  }
//
//  public GetNextMessagesPackResp send(GetNextMessagesPack req) throws AdminException {
//    return client.send(req);
//  }
//
//  public FailResponse send(CancelRequest req) throws AdminException {
//    return client.send(req);
//  }  


}
