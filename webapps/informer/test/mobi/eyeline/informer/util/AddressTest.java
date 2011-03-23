package mobi.eyeline.informer.util;

import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.fail;

/**
 * @author Artem Snopkov
 */
public class AddressTest {

  private static void assertAddressParamsEquals(Address address, int tone, int npi, String addr) {
    assertEquals(addr, address.getAddress());
    assertEquals(tone, address.getTone());
    assertEquals(npi, address.getNpi());
  }

  @Test
  public void testCreateFromStringWithToneAndNpi() {
    assertAddressParamsEquals(new Address(".5.0.MTC"), 5, 0, "MTC");
    assertAddressParamsEquals(new Address(".1.1.913"), 1, 1, "913");
    assertAddressParamsEquals(new Address(".0.0.913"), 0, 0, "913");
  }

  @Test
  public void testCreateFromStringWithoutToneAndNpi() {
    assertAddressParamsEquals(new Address("913"), 0, 1, "913");
  }

  @Test
  public void testCreateFromStringWithPlusPrefix() {
    assertAddressParamsEquals(new Address("+79139489906"), 1, 1, "79139489906");
  }

  @Test
  public void testCreateFromStringWithPrefix7() {
    assertEquals(new Address("79139489906"), new Address("+79139489906"));
    // Длина менее 11 знаков
    assertAddressParamsEquals(new Address("713"), 0, 1, "713");
    // Длина более 11 знаков
    assertAddressParamsEquals(new Address("7131212121212121212"), 0, 1, "7131212121212121212");
  }

  @Test
  public void testCreateFromStringWithPrefix8() {
    // Длина 11 знаков
    assertEquals(new Address("89139489906"), new Address("+79139489906"));
    // Длина менее 11 знаков
    assertAddressParamsEquals(new Address("813"), 0, 1, "813");
    // Длина более 11 знаков
    assertAddressParamsEquals(new Address("8131212121212121212"), 0, 1, "8131212121212121212");
  }

  @Test
  public void testCreateFromAnotherAddress() {
    String addresses[] = new String[] {".1.1.913", ".0.0.913", ".0.1.913", ".1.0.913"};
    for (String address : addresses) {
      Address addr = new Address(address);
      assertEquals(addr, new Address(addr));
    }
  }

  @Test
  public void testCreateFromIncorrectString() {
    String[] incorrectAddresses = new String[] {"Hello", ".1.1.Hello", "", ".0.123", ".1233", ".abc.5.123", ".5.fff.123"};
    for (String incorrectAddress : incorrectAddresses) {
      try {
        new Address(incorrectAddress);
        fail("Create address for this: " + incorrectAddress);
      } catch (IllegalArgumentException expected) {}
    }
  }

  @Test(expected=NullPointerException.class)
  public void testCreateFromNullString() {
    new Address((String)null);
  }

  @Test(expected=NullPointerException.class)
  public void testCreateFromNullAddress() {
    new Address((Address)null);
  }

  @Test
  public void testEquals() {
    assertEquals(new Address("+79139495113"), new Address("79139495113"));
    assertEquals(new Address("+79139495113"), new Address("89139495113"));
    assertFalse(new Address("+7123231239139495113").equals(new Address("7123231239139495113")));
    assertFalse(new Address("+7123231239139495113").equals(new Address("8123231239139495113")));
    assertFalse(new Address("+69139495113").equals(new Address("69139495113")));
    assertFalse(new Address("+69139495113").equals(new Address("89139495113")));
    assertFalse(new Address("+79139495113").equals(new String("+79139495113")));
  }

  @Test
  public void testHashCode()  {
    Address addr1 = new Address("+79139495113");
    Address addr2 = new Address("+79139495113");

    assertEquals(addr1, addr2);
    assertEquals(addr1.hashCode(), addr2.hashCode());
  }

  @Test
  public void testGetSimpleAddress() {
    assertEquals("+79139495113", new Address("+79139495113").getSimpleAddress());
    assertEquals("79139495113", new Address(".0.1.79139495113").getSimpleAddress());
    assertEquals(".0.0.79139495113", new Address(".0.0.79139495113").getSimpleAddress());
    assertEquals(".1.0.79139495113", new Address(".1.0.79139495113").getSimpleAddress());
  }

  @Test
  public void testGetNormalizedAddress() {
    assertEquals(".1.1.79139495113", new Address(".1.1.79139495113").getNormalizedAddress());
    assertEquals(".1.1.79139495113", new Address("+79139495113").getNormalizedAddress());
    assertEquals(".1.1.79139495113", new Address("89139495113").getNormalizedAddress());
    assertEquals(".1.1.79139495113", new Address("79139495113").getNormalizedAddress());

    assertEquals(".0.1.79139495113", new Address(".0.1.79139495113").getNormalizedAddress());
    assertEquals(".0.0.79139495113", new Address(".0.0.79139495113").getNormalizedAddress());
    assertEquals(".1.0.79139495113", new Address(".1.0.79139495113").getNormalizedAddress());
  }

}
