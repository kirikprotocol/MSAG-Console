package ru.novosoft.smsc.admin.cluster_controller;

import org.junit.Test;
import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class CCProfileTest {
  @Test
  public void testSetLatin1() throws Exception {
    CCProfile p = new CCProfile();
    p.setLatin1(false);

    assertFalse(p.isLatin1());

    p.setLatin1(true);

    assertTrue(p.isLatin1());
  }

  @Test
  public void testSetUcs2() throws Exception {
    CCProfile p = new CCProfile();
    p.setUcs2(false);

    assertFalse(p.isUcs2());

    p.setUcs2(true);

    assertTrue(p.isUcs2());
  }

  @Test
  public void testSetUssdIn7Bit() throws Exception {
    CCProfile p = new CCProfile();
    p.setUssdIn7Bit(false);

    assertFalse(p.isUssdIn7Bit());

    p.setUssdIn7Bit(true);

    assertTrue(p.isUssdIn7Bit());
  }
}
