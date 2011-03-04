package ru.novosoft.smsc.jsp.smsc.network_profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.network_profiles.Mask;
import ru.novosoft.smsc.jsp.util.helper.Validation;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.Column;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.Row;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.RowControlButtonColumn;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.TextColumn;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

/**
 * @author Aleksandr Khalitov
 */
public class MasksHelper extends DynamicTableHelper{

  private Column maskColumn;

  public MasksHelper(String name, String uid, int width) {
    super(name, uid);
    maskColumn = new TextColumn(this, "network.masks", uid + "_name", width, Validation.MASK, true);
    Column delColumn = new RowControlButtonColumn(this, "", "delColumn");
    addColumn(maskColumn);
    addColumn(delColumn);
  }

  protected void fillTable() {
  }

  public List getMasks() throws AdminException {
    final List result = new LinkedList();
    for (Iterator iter = getRows(); iter.hasNext();) {
      Row row = (Row) iter.next();
      String mask = (String) (row.getValue(maskColumn));
      result.add(new Mask(mask));
    }
    return result;
  }

  public void fillMasks(Collection masks) {
    if(masks!=null){
      Iterator iter = masks.iterator();
      while(iter.hasNext()) {
        Mask m = (Mask)iter.next();
        Row row = createNewRow();
        row.addValue(maskColumn, m.getMaskSimple());
        System.out.println("Add row: "+m.getMaskSimple());
      }
    }
  }
}
