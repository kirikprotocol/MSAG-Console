package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;

import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class TestDeliveryManager extends DeliveryManager{

  public TestDeliveryManager() {
    super();
  }

  private TestDcpConnection connection = new TestDcpConnection();

  @Override
  protected DcpConnection createConnection(String host, int port, String login, String password) throws AdminException {
    return connection;
  }

  public void forceModifyDeliveries() throws AdminException {
    connection.modifyAll();
  }

  public void forceActivatePlannedDeliveries() throws AdminException {
    connection.forceActivate();
  }


  @Override
  public Delivery createDeliveryWithIndividualTexts(String login, String password, DeliveryPrototype delivery, DataSource<Message> msDataSource) throws AdminException {
    delivery.setStartDate(new Date(delivery.getStartDate().getTime()+50));
    return super.createDeliveryWithIndividualTexts(login, password, delivery, msDataSource);
  }

  @Override
  public Delivery createDeliveryWithSingleText(String login, String password, DeliveryPrototype delivery, DataSource<Address> dataSource) throws AdminException {
    delivery.setStartDate(new Date(delivery.getStartDate().getTime()+50));
    return super.createDeliveryWithSingleText(login, password, delivery, dataSource);
  }

  @Override
  public Delivery createDeliveryWithSingleTextWithData(String login, String password, DeliveryPrototype delivery, DataSource<Message> dataSource) throws AdminException {
    delivery.setStartDate(new Date(delivery.getStartDate().getTime()+50));
    return super.createDeliveryWithSingleTextWithData(login, password, delivery, dataSource);
  }
}
