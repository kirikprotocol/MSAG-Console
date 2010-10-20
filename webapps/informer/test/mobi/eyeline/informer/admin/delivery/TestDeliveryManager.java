package mobi.eyeline.informer.admin.delivery;

/**
 * @author Aleksandr Khalitov
 */
public class TestDeliveryManager extends DeliveryManager{

  public TestDeliveryManager() {
    super(new TestDcpConnectionFactory());
  }
}
