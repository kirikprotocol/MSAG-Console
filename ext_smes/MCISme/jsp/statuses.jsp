<%@ page import="java.util.*, ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.mcisme.backend.CountersSet"%><jsp:useBean id="bean" class="ru.novosoft.smsc.mcisme.beans.Index"/><jsp:setProperty name="bean" property="*"/><%
bean.process(request);
CountersSet cset = bean.getStatistics();
%>MCISmeActiveAbonents, statMissed, statDelivered, statFailed, statNotified
<%= bean.getActiveTasksCount()%>,<%= (cset != null) ? cset.missed:0%>,<%= (cset != null) ? cset.delivered:0%>,<%= (cset != null) ? cset.failed:0%>,<%= (cset != null) ? cset.notified:0%>
