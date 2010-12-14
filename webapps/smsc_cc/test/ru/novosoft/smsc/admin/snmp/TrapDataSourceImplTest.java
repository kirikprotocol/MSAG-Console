package ru.novosoft.smsc.admin.snmp;

import org.junit.Test;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Functions;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */
public class TrapDataSourceImplTest extends TrapDataSourceTestStub{

  @Test
  public void testSnmpImport() throws Exception{

    TrapDataSourceImpl m = new TrapDataSourceImpl(snmpDir, FileSystem.getFSForSingleInst());

    SimpleDateFormat dateFormat = new SimpleDateFormat("yyyyMMdd");

    final int[] recordSize = new int[]{0};

    class V implements SnmpTrapVisitor {
      public boolean visit(SnmpTrap r) throws AdminException {
        System.out.println(r);
        recordSize[0]++;
        return true;
      }
    }

    V v = new V();

    Date from  = Functions.convertTime(dateFormat.parse("20101109"), TimeZone.getTimeZone("UTC"), TimeZone.getDefault());
    Date till  = Functions.convertTime(dateFormat.parse("20101111"), TimeZone.getTimeZone("UTC"), TimeZone.getDefault());

    m.getTraps(from, till, v);
    assertEquals(recordSize[0], 4);

    recordSize[0] = 0;


    System.out.println();
    System.out.println();

    dateFormat = new SimpleDateFormat("yyyyMMddHHmmss");

    from  = Functions.convertTime(dateFormat.parse("20101109221300"), TimeZone.getTimeZone("UTC"), TimeZone.getDefault());
    till  = Functions.convertTime(dateFormat.parse("20101109221400"), TimeZone.getTimeZone("UTC"), TimeZone.getDefault());

    m.getTraps(from, till, v);
    assertEquals(recordSize[0], 2);

    recordSize[0] = 0;


    System.out.println();
    System.out.println();


    from  = Functions.convertTime(dateFormat.parse("20101109221600"), TimeZone.getTimeZone("UTC"), TimeZone.getDefault());
    till  = null;

    m.getTraps(from, till, v);
    assertEquals(recordSize[0], 0);

    recordSize[0] = 0;

    System.out.println();
    System.out.println();


    from  = null;
    till  = Functions.convertTime(dateFormat.parse("20101109221000"), TimeZone.getTimeZone("UTC"), TimeZone.getDefault());

    m.getTraps(from, till, v);
    assertEquals(recordSize[0], 0);

    recordSize[0] = 0;

  }
}


