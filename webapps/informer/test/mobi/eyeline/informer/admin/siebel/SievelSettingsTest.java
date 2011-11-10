package mobi.eyeline.informer.admin.siebel;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Test;

import java.util.Properties;

import static org.junit.Assert.assertNotNull;

/**
 * author: Aleksandr Khalitov
 */
public class SievelSettingsTest {

  private static Properties buildRightProps() {
    Properties props = new Properties();
    props.setProperty("removeOnStop", Boolean.FALSE.toString());
    props.setProperty("timeout", "30");
    props.setProperty("statsPeriod", "30");
    props.setProperty("jdbc.pool.type", "MYSQL");
    props.setProperty("jdbc.user", "siebel");
    props.setProperty("jdbc.source", "siebel_source");
    props.setProperty("siebelUser", "siebel");
    props.setProperty("jdbc.password", "password123");
    props.setProperty("validityPeriod.def","10:00:00");
    props.setProperty("validityPeriod.min","01:00:00");
    props.setProperty("validityPeriod.max","12:00:00");
    props.setProperty("priority.min","1");
    props.setProperty("priority.max","1000");
    return props;
  }

  private static SiebelSettings buildSettigns() throws AdminException{
    return new SiebelSettings(buildRightProps());
  }

  @Test
  public void testOk() throws AdminException {
    buildSettigns().validate();
  }

  @Test(expected =  AdminException.class)
  public void testIllegTimeout() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("timeout","sdadasd");
    new SiebelSettings(p);
  }

  @Test(expected =  AdminException.class)
  public void testEmptyTimeout() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("timeout","");
    new SiebelSettings(p);
  }

  @Test(expected =  AdminException.class)
  public void testNullTimeout() throws AdminException {
    Properties p = buildRightProps();
    p.remove("timeout");
    new SiebelSettings(p);
  }


  @Test(expected =  AdminException.class)
  public void testIllegStatsPeriod() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("statsPeriod","sdadasd");
    new SiebelSettings(p);
  }

  @Test(expected =  AdminException.class)
  public void testEmptyStatsPeriod() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("statsPeriod","");
    new SiebelSettings(p);
  }

  @Test(expected =  AdminException.class)
  public void testNullType() throws AdminException {
    Properties p = buildRightProps();
    p.remove("jdbc.pool.type");
    new SiebelSettings(p).validate();
  }

  @Test(expected =  AdminException.class)
  public void testEmptyType() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("jdbc.pool.type","");
    new SiebelSettings(p).validate();
  }

  @Test(expected =  AdminException.class)
  public void testNullJDBCUser() throws AdminException {
    Properties p = buildRightProps();
    p.remove("jdbc.user");
    new SiebelSettings(p).validate();
  }

  @Test(expected =  AdminException.class)
  public void testEmptyJDBCUser() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("jdbc.user", "");
    new SiebelSettings(p).validate();
  }

  @Test(expected =  AdminException.class)
  public void testNullJDBCSource() throws AdminException {
    Properties p = buildRightProps();
    p.remove("jdbc.source");
    new SiebelSettings(p).validate();
  }

  @Test(expected =  AdminException.class)
  public void testEmptyJDBCSource() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("jdbc.source","");
    new SiebelSettings(p).validate();
  }

  @Test(expected =  AdminException.class)
  public void testNullUser() throws AdminException {
    Properties p = buildRightProps();
    p.remove("siebelUser");
    new SiebelSettings(p).validate();
  }

  @Test(expected =  AdminException.class)
  public void testEmptyUser() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("siebelUser","");
    new SiebelSettings(p).validate();
  }

  @Test(expected =  AdminException.class)
  public void testNullPass() throws AdminException {
    Properties p = buildRightProps();
    p.remove("jdbc.password");
    new SiebelSettings(p).validate();
  }

  @Test
  public void testEmptyPass() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("jdbc.password", "");
    new SiebelSettings(p).validate();
  }      

  @Test
  public void testNullValidityPeriodDef() throws AdminException {
    Properties p = buildRightProps();
    p.remove("validityPeriod.def");
    SiebelSettings s = new SiebelSettings(p);
    s.validate();
    assertNotNull(s.getDefValidityPeriod());
  }

  @Test
  public void testEmptyValidityPeriodDef() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("validityPeriod.def","");
    SiebelSettings s = new SiebelSettings(p);
    s.validate();
    assertNotNull(s.getDefValidityPeriod());
  }

  @Test(expected = SiebelException.class)
  public void testIllegValidityPeriodDef() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("validityPeriod.def","dsasdaasdasd");
    new SiebelSettings(p).validate();
  }

  @Test
  public void testNullValidityPeriodMax() throws AdminException {
    Properties p = buildRightProps();
    p.remove("validityPeriod.max");
    SiebelSettings s = new SiebelSettings(p);
    s.validate();
    assertNotNull(s.getMaxValidityPeriod());
  }

  @Test
  public void testEmptyValidityPeriodMax() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("validityPeriod.max","");
    SiebelSettings s = new SiebelSettings(p);
    s.validate();
    assertNotNull(s.getMaxValidityPeriod());
  }

  @Test(expected = SiebelException.class)
  public void testIllegValidityPeriodMax() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("validityPeriod.max","dsasdaasdasd");
    new SiebelSettings(p).validate();
  }

  @Test
  public void testNullValidityPeriodMin() throws AdminException {
    Properties p = buildRightProps();
    p.remove("validityPeriod.min");
    SiebelSettings s = new SiebelSettings(p);
    s.validate();
    assertNotNull(s.getMinValidityPeriod());
  }

  @Test
  public void testEmptyValidityPeriodMin() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("validityPeriod.min","");
    SiebelSettings s = new SiebelSettings(p);
    s.validate();
    assertNotNull(s.getMinValidityPeriod());
  }

  @Test(expected = SiebelException.class)
  public void testIllegValidityPeriodMin() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("validityPeriod.min","dsasdaasdasd");
    new SiebelSettings(p).validate();
  }

  @Test
  public void testDefValidityOk() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("validityPeriod.min","01:00:00");
    p.setProperty("validityPeriod.def","02:00:00");
    p.setProperty("validityPeriod.max","12:00:00");
    new SiebelSettings(p).validate();
  }

  @Test(expected = AdminException.class)
  public void testDefValidityError() throws AdminException {
    Properties p = buildRightProps();
    p.setProperty("validityPeriod.min","03:00:00");
    p.setProperty("validityPeriod.def","02:00:00");
    p.setProperty("validityPeriod.max","12:00:00");
    new SiebelSettings(p).validate();
  }

}
