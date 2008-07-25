package ru.sibinco.smsx.utils.operators;

import org.apache.log4j.Category;
import ru.aurorisoft.smpp.Address;
import ru.aurorisoft.smpp.SMPPAddressException;

import java.util.Iterator;

import com.eyeline.utils.tree.radix.TemplatesRadixTree;

/**
 * User: artem
 * Date: 09.07.2007
 */

public class OperatorsList {

  private static final Category log = Category.getInstance("OPERATORS LIST");

  private TemplatesRadixTree operatorsTree;

  private final OperatorsDataSource ds;


  public OperatorsList(String configFile) {
    ds = new OperatorsDataSource(configFile);
    reloadOperators();
  }

  public Operator getOperatorByAddress(String address) {
    try {
      final Address addr = new Address(address);
      return (Operator) operatorsTree.getValue("." + addr.getTon() + '.' + addr.getNpi() + '.' + addr.getAddress());
    } catch (SMPPAddressException e) {
      log.error(e,e);
      return null;
    }

  }

  public void reloadOperators() {
    final TemplatesRadixTree newOperatorsTree = new TemplatesRadixTree();
    try {
      for (Iterator iter = ds.getOperators().iterator(); iter.hasNext();) {
        final Operator oper = (Operator)iter.next();
        newOperatorsTree.add(oper.getMasks(), oper);
      }
    } catch (Throwable e) {
      log.error(e,e);
    }

    operatorsTree = newOperatorsTree;
  }
}
