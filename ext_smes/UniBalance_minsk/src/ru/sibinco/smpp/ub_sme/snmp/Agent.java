package ru.sibinco.smpp.ub_sme.snmp;

//import com.sun.management.comm.SnmpAdaptorServer;
//import com.sun.management.snmp.*;
import com.sun.jmx.snmp.daemon.SnmpAdaptorServer;
import com.sun.jmx.snmp.*;

import javax.management.MBeanServer;
import javax.management.MBeanServerFactory;
import javax.management.ObjectName;

import ru.sibinco.smpp.ub_sme.InitializationException;

import java.util.Properties;
import java.net.InetAddress;
import java.net.UnknownHostException;

import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 23.12.2008
 * Time: 10:55:15
 * To change this template use File | Settings | File Templates.
 */
public class Agent {
  private final static Category logger = Category.getInstance(Agent.class);

  private SnmpAdaptorServer snmpAdaptor = null;
  private InetAddress addr;
  private String communityString="public";


  public final static String smscNewAlertFFMR_OID = "1.3.6.1.4.1.26757.1.0.4";

  public final static String alertMessage_OID = "1.3.6.1.4.1.26757.1.5";
  public final static String alertSeverity_OID = "1.3.6.1.4.1.26757.1.7";
  public final static String alertObjCategory_OID = "1.3.6.1.4.1.26757.1.9";
  public final static String alertId_OID = "1.3.6.1.4.1.26757.1.8";
    
  public final static byte SNMP_SEVERITY_NORMAL = 1;
  public final static byte SNMP_SEVERITY_WARNING = 2;
  public final static byte SNMP_SEVERITY_MINOR = 3;
  public final static byte SNMP_SEVERITY_MAJOR = 4;
  public final static byte SNMP_SEVERITY_CRITICAL = 5;

  public final static byte SNMP_STATUS_NEW = 0;
  public final static byte SNMP_STATUS_CLEAR = 1;

  public final static String[] SNMP_STATUS = {"NEW", "CLEAR"};

  public void init(Properties config) throws InitializationException {
    if (logger.isDebugEnabled()) logger.debug("SNMP Agent init started");

    final MBeanServer server;
    final ObjectName snmpObjName;
    int snmpPort = 161;

    String snmpHost=config.getProperty("snmp.agent.host","");
    if(snmpHost.length()==0){
      throw new InitializationException("Mandatory config parameter \"snmp.agent.host\" is missed!");
    }
    try {
      addr = InetAddress.getByName(snmpHost);
    } catch (UnknownHostException e) {
      throw new InitializationException("Config parameter \"snmp.agent.host\" is invalid: "+e, e);
    }

    try {
      snmpPort=Integer.parseInt(config.getProperty("snmp.agent.port", Integer.toString(snmpPort)));
    } catch (NumberFormatException e) {
      logger.error("Illegal int value: "+config.getProperty("snmp.agent.port"), e);
    }

    communityString=config.getProperty("snmp.agent.community.string",communityString);
    if(communityString.length()==0){
      throw new InitializationException("Mandatory config parameter \"snmp.agent.community.string\" is missed!");
    }

    try {
      server = MBeanServerFactory.createMBeanServer();
      String domain = server.getDefaultDomain();

      // Create and start the SNMP adaptor.
      //
      snmpObjName = new ObjectName(domain +
          ":class=SnmpAdaptorServer,protocol=snmp,port=" + snmpPort);
      snmpAdaptor = new SnmpAdaptorServer(snmpPort);
      server.registerMBean(snmpAdaptor, snmpObjName);
      snmpAdaptor.start();

    } catch (Exception e) {
      throw new InitializationException("Can't init SNMP Agent: "+e, e);
    }

    if (logger.isDebugEnabled()) logger.debug("SNMP Agent init finished");
  }

  public SnmpAdaptorServer getSnmpAdaptor() {
    return snmpAdaptor;
  }

  public void sendTrap(int status, String alarmId, int severity){
    if(logger.isDebugEnabled())
      logger.debug("Send trap: " + Agent.SNMP_STATUS[status] + " UNIBALANCE BannerRotator Threshold crossed (AlarmID="+alarmId+"; severity=" + severity+")");

    SnmpAdaptorServer snmpAdaptor = getSnmpAdaptor();
    SnmpVarBindList varBindList = new SnmpVarBindList();

    SnmpOid smscNewAlertFFMR_OID = new SnmpOid(Agent.smscNewAlertFFMR_OID);

    SnmpOid alertMessage_OID = new SnmpOid(Agent.alertMessage_OID);
    SnmpString alertMessage = new SnmpString("Threshold crossed (AlarmID="+alarmId+"; severity="+severity+")");
    SnmpVarBind alertMessageBind = new SnmpVarBind(alertMessage_OID, alertMessage);
    varBindList.addVarBind(alertMessageBind);

    SnmpOid alertSeverity_OID = new SnmpOid(Agent.alertSeverity_OID);
    SnmpInt alertSeverity = new SnmpInt(severity);
    SnmpVarBind alertSeverityBind = new SnmpVarBind(alertSeverity_OID, alertSeverity);
    varBindList.addVarBind(alertSeverityBind);

    SnmpOid alertObjCategory_OID = new SnmpOid(Agent.alertObjCategory_OID);
    SnmpString alertObjCategory = new SnmpString("UNIBALANCE");
    SnmpVarBind alertObjCategoryBind = new SnmpVarBind(alertObjCategory_OID, alertObjCategory);
    varBindList.addVarBind(alertObjCategoryBind);

    SnmpOid alertId_OID = new SnmpOid(Agent.alertId_OID);
    SnmpString alertId = new SnmpString(alarmId);
    SnmpVarBind alertIdBind = new SnmpVarBind(alertId_OID, alertId);
    varBindList.addVarBind(alertIdBind);

    try {
      snmpAdaptor.snmpV2Trap(addr, communityString, smscNewAlertFFMR_OID, varBindList);
    } catch (Exception e) {
      logger.error("Can't send trap: " + Agent.SNMP_STATUS[status] + " UNIBALANCE BannerRotator Threshold crossed (AlarmID="+alarmId+"; severity=" + severity + "): "+e, e);
    }
  }

}