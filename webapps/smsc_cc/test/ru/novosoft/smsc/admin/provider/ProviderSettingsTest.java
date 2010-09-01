package ru.novosoft.smsc.admin.provider;

import junit.framework.TestCase;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;

import static org.junit.Assert.*;

import java.util.ArrayList;

/**
 * @author Artem Snopkov
 */
public class ProviderSettingsTest  {

  @Test
  public void testAddProvider() throws Exception {
    ProviderSettings s = new ProviderSettings(new ArrayList<Provider>(), -1);

    s.addProvider("name");

    assertEquals(0, s.getLastProviderId());

    try {
      s.addProvider("name");
      assertTrue(false);
    } catch (AdminException ignored) {}

    try {
      s.addProvider("");
      assertTrue(false);
    } catch (AdminException ignored) {}

    try {
      s.addProvider(null);
      assertTrue(false);
    } catch (AdminException ignored) {}
  }

  @Test
  public void testUpdateProvider() throws Exception {
    ProviderSettings s = new ProviderSettings(new ArrayList<Provider>(), -1);

    s.addProvider("name");
    Provider p = s.addProvider("name1");

    assertNotNull(p);
    assertEquals(p, s.getProvider(p.getId()));


    try {
      p.setName(null);
      s.updateProvider(p);
      assertTrue(false);
    } catch (AdminException ignored) {}

    try {
      p.setName("");
      s.updateProvider(p);
      assertTrue(false);
    } catch (AdminException ignored) {}

    try {
      p.setName("name");
      s.updateProvider(p);
      assertTrue(false);
    } catch (AdminException ignored) {}

    assertEquals("name1", s.getProvider(p.getId()).getName());
  }
}
