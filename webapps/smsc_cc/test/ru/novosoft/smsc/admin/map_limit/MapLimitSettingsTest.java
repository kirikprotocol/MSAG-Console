package ru.novosoft.smsc.admin.map_limit;

import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.TestClusterController;

import static org.junit.Assert.assertFalse;

/**
 * @author Artem Snopkov
 */
public class MapLimitSettingsTest {

  @Test
  public void setCongestionLevelsTest() throws AdminException {
    MapLimitSettings settings = new MapLimitSettings();
    try {
      CongestionLevel levels[] = new CongestionLevel[MapLimitManager.MAX_CONGESTON_LEVELS];
      settings.setCongestionLevels(levels);
      assertFalse(true);
    } catch (AdminException e) {
    }

    try {
      CongestionLevel levels[] = new CongestionLevel[]{new CongestionLevel(1, 2, 3, 4)};
      settings.setCongestionLevels(levels);
      assertFalse(true);
    } catch (AdminException e) {
    }

    try {
      CongestionLevel levels[] = new CongestionLevel[]{
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4),
          new CongestionLevel(1, 2, 3, 4)
      };
      settings.setCongestionLevels(levels);

    } catch (AdminException e) {
      assertFalse(true);
    }
  }

}
