package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import org.junit.Test;

import static junit.framework.Assert.assertTrue;
import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */
public class MessageTest {


  @Test
  public void testCreate() {
    Message m = Message.newGlossaryMessage(1);
    assertEquals(new Integer(1), m.getIndex());
    assertEquals(MessageType.GlossaryMessage, m.getMsgType());

    m = Message.newTextMessage("t");
    assertEquals("t", m.getText());
    assertEquals(MessageType.TextMessage, m.getMsgType());
  }

  @Test
  public void testMsisdn() throws AdminException{
    Message m = new Message();
    try{
      m.setAbonent(null);
      assertTrue(false);
    }catch (AdminException e) {}
      m.setAbonent(new Address("+79139489906"));
  }

}
