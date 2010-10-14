package mobi.eyeline.informer.admin.smsc;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertTrue;

/**
 * @author Aleksandr Khalitov
 */
public class SmscSettingsTest {
  
  
  private SmscSettings settings;
  
  private Smsc smsc;
  
  @Before
  public void before() throws AdminException{
    settings = new SmscSettings();
    smsc = new Smsc("name1");
    smsc.setHost("dsa");
    smsc.setPort(123);
    smsc.setPassword("qwerty");
    smsc.setInterfaceVersion(2);
    smsc.setSystemId("sid1");
    smsc.setSystemType("sType");
    smsc.setUssdServiceOp(12);
    smsc.setVlrUssdServiceOp(562);
    smsc.setTimeout(53);
    smsc.setRangeOfAddress(67);
  }
  
  
  @Test
  public void addGet() throws AdminException{
    
    settings.addSmsc(smsc);
    assertTrue(smsc == settings.getSmsc(smsc.getName()));

    assertTrue(settings.getSmscs().size() == 1);
    assertTrue(settings.getSmscs().iterator().next() == smsc);


    try{
      settings.addSmsc(smsc);
      assertTrue(false);
    }catch (AdminException e){}
  }


  @Test
  public void addUpdate() throws AdminException{

    settings.addSmsc(smsc);

    smsc = settings.getSmsc(smsc.getName());
    smsc.setHost("phoenix");

    settings.updateSmsc(smsc);

    assertTrue(settings.getSmscs().size() == 1);
    assertTrue(settings.getSmscs().iterator().next() == smsc);

    try{
      settings.updateSmsc(new Smsc("dasdsa"));
      assertTrue(false);
    }catch (AdminException e){}
  }

  @Test
  public void addRemove() throws AdminException{

    settings.addSmsc(smsc);

    settings.removeSmsc(smsc.getName());

    try{
      settings.updateSmsc(smsc);
      assertTrue(false);
    }catch (AdminException e){}

    assertTrue(settings.getSmscs().size() == 0);
    assertTrue(settings.getSmsc(smsc.getName()) == null);

    try{
      settings.updateSmsc(new Smsc("dasdsa"));
      assertTrue(false);
    }catch (AdminException e){}

    settings.addSmsc(smsc);
  }


}
