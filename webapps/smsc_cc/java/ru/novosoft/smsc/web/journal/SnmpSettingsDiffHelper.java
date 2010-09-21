package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.snmp.SnmpCounter;
import ru.novosoft.smsc.admin.snmp.SnmpObject;
import ru.novosoft.smsc.admin.snmp.SnmpSettings;
import ru.novosoft.smsc.admin.snmp.SnmpSeverity;

import java.util.Map;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 20.09.2010
 * Time: 14:07:58
 */
public class SnmpSettingsDiffHelper extends DiffHelper {
  public SnmpSettingsDiffHelper(String subject) {
    super(subject);
  }


  public void logChanges(Journal j, SnmpSettings oldSettings, SnmpSettings newSettings, String user) {
    oldSettings.getCounterInterval();

    if(oldSettings.getCounterInterval()!=newSettings.getCounterInterval()) {
        j.addRecord(JournalRecord.Type.CHANGE,subject,user).setDescription("snmp.counterInterval.change",
               Integer.toString(oldSettings.getCounterInterval()),
               Integer.toString(newSettings.getCounterInterval()));
    }
    logObjectChanges(j,null,oldSettings.getDefaultSnmpObject(),newSettings.getDefaultSnmpObject(),user);

    Map<String,SnmpObject> oldSnmpObjects = oldSettings.getSnmpObjects();
    Map<String,SnmpObject> newSnmpObjects = newSettings.getSnmpObjects();

    for(String objName : oldSettings.getSnmpObjects().keySet()) {
      if(newSnmpObjects.get(objName)==null) {
        SnmpObject oldSnmpObject = oldSettings.getSnmpObjects().get(objName);
        logCounter(j, objName, "counter000d" ,oldSnmpObject.getCounter000d(), null,user);
        logCounter(j, objName, "counter0014" ,oldSnmpObject.getCounter0014(), null,user);
        logCounter(j, objName, "counter0058" ,oldSnmpObject.getCounter0058(), null,user);
        logCounter(j, objName, "counterAccepted" ,oldSnmpObject.getCounterAccepted(), null,user);
        logCounter(j, objName, "counterDelivered" ,oldSnmpObject.getCounterDelivered(), null,user);
        logCounter(j, objName, "counterFailed" ,oldSnmpObject.getCounterFailed(), null,user);
        logCounter(j, objName, "counterOther" ,oldSnmpObject.getCounterOther(), null,user);
        logCounter(j, objName, "counterRejected" ,oldSnmpObject.getCounterRejected(), null,user);
        logCounter(j, objName, "counterRetired" ,oldSnmpObject.getCounterRetried(), null,user);
        logCounter(j, objName, "counterSDP" ,oldSnmpObject.getCounterSDP(), null,user);
        logCounter(j, objName, "counterTempError" ,oldSnmpObject.getCounterTempError(), null,user);

        logSeverity(j, objName,"severityRegister",oldSnmpObject.getSeverityRegister(),null,user);
        logSeverity(j, objName,"severityRegisterFailed",oldSnmpObject.getSeverityRegisterFailed(),null,user);
        logSeverity(j, objName,"severityUnregister",oldSnmpObject.getSeverityUnregister(),null,user);
        logSeverity(j, objName,"severityUnregisterFailed",oldSnmpObject.getSeverityUnregisterFailed(),null,user);        
        j.addRecord(JournalRecord.Type.REMOVE,subject,user).setDescription("snmp.object.remove",objName);
      }
      else {
        logObjectChanges(j,objName,oldSnmpObjects.get(objName),newSnmpObjects.get(objName),user);
      }
    }

    for(String objName : newSettings.getSnmpObjects().keySet()) {
      if(oldSettings.getSnmpObjects().get(objName)==null) {
        SnmpObject newSnmpObject = newSettings.getSnmpObjects().get(objName);
        j.addRecord(JournalRecord.Type.ADD,subject,user).setDescription("snmp.object.add",objName);
        logCounter(j, objName, "counter000d" ,null, newSnmpObject.getCounter000d(),user);
        logCounter(j, objName, "counter0014" ,null, newSnmpObject.getCounter0014(),user);
        logCounter(j, objName, "counter0058" ,null, newSnmpObject.getCounter0058(),user);
        logCounter(j, objName, "counterAccepted" ,null, newSnmpObject.getCounterAccepted(),user);
        logCounter(j, objName, "counterDelivered" ,null, newSnmpObject.getCounterDelivered(),user);
        logCounter(j, objName, "counterFailed" ,null, newSnmpObject.getCounterFailed(),user);
        logCounter(j, objName, "counterOther" ,null, newSnmpObject.getCounterOther(),user);
        logCounter(j, objName, "counterRejected" ,null, newSnmpObject.getCounterRejected(),user);
        logCounter(j, objName, "counterRetired" ,null, newSnmpObject.getCounterRetried(),user);
        logCounter(j, objName, "counterSDP" ,null, newSnmpObject.getCounterSDP(),user);
        logCounter(j, objName, "counterTempError" ,null, newSnmpObject.getCounterTempError(),user);

        logSeverity(j, objName,"severityRegister",null,newSnmpObject.getSeverityRegister(),user);
        logSeverity(j, objName,"severityRegisterFailed",null,newSnmpObject.getSeverityRegisterFailed(),user);
        logSeverity(j, objName,"severityUnregister",null,newSnmpObject.getSeverityUnregister(),user);
        logSeverity(j, objName,"severityUnregisterFailed",null,newSnmpObject.getSeverityUnregisterFailed(),user);

      }
    }



  }


