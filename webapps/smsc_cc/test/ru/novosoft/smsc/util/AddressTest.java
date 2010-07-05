package ru.novosoft.smsc.util;

import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class AddressTest {

  @Test
  public void createOkTest() throws AdminException {
    new Address("+79139495113");
    new Address(".0.1.79139495113");
    new Address("79139495113");
    new Address(".1.1.79139495113");
    new Address(".5.0.HelloWorld");
  }

  @Test
  public void createCopyTest() throws AdminException {
    Address addr = new Address("+79139491513");
    assertEquals(addr, new Address(addr));
  }

  @Test(expected = AdminException.class)
  public void createFailedTest1() throws AdminException {
    new Address("HelloWorld");
  }

  @Test(expected = AdminException.class)
  public void createFailedTest2() throws AdminException {
    new Address("");
  }

  @Test(expected = AdminException.class)
  public void createFailedTest3() throws AdminException {
    new Address(".1.1.HelloWorld");
  }

  @Test
  public void getToneTest() throws AdminException {
    assertEquals(1, new Address("+79139495113").getTone());
    assertEquals(0, new Address("79139495113").getTone());
    assertEquals(5, new Address(".5.0.HelloWorld").getTone());
  }

  @Test
  public void getNpiTest() throws AdminException {
    assertEquals(1, new Address("+79139495113").getNpi());
    assertEquals(1, new Address("79139495113").getNpi());
    assertEquals(0, new Address(".5.0.HelloWorld").getNpi());
  }

  @Test
  public void getAddressTest() throws AdminException {
    assertEquals("79139495113", new Address("+79139495113").getAddress());
    assertEquals("79139495113", new Address("79139495113").getAddress());
    assertEquals("HelloWorld", new Address(".5.0.HelloWorld").getAddress());
  }

  @Test
  public void hashCodeTest() throws AdminException {
    Address addr1 = new Address("+79139495113");
    Address addr2 = new Address("+79139495113");

    assertEquals(addr1, addr2);
    assertEquals(addr1.hashCode(), addr2.hashCode());
  }

}
