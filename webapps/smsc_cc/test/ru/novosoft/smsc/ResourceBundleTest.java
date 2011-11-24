package ru.novosoft.smsc;

import org.junit.Test;
import ru.novosoft.smsc.admin.AdminContextException;
import ru.novosoft.smsc.admin.acl.Acl;
import ru.novosoft.smsc.admin.acl.AclException;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasManagerException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonException;
import ru.novosoft.smsc.admin.archive_daemon.ArchiveDaemonSettings;
import ru.novosoft.smsc.admin.callable.CallableServiceException;
import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.category.CategoryException;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroup;
import ru.novosoft.smsc.admin.closed_groups.ClosedGroupException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerException;
import ru.novosoft.smsc.admin.cluster_controller.ClusterControllerSettings;
import ru.novosoft.smsc.admin.filesystem.FileSystemException;
import ru.novosoft.smsc.admin.fraud.FraudSettings;
import ru.novosoft.smsc.admin.logging.Logger;
import ru.novosoft.smsc.admin.logging.LoggerException;
import ru.novosoft.smsc.admin.map_limit.CongestionLevel;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.admin.mcisme.MCISmeException;
import ru.novosoft.smsc.admin.mcisme.MCISmeSettings;
import ru.novosoft.smsc.admin.msc.MscException;
import ru.novosoft.smsc.admin.operative_store.OperativeStoreException;
import ru.novosoft.smsc.admin.perfmon.PerfMonitorException;
import ru.novosoft.smsc.admin.profile.*;
import ru.novosoft.smsc.admin.protocol.ProtocolException;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.provider.ProviderException;
import ru.novosoft.smsc.admin.region.Region;
import ru.novosoft.smsc.admin.region.RegionException;
import ru.novosoft.smsc.admin.region.RegionSettings;
import ru.novosoft.smsc.admin.reschedule.Reschedule;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.resource.ResourceSettings;
import ru.novosoft.smsc.admin.route.*;
import ru.novosoft.smsc.admin.service.ServiceManagerException;
import ru.novosoft.smsc.admin.service.daemon.DaemonException;
import ru.novosoft.smsc.admin.service.resource_group.ResourceGroupException;
import ru.novosoft.smsc.admin.sme.*;
import ru.novosoft.smsc.admin.smsc.CommonSettings;
import ru.novosoft.smsc.admin.smsc.InstanceSettings;
import ru.novosoft.smsc.admin.smsc.SmscException;
import ru.novosoft.smsc.admin.snmp.SnmpCounter;
import ru.novosoft.smsc.admin.snmp.SnmpException;
import ru.novosoft.smsc.admin.snmp.SnmpObject;
import ru.novosoft.smsc.admin.snmp.SnmpSettings;
import ru.novosoft.smsc.admin.stat.StatException;
import ru.novosoft.smsc.admin.timezone.Timezone;
import ru.novosoft.smsc.admin.timezone.TimezoneException;
import ru.novosoft.smsc.admin.timezone.TimezoneSettings;
import ru.novosoft.smsc.admin.topmon.TopMonitorException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.admin.users.UserPreferences;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.admin.util.DBExportSettings;
import ru.novosoft.smsc.web.controllers.ConcurrentModificationException;
import ru.novosoft.smsc.web.journal.JournalRecord;

import java.io.IOException;
import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

import static org.junit.Assert.fail;

/**
 * User: artem
 * Date: 28.10.11
 */
public class ResourceBundleTest {

