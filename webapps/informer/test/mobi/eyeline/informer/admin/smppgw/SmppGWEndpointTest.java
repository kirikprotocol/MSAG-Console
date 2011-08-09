package mobi.eyeline.informer.admin.smppgw;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotSame;

/**
 * author: Aleksandr Khalitov
 */
public class SmppGWEndpointTest {


  private static SmppGWEndpoint createEndpoint() {
    SmppGWEndpoint e = new SmppGWEndpoint();
    e.setName("enpoint1");
    e.setSystemId("systemid");
    e.setPassword("password");
    return e;
  }

  @Test
  public void testOk() throws AdminException{
    createEndpoint().validate();
  }

  @Test(expected = AdminException.class)
  public void testNullName() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setName(null);
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testEmptyName() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setName("");
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testNullSystemId() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setSystemId(null);
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testEmptySystemId() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setSystemId("");
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testNullPassword() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setPassword(null);
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testEmptyPassword() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setPassword("");
    e.validate();
  }


  @Test(expected = AdminException.class)
  public void testShortSystemId1() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setSystemId("a");
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testShortSystemId2() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setSystemId("aa");
    e.validate();
  }

  @Test
  public void testShortSystemId3() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setSystemId("aaa");
    e.validate();
  }

  @Test
  public void testLongSystemId15() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setSystemId("aabbccddeeffggh");
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testLongSystemId16() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setSystemId("aabbccddeeffgghh");
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testLatinSystemIdRus() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setSystemId("системИд");
    e.validate();
  }

  @Test
  public void testLatinSystemIdDigits() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setSystemId("syst2emId1");
    e.validate();
  }


  @Test(expected = AdminException.class)
  public void testLatinSystemId() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setSystemId("sys!temId;");
    e.validate();
  }


  @Test(expected = AdminException.class)
  public void testShortPassword1() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setPassword("a");
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testShortPassword2() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setPassword("aa");
    e.validate();
  }

  @Test
  public void testShortPassword3() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setPassword("aaa");
    e.validate();
  }

  @Test
  public void testLongPassword8() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setPassword("aabbccdd");
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testLongPassword9() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setPassword("aabbccdde");
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testLatinPasswordRus() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setPassword("системИд");
    e.validate();
  }

  @Test(expected = AdminException.class)
  public void testLatinPasswordDigits() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setPassword("password1");
    e.validate();
  }


  @Test(expected = AdminException.class)
  public void testLatinPassword() throws AdminException {
    SmppGWEndpoint e = createEndpoint();
    e.setPassword("sys!temId;");
    e.validate();
  }

  @Test
  public void testEquals() {
    SmppGWEndpoint e1 = createEndpoint();
    SmppGWEndpoint e2 = createEndpoint();
    assertEquals(e1, e2);
  }

  @Test
  public void testNotEquals() {
    SmppGWEndpoint e1 = createEndpoint();
    SmppGWEndpoint e2 = createEndpoint();
    e2.setSystemId(e1.getSystemId()+'s');
    assertNotSame(e1, e2);
  }

  @Test
  public void testCopy() {
    SmppGWEndpoint e1 = createEndpoint();
    assertEquals(e1, new SmppGWEndpoint(e1));
  }

}
