package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.jsp.util.helper.dynamictable.DynamicTableHelper;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.Column;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.Row;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.IncorrectValueException;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.TextColumn;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.column.RowControlButtonColumn;
import ru.novosoft.smsc.jsp.util.helper.Validation;
import ru.novosoft.smsc.infosme.backend.config.InfoSmeConfig;

import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;
import java.util.Collection;

/**
 * author: alkhal
 */
public class OptionsSmscHelper extends DynamicTableHelper{

  private Column nameColumn;
  private Column sidColumn;
  private Column passColumn;
  private Column timeoutColumn;
  private Column hostColumn;
  private Column portColumn;

  public OptionsSmscHelper(String name, String uid, int width,  boolean allowEditPropsAfterAdd) {
    super(name, uid);
    nameColumn = new TextColumn(this, "infosme.label.smsc.name", uid + "_name", width / 6, Validation.NON_EMPTY, allowEditPropsAfterAdd);
    sidColumn = new TextColumn(this, "infosme.label.smsc.sid", uid + "_sid", width / 5, Validation.NON_EMPTY, allowEditPropsAfterAdd);
    passColumn = new TextColumn(this, "infosme.label.smsc.pass", uid + "_pass", width / 6, null, allowEditPropsAfterAdd);
    hostColumn = new TextColumn(this, "infosme.label.smsc.host", uid + "_host", width / 6, Validation.NON_EMPTY, allowEditPropsAfterAdd);
    portColumn = new TextColumn(this, "infosme.label.smsc.port", uid + "_port", width / 6, Validation.PORT, allowEditPropsAfterAdd);
    timeoutColumn = new TextColumn(this, "infosme.label.smsc.timeout", uid + "_timeout", width / 6, Validation.POSITIVE, allowEditPropsAfterAdd);
    Column delColumn = new RowControlButtonColumn(this, "", "delColumn");
    addColumn(nameColumn);
    addColumn(sidColumn);
    addColumn(passColumn);
    addColumn(hostColumn);
    addColumn(portColumn);
    addColumn(timeoutColumn);
    addColumn(delColumn);
  }

  protected void fillTable() {
  }

  public List getSmscConn() throws IncorrectValueException {
    final List result = new LinkedList();
    for (Iterator iter = getRows(); iter.hasNext();) {
      Row row = (Row) iter.next();
      String name = (String) (row.getValue(nameColumn));
      String host = (String) (row.getValue(hostColumn));
      String port = (String) (row.getValue(portColumn));
      String sid = (String) (row.getValue(sidColumn));
      String password = (String) (row.getValue(passColumn));
      String timeout = (String) (row.getValue(timeoutColumn));

      try{
        InfoSmeConfig.SmscConnector conn = new InfoSmeConfig.SmscConnector();
        conn.setHost(host.trim());
        conn.setPort(Integer.parseInt(port.trim()));
        conn.setTimeout(Integer.parseInt(timeout.trim()));
        conn.setName(name.trim());
        conn.setSid(sid.trim());
        conn.setPassword(password.trim());
        result.add(conn);
      }catch (NumberFormatException e) {
        throw new IncorrectValueException(e);
      }
    }
    return result;
  }

  public void fillSmscConn(Collection conns) {
    if(conns!=null){
      Iterator iter = conns.iterator();
      while(iter.hasNext()) {
        InfoSmeConfig.SmscConnector conn = (InfoSmeConfig.SmscConnector)iter.next();
        Row row = createNewRow();
        row.addValue(nameColumn,conn.getName());
        row.addValue(hostColumn,conn.getHost());
        row.addValue(portColumn,Integer.toString(conn.getPort()));
        row.addValue(sidColumn,conn.getSid());
        row.addValue(passColumn,conn.getPassword());
        row.addValue(timeoutColumn, Integer.toString(conn.getTimeout()));
      }
    }
  }





}
