<%@ page import="ru.novosoft.smsc.mcisme.backend.CountersSet,
                 ru.novosoft.smsc.mcisme.backend.RuntimeSet"%><jsp:useBean id="bean" class="ru.novosoft.smsc.mcisme.beans.Index"/><jsp:setProperty name="bean" property="*"/><%
bean.process(request);
CountersSet cset = bean.getStatistics(); RuntimeSet  rset = bean.getRuntime();
%>rstatActive, rstatInQS, rstatOutQS, rstatInSpeed, rstatOutSpeed, cstatMissed, cstatDelivered, cstatFailed, cstatNotified
<%= (rset != null) ? rset.activeTasks:0%>,<%= (rset != null) ? rset.inQueueSize:0%>,<%= (rset != null) ? rset.outQueueSize:0%>,<%= (rset != null) ? rset.inSpeed:0%>,<%= (rset != null) ? rset.outSpeed:0%>,<%= (cset != null) ? cset.missed:0%>,<%= (cset != null) ? cset.delivered:0%>,<%= (cset != null) ? cset.failed:0%>,<%= (cset != null) ? cset.notified:0%>
