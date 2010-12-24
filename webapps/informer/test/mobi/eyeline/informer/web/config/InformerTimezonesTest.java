package mobi.eyeline.informer.web.config;

import org.junit.Before;
import org.junit.Test;

import java.util.*;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

/**
 * @author Artem Snopkov
 */
public class InformerTimezonesTest {

  private static InformerTimezone createTimezone(String id, String enName, String ruName) {
    Map<String, String> aliases = new HashMap<String, String>();
    aliases.put("ru", ruName);
    aliases.put("en", enName);
    return new InformerTimezone(TimeZone.getTimeZone(id), aliases);
  }

  private static InformerTimezones createTimezones(InformerTimezone ... zones) throws ConfigException {
    return new InformerTimezones(Arrays.asList(zones));
  }

  @Test(expected=ConfigException.class)
  public void testDuplicateIds() throws Exception {
    createTimezones(
        createTimezone("Asia/Novosibirsk", "a", "b"),
        createTimezone("Asia/Novosibirsk", "c", "d")
    );
  }

  @Test(expected=ConfigException.class)
  public void testDuplicateNames() throws Exception {
    createTimezones(
        createTimezone("Asia/Novosibirsk", "a", "b"),
        createTimezone("Europe/Moscow", "a", "d")
    );
  }

  @Test
  public void testGetTimezoneByID() throws Exception {
    InformerTimezones timezones = createTimezones(
        createTimezone("Asia/Novosibirsk", "a", "b"),
        createTimezone("Europe/Moscow", "c", "d")
    );

    assertNotNull(timezones.getTimezoneByID("Asia/Novosibirsk"));
    assertEquals("a", timezones.getTimezoneByID("Asia/Novosibirsk").getAlias(new Locale("en")));
  }

  @Test
  public void testGetTimezoneByAlias() throws Exception {
    InformerTimezones timezones = createTimezones(
        createTimezone("Asia/Novosibirsk", "a", "b"),
        createTimezone("Europe/Moscow", "c", "d")
    );

    assertNotNull(timezones.getTimezoneByAlias("a", new Locale("en")));
    assertEquals("Asia/Novosibirsk", timezones.getTimezoneByAlias("a", new Locale("en")).getId());
    assertEquals("Asia/Novosibirsk", timezones.getTimezoneByAlias("a").getId());
  }
}