  private void logObjectChanges(Journal j, String objName, SnmpObject oldSnmpObject, SnmpObject newSnmpObject, String user) {
     if(objName==null) objName = "DEFAULT";
     if(oldSnmpObject.isEnabled()!=newSnmpObject.isEnabled()) {
       j.addRecord(JournalRecord.Type.CHANGE,subject,user).setDescription("snmp.object.enableChanged",objName,
           Boolean.toString(newSnmpObject.isEnabled())
       );
     }
     logCounter(j, objName, "counter000d" ,oldSnmpObject.getCounter000d(), newSnmpObject.getCounter000d(),user);
     logCounter(j, objName, "counter0014" ,oldSnmpObject.getCounter0014(), newSnmpObject.getCounter0014(),user);
     logCounter(j, objName, "counter0058" ,oldSnmpObject.getCounter0058(), newSnmpObject.getCounter0058(),user);
     logCounter(j, objName, "counterAccepted" ,oldSnmpObject.getCounterAccepted(), newSnmpObject.getCounterAccepted(),user);
     logCounter(j, objName, "counterDelivered" ,oldSnmpObject.getCounterDelivered(), newSnmpObject.getCounterDelivered(),user);
     logCounter(j, objName, "counterFailed" ,oldSnmpObject.getCounterFailed(), newSnmpObject.getCounterFailed(),user);
     logCounter(j, objName, "counterOther" ,oldSnmpObject.getCounterOther(), newSnmpObject.getCounterOther(),user);
     logCounter(j, objName, "counterRejected" ,oldSnmpObject.getCounterRejected(), newSnmpObject.getCounterRejected(),user);
     logCounter(j, objName, "counterRetired" ,oldSnmpObject.getCounterRetried(), newSnmpObject.getCounterRetried(),user);
     logCounter(j, objName, "counterSDP" ,oldSnmpObject.getCounterSDP(), newSnmpObject.getCounterSDP(),user);
     logCounter(j, objName, "counterTempError" ,oldSnmpObject.getCounterTempError(), newSnmpObject.getCounterTempError(),user);

     logSeverity(j, objName,"severityRegister",oldSnmpObject.getSeverityRegister(),newSnmpObject.getSeverityRegister(),user);
     logSeverity(j, objName,"severityRegisterFailed",oldSnmpObject.getSeverityRegisterFailed(),newSnmpObject.getSeverityRegisterFailed(),user);
     logSeverity(j, objName,"severityUnregister",oldSnmpObject.getSeverityUnregister(),newSnmpObject.getSeverityUnregister(),user);
     logSeverity(j, objName,"severityUnregisterFailed",oldSnmpObject.getSeverityUnregisterFailed(),newSnmpObject.getSeverityUnregisterFailed(),user);
  }

  private void logSeverity(Journal j, String objName, String severityKey, SnmpSeverity severityOld, SnmpSeverity severityNew, String user) {
     if(severityNew==null && severityOld==null) return;
     if(severityOld==null) {
       j.addRecord(JournalRecord.Type.ADD,subject,user).setDescription("snmp.severity.add",severityKey,objName,
               severityNew.toString()
       );
     }
     else if(severityNew==null) {
       j.addRecord(JournalRecord.Type.REMOVE,subject,user).setDescription("snmp.severity.remove",severityKey,objName,
               severityOld.toString()
       );
     }
     else {
       if(!severityOld.equals(severityNew)) {
         j.addRecord(JournalRecord.Type.CHANGE,subject,user).setDescription("snmp.severity.change",severityKey,objName,
               severityOld.toString(),severityNew.toString()
         );
       }
     }
  }

  private void logCounter(Journal j, String objName, String counterKey, SnmpCounter counterOld, SnmpCounter counterNew, String user) {
     if(counterOld==null && counterNew==null) return;
     if(counterOld==null) {
       j.addRecord(JournalRecord.Type.ADD,subject,user).setDescription("snmp.counter.add",counterKey,objName,
               Integer.toString(counterNew.getWarning()),
               Integer.toString(counterNew.getMinor()),
               Integer.toString(counterNew.getMajor()),
               Integer.toString(counterNew.getCritical())
       );
     }
     else if(counterNew==null) {
       j.addRecord(JournalRecord.Type.REMOVE,subject,user).setDescription("snmp.counter.remove",counterKey,objName,
               Integer.toString(counterOld.getWarning()),
               Integer.toString(counterOld.getMinor()),
               Integer.toString(counterOld.getMajor()),
               Integer.toString(counterOld.getCritical())
       );
     }
     else {
       if(counterOld.getWarning()!=counterNew.getWarning()) {
         j.addRecord(JournalRecord.Type.CHANGE,subject,user).setDescription("snmp.counter.change.warining",counterKey,objName,
               Integer.toString(counterOld.getWarning()),
               Integer.toString(counterNew.getWarning()));
       }
       if(counterOld.getMinor()!=counterNew.getMinor()) {
         j.addRecord(JournalRecord.Type.CHANGE,subject,user).setDescription("snmp.counter.change.minor",counterKey,objName,
               Integer.toString(counterOld.getMinor()),
               Integer.toString(counterNew.getMinor()));
       }
       if(counterOld.getMajor()!=counterNew.getMajor()) {
         j.addRecord(JournalRecord.Type.CHANGE,subject,user).setDescription("snmp.counter.change.major",counterKey,objName,
               Integer.toString(counterOld.getMajor()),
               Integer.toString(counterNew.getMajor()));
       }
       if(counterOld.getCritical()!=counterNew.getCritical()) {
         j.addRecord(JournalRecord.Type.CHANGE,subject,user).setDescription("snmp.counter.change.critical",counterKey,objName,
               Integer.toString(counterOld.getCritical()),
               Integer.toString(counterNew.getCritical()));
       }
     }
  }
}
