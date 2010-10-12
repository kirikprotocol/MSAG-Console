package mobi.eyeline.informer.admin.retry_policies;

import mobi.eyeline.informer.admin.AdminException;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertTrue;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.10.2010
 * Time: 15:02:09
 */
public class RetryPolicyTest {


  @Test
  public void testEntry() throws AdminException {

    RetryPolicyEntry entry = new RetryPolicyEntry(1000,500);
    try {
      new RetryPolicyEntry(-1000,1000);
      assertTrue(false);
    }
    catch (AdminException e) {}

    try {
      new RetryPolicyEntry(1000,-1000);
      assertTrue(false);
    }
    catch (AdminException e) {}

    RetryPolicyEntry e2 = new RetryPolicyEntry(entry);
    assertTrue(e2.errCode==entry.errCode);
    assertTrue(e2.time==entry.time);
  }


  @Test
  public void testPolicy() throws AdminException {
    try {
      new RetryPolicy(null,100);
      assertTrue(false);
    }
    catch (AdminException e) {}

    try {
      new RetryPolicy("",100);
      assertTrue(false);
    }
    catch (AdminException e) {}

    try {
      new RetryPolicy("a",-100);
      assertTrue(false);
    }
    catch (AdminException e) {}

    RetryPolicy p = new RetryPolicy("b",1000);
    assertTrue(p.getPolicyId().equals("b"));
    assertTrue(p.getDefaultTime()==1000);
    try {
      p.setDefaultTime(-100);
      assertTrue(false);
    }
    catch (AdminException e) {}


    List<RetryPolicyEntry> entries = null;
    try {
      p.setEntries(entries);
      assertTrue(false);
    }
    catch (AdminException e){}

    entries = new ArrayList<RetryPolicyEntry>();
    RetryPolicyEntry e1 = new RetryPolicyEntry(1000,5000);
    RetryPolicyEntry e2 = new RetryPolicyEntry(1001,5001);
    entries.add(e1);
    entries.add(e2);
    p.setEntries(entries);

    assertTrue(p.getEntries().size()==2);
    assertTrue(p.getEntries().get(0)==e1);
    assertTrue(p.getEntries().get(0).getErrCode()==1000);
    assertTrue(p.getEntries().get(1).getErrCode()==1001);
    assertTrue(p.getEntries().get(0).getTime()==5000);
    assertTrue(p.getEntries().get(1).getTime()==5001);


    RetryPolicyEntry e3 = new RetryPolicyEntry(1000,5000);
    entries.add(e3);
    try {
      p.setEntries(entries);
      assertTrue(false);
    }
    catch (AdminException e){}

    RetryPolicy p2 = new RetryPolicy(p);
    assertTrue(p2.equals(p));
  }



}