  @Test
  public void testBundles() throws IOException {
    assertResourceBundleCorrect(Acl.class);
    assertResourceBundleCorrect(AclException.class);
    assertResourceBundleCorrect(Alias.class);
    assertResourceBundleCorrect(AliasManagerException.class);
    assertResourceBundleCorrect(ArchiveDaemonException.class);
    assertResourceBundleCorrect(ArchiveDaemonSettings.class);
    assertResourceBundleCorrect(CallableServiceException.class);
    assertResourceBundleCorrect(Category.class);
    assertResourceBundleCorrect(CategoryException.class);
    assertResourceBundleCorrect(ClosedGroup.class);
    assertResourceBundleCorrect(ClosedGroupException.class);
    assertResourceBundleCorrect(ClusterControllerException.class);
    assertResourceBundleCorrect(ClusterControllerSettings.class);
    assertResourceBundleCorrect(FileSystemException.class);
    assertResourceBundleCorrect(FraudSettings.class);
    assertResourceBundleCorrect(Logger.class);
    assertResourceBundleCorrect(LoggerException.class);
    assertResourceBundleCorrect(CongestionLevel.class);
    assertResourceBundleCorrect(MapLimitSettings.class);
    assertResourceBundleCorrect(MscException.class);
    assertResourceBundleCorrect(OperativeStoreException.class);
    assertResourceBundleCorrect(PerfMonitorException.class);
    assertResourceBundleCorrect(AliasHide.class);
    assertResourceBundleCorrect(Profile.class);
    assertResourceBundleCorrect(ProfileException.class);
    assertResourceBundleCorrect(ProfileLookupResult.class);
    assertResourceBundleCorrect(ReportOption.class);
    assertResourceBundleCorrect(ProtocolException.class);
    assertResourceBundleCorrect(Provider.class);
    assertResourceBundleCorrect(ProviderException.class);
    assertResourceBundleCorrect(Region.class);
    assertResourceBundleCorrect(RegionException.class);
    assertResourceBundleCorrect(RegionSettings.class);
    assertResourceBundleCorrect(Reschedule.class);
    assertResourceBundleCorrect(RescheduleSettings.class);
    assertResourceBundleCorrect(ResourceSettings.class);
    assertResourceBundleCorrect(BillingMode.class);
    assertResourceBundleCorrect(DeliveryMode.class);
    assertResourceBundleCorrect(Destination.class);
    assertResourceBundleCorrect(ReplayPath.class);
    assertResourceBundleCorrect(Route.class);
    assertResourceBundleCorrect(RouteException.class);
    assertResourceBundleCorrect(RouteSubjectSettings.class);
    assertResourceBundleCorrect(Source.class);
    assertResourceBundleCorrect(Subject.class);
    assertResourceBundleCorrect(DaemonException.class);
    assertResourceBundleCorrect(ResourceGroupException.class);
    assertResourceBundleCorrect(ServiceManagerException.class);
    assertResourceBundleCorrect(Sme.class);
    assertResourceBundleCorrect(SmeBindMode.class);
    assertResourceBundleCorrect(SmeConnectStatus.class);
    assertResourceBundleCorrect(SmeConnectType.class);
    assertResourceBundleCorrect(SmeException.class);
    assertResourceBundleCorrect(SmeServiceStatus.class);
    assertResourceBundleCorrect(SmeSmscStatus.class);
    assertResourceBundleCorrect(CommonSettings.class);
    assertResourceBundleCorrect(InstanceSettings.class);
    assertResourceBundleCorrect(SmscException.class);
    assertResourceBundleCorrect(SnmpCounter.class);
    assertResourceBundleCorrect(SnmpException.class);
    assertResourceBundleCorrect(SnmpObject.class);
    assertResourceBundleCorrect(SnmpSettings.class);
    assertResourceBundleCorrect(DBExportSettings.class);
    assertResourceBundleCorrect(StatException.class);
    assertResourceBundleCorrect(Timezone.class);
    assertResourceBundleCorrect(TimezoneException.class);
    assertResourceBundleCorrect(TimezoneSettings.class);
    assertResourceBundleCorrect(TopMonitorException.class);
    assertResourceBundleCorrect(User.class);
    assertResourceBundleCorrect(UserPreferences.class);
    assertResourceBundleCorrect(UsersSettings.class);
    assertResourceBundleCorrect(AdminContextException.class);
    assertResourceBundleCorrect(ConcurrentModificationException.class);
    assertResourceBundleCorrect(JournalRecord.class);
    assertResourceBundleCorrect("ru.novosoft.smsc.web.resources.Smsc");
    assertResourceBundleCorrect(MCISmeException.class);
    assertResourceBundleCorrect(MCISmeSettings.class);
  }
  
  private static void assertContainsAllKeys(String bundleName, ResourceBundle props1, ResourceBundle props2) {
    for (Object key : props1.keySet()) {
      try {
        props2.getString((String) key);
      } catch (MissingResourceException ex) {
        fail("Invalid resource bundle: " + bundleName + ". Property '" + key + "' does not exists in all locales.");
      }
    }
  }

  public static void assertResourceBundleCorrect(Class clazz) {
    assertResourceBundleCorrect(clazz.getCanonicalName());
  }

  public static void assertResourceBundleCorrect(String bundleName) {
    ResourceBundle ru = ResourceBundle.getBundle(bundleName, new Locale("ru"));
    ResourceBundle en = ResourceBundle.getBundle(bundleName, new Locale("en"));

    assertContainsAllKeys(bundleName, ru, en);
    assertContainsAllKeys(bundleName, en, ru);
  }

}
