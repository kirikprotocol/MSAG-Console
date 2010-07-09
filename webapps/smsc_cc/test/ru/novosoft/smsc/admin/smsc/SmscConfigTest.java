package ru.novosoft.smsc.admin.smsc;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigException;
import testutils.TestUtils;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.*;

/**
 * @author Artem Snopkov
 */
public class SmscConfigTest {

  private static File configFile, backupDir;

  @BeforeClass
  public static void beforeClass() throws IOException, AdminException {
    configFile = TestUtils.exportResourceToRandomFile(SmscConfigTest.class.getResourceAsStream("config.xml"), ".smsc");
    backupDir = TestUtils.createRandomDir(".smscbackup");
  }

  @AfterClass
  public static void afterClass() {
    if (configFile != null)
      configFile.delete();
    if (backupDir != null)
      TestUtils.recursiveDeleteFolder(backupDir);
  }

  @Test
  public void loadTest() throws AdminException {
    SmscConfig config = new SmscConfig(configFile, backupDir, FileSystem.getFSForSingleInst());

    CommonSettings cs = config.getCommonSettings();
    assertNotNull(cs);

    assertEquals("mobile_access_address", cs.getAbInfoMobileAccessAddress());
    assertEquals(34,cs.getAbInfoProtocolId());
    assertEquals("abnfo",cs.getAbInfoServiceType());
    assertEquals("smsc.abinfo.sme.address",cs.getAbInfoSmppAccessAddress());
    assertEquals("abonentinfo",cs.getAbInfoSystemId());
    assertEquals(10000,cs.getAclPreCreateSize());
    assertEquals("localstore.dir/acl",cs.getAclStoreDir());
    assertEquals("1212",cs.getAdd_ussd_ssn()[0]);
    assertEquals("@localstore.dir@/aliases.bin",cs.getAliasStoreFile());
    assertEquals("en_en",cs.getDefault_locale());
    assertEquals("def",cs.getDirectives().get("d"));
    assertEquals("template",cs.getDirectives().get("t"));
    assertEquals(1000,cs.getEventQueueLimit());
    assertEquals("deliver",cs.getInmanChargingOther());
    assertEquals("deliver",cs.getInmanChargingPeer2peer());
    assertEquals("inman.host",cs.getInmanHost());
    assertEquals(399,cs.getInmanPort());
    assertEquals("en_en",cs.getLocales()[0].trim());
    assertEquals("ru_ru",cs.getLocales()[1].trim());
    assertEquals(12345,cs.getLsMaxStoreSize());
    assertEquals(5321,cs.getLsMinRollTime());
    assertEquals(1,cs.getMainLoopsCount());
    assertEquals(20,cs.getMapBusyMTDelay());
    assertEquals(5,cs.getMapIOTasksCount());
    assertEquals(20,cs.getMapLockedByMODelay());
    assertEquals(45,cs.getMapMOLockTimeout());
    assertEquals(600,cs.getMergeTimeout());
    assertEquals(60,cs.getMsArchiveInterval());
    assertEquals(12,cs.getMscFailureLimit());
    assertEquals(12,cs.getMscSingleAttemptTimeout());
    assertEquals("@localstore.dir@/msc.txt",cs.getMscStoreFile());
    assertEquals(51,cs.getProfilerConcatOff());
    assertEquals(50,cs.getProfilerConcatOn());
    assertEquals(3,cs.getProfilerDefault());
    assertEquals(52,cs.getProfilerDivertAbsentOff());
    assertEquals(53,cs.getProfilerDivertAbsentOn());
    assertEquals(54,cs.getProfilerDivertBarredOff());
    assertEquals(55,cs.getProfilerDivertBarredOn());
    assertEquals(56,cs.getProfilerDivertBlockedOff());
    assertEquals(57,cs.getProfilerDivertBlockedOn());
    assertEquals(58,cs.getProfilerDivertCapacityOff());
    assertEquals(59,cs.getProfilerDivertCapacityOn());
    assertEquals(31,cs.getProfilerDivertOff());
    assertEquals(30,cs.getProfilerDivertOn());
    assertEquals(60,cs.getProfilerDivertStatus());
    assertEquals(40,cs.getProfilerDivertTo());
    assertEquals(5,cs.getProfilerHide());
    assertEquals(21,cs.getProfilerLocaleEN());
    assertEquals(20,cs.getProfilerLocaleRU());
    assertEquals(34,cs.getProfilerProtocolId());
    assertEquals(1,cs.getProfilerReportFinal());
    assertEquals(2,cs.getProfilerReportFull());
    assertEquals(0,cs.getProfilerReportNone());
    assertEquals("prflr",cs.getProfilerServiceType());
    assertEquals("@localstore.dir@/profiles.bin",cs.getProfilerStoreFile());
    assertEquals("profiler",cs.getProfilerSystemId());
    assertEquals(10,cs.getProfilerTranslitOff());
    assertEquals(9,cs.getProfilerTranslitOn());
    assertEquals(4,cs.getProfilerUCS2());
    assertEquals(6,cs.getProfilerUnhide());
    assertEquals(8,cs.getProfilerUSSD7BitOff());
    assertEquals(7,cs.getProfilerUSSD7BitOn());
    assertEquals(34,cs.getProtocol_id());
    assertEquals(2000000,cs.getSchedulerHardLimit());
    assertEquals(1800000,cs.getSchedulerSoftLimit());
    assertEquals("@smsc.sc.address@",cs.getService_center_address());
    assertEquals("SMSC",cs.getService_type());
    assertEquals(10,cs.getSmppBindTimeout());
    assertEquals(1000,cs.getSmppDefaultConnectionsLimit());
    assertEquals(60,cs.getSmppInactivityTime());
    assertEquals(600,cs.getSmppInactivityTimeOut());
    assertEquals(60,cs.getSmppReadTimeout());
    assertEquals(1209600,cs.getSmsMaxValidTime());
    assertEquals(60,cs.getSnmpCacheTimeout());
    assertEquals("@localstore.dir@/snmp",cs.getSnmpCsvFileDir());
    assertEquals(60,cs.getSnmpCsvFileRollInterval());
    assertEquals(5,cs.getState_machines_count());
    assertEquals("smscsme",cs.getSystemId());
    assertEquals("timezones.xml",cs.getTimezones_config());
    assertEquals(1,cs.getTrafficShapeTimeFrame());
    assertEquals(1,cs.getTrafficStatTimeFrame());
    assertEquals("@smsc.ussd.address@",cs.getUssd_center_address());
    assertEquals(213,cs.getUssd_ssn());

    assertEquals(3, config.getSmscInstancesCount());

    for (int i=0; i<3; i++) {
      InstanceSettings s = config.getInstanceSettings(i);

      assertEquals(i + "", s.getAdminHost());
      assertEquals(i, s.getAdminPort());
      assertEquals(i + "", s.getAgentHost());
      assertEquals(i, s.getAgentPort());
      assertEquals(i + "", s.getCacheDir());
      assertEquals(i + "", s.getCorePerfHost());
      assertEquals(i, s.getCorePerfPort());
      assertEquals(i+ "", s.getLocalStoreFilename());
      assertEquals(i + "", s.getLocalStoreMessIdFile());
      assertEquals(i + "", s.getMessageStoreArchDir());
      assertEquals(i + "", s.getSmePerfHost());
      assertEquals(i, s.getSmePerfPort());
      assertEquals(i+ "", s.getSmppHost());
      assertEquals(i, s.getSmppPort());
    }
  }

  @Test
  public void saveTest() throws AdminException, XmlConfigException {
    // Загружаем первоначальный конфиг
    XmlConfig cfg = new XmlConfig();
    cfg.load(configFile);

    // Создаем инстанц SmscConfig
    SmscConfig config1 = new SmscConfig(configFile, backupDir, FileSystem.getFSForSingleInst());
    // Сохраняем SmscConfig
    config1.save();

    // Проверяем, что в директории backup появились файлы
    assertFalse(backupDir.delete()); // Не можем удалить директорию т.к. там появились файлы

    // Снова загружаем конфиг
    XmlConfig cfg1 = new XmlConfig();
    cfg1.load(configFile);

    // Проверяем эквивалентность первоначальному конфигу. 
    assertEquals(cfg, cfg1);
  }





}
