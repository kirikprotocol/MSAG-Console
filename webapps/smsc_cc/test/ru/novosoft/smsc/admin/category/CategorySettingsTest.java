package ru.novosoft.smsc.admin.category;

import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;

import java.util.ArrayList;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

/**
 * @author Artem Snopkov
 */
public class CategorySettingsTest {
  @Test
  public void testAddCategory() throws Exception {
    CategorySettings s = new CategorySettings(new ArrayList<Category>(), -1);

    s.addCategory("name");

    assertEquals(0, s.getLastCategoryId());

    try {
      s.addCategory("name");
      assertTrue(false);
    } catch (AdminException ignored) {}

    try {
      s.addCategory("");
      assertTrue(false);
    } catch (AdminException ignored) {}

    try {
      s.addCategory(null);
      assertTrue(false);
    } catch (AdminException ignored) {}
  }

  @Test
  public void testUpdateCategory() throws Exception {
    CategorySettings s = new CategorySettings(new ArrayList<Category>(), -1);

    s.addCategory("name");
    Category p = s.addCategory("name1");

    assertNotNull(p);
    assertEquals(p, s.getCategory(p.getId()));


    try {
      p.setName(null);
      s.updateCategory(p);
      assertTrue(false);
    } catch (AdminException ignored) {}

    try {
      p.setName("");
      s.updateCategory(p);
      assertTrue(false);
    } catch (AdminException ignored) {}

    try {
      p.setName("name");
      s.updateCategory(p);
      assertTrue(false);
    } catch (AdminException ignored) {}

    assertEquals("name1", s.getCategory(p.getId()).getName());
  }
}
