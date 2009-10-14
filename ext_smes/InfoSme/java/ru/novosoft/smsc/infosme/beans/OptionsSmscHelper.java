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
  private Column systemTypeColumn;
  private Column rangeOfAddressColumn;
  private Column interfaceVersionColumn;

  public OptionsSmscHelper(String name, String uid, int width,  boolean allowEditPropsAfterAdd) {
    super(name, uid);
    nameColumn = new TextColumn(this, "infosme.label.smsc.name", uid + "_name", width / 9, Validation.NON_EMPTY, allowEditPropsAfterAdd);
    sidColumn = new TextColumn(this, "infosme.label.smsc.sid", uid + "_sid", width / 9, Validation.NON_EMPTY, allowEditPropsAfterAdd);
    passColumn = new TextColumn(this, "infosme.label.smsc.pass", uid + "_pass", width / 9, null, allowEditPropsAfterAdd);
    hostColumn = new TextColumn(this, "infosme.label.smsc.host", uid + "_host", width / 9, Validation.NON_EMPTY, allowEditPropsAfterAdd);
    portColumn = new TextColumn(this, "infosme.label.smsc.port", uid + "_port", width / 9, Validation.PORT, allowEditPropsAfterAdd);
    timeoutColumn = new TextColumn(this, "infosme.label.smsc.timeout", uid + "_timeout", width / 9, Validation.POSITIVE, allowEditPropsAfterAdd);
    systemTypeColumn = new TextColumn(this, "infosme.label.smsc.systemType", uid + "_systemType", width / 9, 12, null, allowEditPropsAfterAdd);
    rangeOfAddressColumn = new TextColumn(this, "infosme.label.smsc.rangeOfAddress", uid + "_rangeOfAddress", width / 9, 40, null, allowEditPropsAfterAdd);
    interfaceVersionColumn = new TextColumn(this, "infosme.label.smsc.interfaceVersion", uid + "_interfaceVersionColumn", width / 9, null, allowEditPropsAfterAdd);
    Column delColumn = new RowControlButtonColumn(this, "", "delColumn");
    addColumn(nameColumn);
    addColumn(sidColumn);
    addColumn(passColumn);
    addColumn(hostColumn);
    addColumn(portColumn);
    addColumn(timeoutColumn);
    addColumn(systemTypeColumn);
    addColumn(rangeOfAddressColumn);
    addColumn(interfaceVersionColumn);
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
      String systemType = (String)(row.getValue(systemTypeColumn));
      if (systemType != null && systemType.trim().length() == 0)
        systemType = null;
      String rangeOfAddress = (String)(row.getValue(rangeOfAddressColumn));
      if (rangeOfAddress != null && rangeOfAddress.trim().length() == 0)
        rangeOfAddress = null;
      String interfaceVersionStr = (String)(row.getValue(interfaceVersionColumn));
      int intefaceVersion;
      if (interfaceVersionStr != null)
        intefaceVersion = convertInterfaceVersion(interfaceVersionStr);
      else
        intefaceVersion = -1;

      try{
        InfoSmeConfig.SmscConnector conn = new InfoSmeConfig.SmscConnector();
        conn.setHost(host.trim());
        conn.setPort(Integer.parseInt(port.trim()));
        conn.setTimeout(Integer.parseInt(timeout.trim()));
        conn.setName(name.trim());
        conn.setSid(sid.trim());
        conn.setPassword(password.trim());
        conn.setInterfaceVersion(intefaceVersion);
        conn.setSystemType(systemType);
        conn.setRangeOfAddress(rangeOfAddress);
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
        row.addValue(systemTypeColumn, conn.getSystemType() == null ? "" : conn.getSystemType());
        row.addValue(rangeOfAddressColumn, conn.getRangeOfAddress() == null ? "" : conn.getRangeOfAddress());
        row.addValue(interfaceVersionColumn, conn.getInterfaceVersion() >=0 ? convertInterfaceVersion(conn.getInterfaceVersion()) : "");
      }
    }
  }

  private static int convertInterfaceVersion(String version) {
    int pos = version.indexOf('.');
    if (pos > 0) {
      return (Integer.parseInt(version.substring(0, pos)) << 4) + (Integer.parseInt(version.substring(pos + 1)));
    } else
      return -1;
  }

  private static String convertInterfaceVersion(int v) {
    return String.valueOf(v >> 4) + '.' + String.valueOf(v & 0xF);
  }





}
