package mobi.eyeline.informer.admin.delivery;

import junit.framework.Assert;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.util.Address;
import org.junit.Test;

import java.util.Date;

import static junit.framework.Assert.assertTrue;
import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */
public class MessageTest {


  @Test
  public void testCreate() {
    Message m = Message.newMessage(null);
    assertEquals(null, m.getText());

    m = Message.newMessage("t");
    assertEquals("t", m.getText());
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

  @Test
  public void copyTest() throws AdminException {
    Message m = Message.newMessage(new Address("+79139485453"),"dasdas" );
    m.setDate(new Date());
    m.setErrorCode(1212);
    m.setGlossaryIndex(1);
    m.setId(12121l);
    m.setProperty("prop1", "value1");
    m.setProperty("prop2", "value2");
    Assert.assertEquals(m.cloneMessage(), m.cloneMessage());
  }

}
