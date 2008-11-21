<%@ include file="/WEB-INF/inc/header.jspf" %>

<script src="content/scripts/tree.js"></script>

<sm:page title="config.title" onLoad="enableDisableStartStopButtonsForSCAGStatusPage();">
<jsp:attribute name="menu">
    <sm-pm:menu>
        <sm-pm:item name="mbSave" value="config.item.mbsave.value" enabled="false" title="config.item.mbsave.title"/>
        <sm-pm:item name="mbCancel" value="config.item.mbcancel.value" title="config.item.mbcancel.title" onclick="clickCancel()"/>
        <sm-pm:space/>
        <sm-pm:item name="mbStart" value="config.item.mbstart.value" title="config.item.mbstart.title" isCheckSensible="false"/>
        <sm-pm:item name="mbStop" value="config.item.mbstop.value" title="config.item.mbstop.title" isCheckSensible="false"/>
    </sm-pm:menu>
</jsp:attribute>

<jsp:body>

    <script language="javaScript">
        function enableDisableByIdFunction(itemId, isDisabled) {
//            var items = opForm.all[itemId];
            //alert( itemId);
            var items = document.getElementsByName(itemId);
//            alert( "LENGTH=" + items.length );
            for (var i = 0; i < items.length; i++) {
                items[i].disabled = isDisabled;
            }
        }

        function configChanged() {
            enableDisableByIdFunction('mbSave', false);
        }

        function openCloseProp(id){
//            alert("id=" + id);
            var tbl = getElementByIdUni(id);
            var divCat = getElementByIdUni(id+"_div");
//            alert( tbl+"_div" + " DIV=" + divCat );
            if(tbl.style.display=='none'){
                tbl.style.display = "";
                divCat.className = "collapsing_tree_opened";
            } else {
                tbl.style.display = "none";
                divCat.className = "collapsing_tree_closed";
            }
        }

        function enableDisableStartStopButtonsForSCAGStatusPage() {
            var SCAGStatusSpan = getElementByIdUni("SCAGStatusSpan");
            if (SCAGStatusSpan.innerText == ' <fmt:message>status.running</fmt:message>') {
                enableDisableByIdFunction('mbStart', SCAGStatusSpan.innerText == ' <fmt:message>status.running</fmt:message>');
                enableDisableByIdFunction('mbStop', SCAGStatusSpan.innerText == ' <fmt:message>status.stopped</fmt:message>');
            } else if (SCAGStatusSpan.innerText == ' <fmt:message>status.stopped</fmt:message>' || SCAGStatusSpan.innerText == ' <fmt:message>status.stopping</fmt:message>') {
                enableDisableByIdFunction('mbStop', SCAGStatusSpan.innerText == ' <fmt:message>status.stopped</fmt:message>');
                enableDisableByIdFunction('mbStart', SCAGStatusSpan.innerText == ' <fmt:message>status.running</fmt:message>');
            } else if (SCAGStatusSpan.innerText == ' <fmt:message>status.unknown</fmt:message>') {
                enableDisableByIdFunction('mbStop', all.SCAGStatusSpan.innerText == ' <fmt:message>status.unknown</fmt:message>');
                enableDisableByIdFunction('mbStart', SCAGStatusSpan.innerText == ' <fmt:message>status.unknown</fmt:message>');
            }
        }
    </script>
    <sm-et:section title="Billing Manager" name="BillingManager" type="table" >
<%--        <sm-ep:properties id="BillingManager" display="none">--%>
        <table id="BillingManager" style="display:none">
                <sm-et:txt title="Config Dir" name="configDir" validation="nonEmpty" onchanged="configChanged();"/>
                <sm-et:txt title="Billing Server Host" name="BillingServerHost" validation="nonEmpty"
                           onchanged="configChanged();"/>
                <sm-et:txt title="Billing Server Port" name="BillingServerPort" validation="port"
                           onchanged="configChanged();"/>
                <sm-et:txt title="Billing Time Out" name="BillingTimeOut" validation="nonEmpty"
                           onchanged="configChanged();"/>
        </table>
<%--        </sm-ep:properties>--%>
    </sm-et:section>

<%--    <sm-et:section title="Session Manager" name="SessionManager" type="table" >--%>
<%--        <sm-ep:properties id="SessionManager" display="none">--%>
<%--<%--        <table id="SessionManager" style="display:none">--%>
<%--            <sm-et:txt title="Location" name="location" type="string" onchanged="configChanged();"/>--%>
<%--            <sm-et:txt title="Expire Interval" name="expireInterval" type="int" onchanged="configChanged();"/>--%>
<%--<%--        </table>--%>
<%--        </sm-ep:properties>--%>
<%--    </sm-et:section>--%>

    <sm-et:section title="Session Manager" name="SessionManager" type="table" >
        <sm-ep:properties id="SessionManager" display="none">
<%--        <table id="SessionManager" style="display:none">--%>
<%--            <tr><td colspan=2>--%>
<%--                <sm-et:section title="Locations" name="Locations" type="table">--%>
<%--                    <sm-ep:properties id="Locations" display="none">--%>
<%--                        <sm-et:txt title="Name" name="name" type="string" onchanged="configChanged();"/>--%>
<%--                    </sm-ep:properties>--%>
<%--                </sm-et:section>--%>
<%--            </td></tr>--%>
<%--            <sm-et:txt title="Location" name="location" type="string" onchanged="configChanged();"/>--%>
<%--            <sm-et:txt title="Name" name="name" type="string" onchanged="configChanged();"/>--%>
            <sm-et:txt title="ExpireInterval"    name="expireInterval"     type="int"  onchanged="configChanged();"/>
            <sm-et:txt title="IndexGrowth"        name="indexGrowth"        type="int"  onchanged="configChanged();"/>
            <sm-et:txt title="Pagesize"           name="pagesize"           type="int"  onchanged="configChanged();"/>
            <sm-et:txt title="Prealloc"           name="prealloc"           type="int"  onchanged="configChanged();"/>
            <sm-et:txt title="FlushCountLimit"    name="flushCountLimit"    type="int"  onchanged="configChanged();"/>
            <sm-et:txt title="FlushTimeLimit"     name="flushTimeLimit"     type="int"  onchanged="configChanged();"/>
            <sm-et:txt title="initUploadCount"    name="initUploadCount"    type="int"  onchanged="configChanged();"/>
            <sm-et:txt title="initUploadInterval" name="initUploadInterval" type="int"  onchanged="configChanged();"/>
<%--            <sm-et:txt title="Diskio"             name="diskio"             type="bool" onchanged="configChanged();"/>--%>
            <sm-et:checkin title="Diskio"         name="diskio"                         onchanged="configChanged();"/>
<%--                <sm-et:section title="OldLocations" name="Locations" type="table">--%>
<%--                    <sm-ep:properties id="Locations" display="none">--%>
<%--                        <sm-et:txt title="Name" name="name" type="string" onchanged="configChanged();"/>--%>
<%--                    </sm-ep:properties>--%>
<%--                </sm-et:section>--%>

        </sm-ep:properties>
    </sm-et:section>


    <sm-et:section title="Statistics Manager" name="StatisticsManager" type="table">
        <sm-ep:properties id="StatisticsManager" display="none">
<%--        <table id="StatisticsManager" style="display:none">--%>
            <sm-et:txt title="statistics Dir" name="statisticsDir" validation="nonEmpty" onchanged="configChanged();"/>
            <sm-et:txt title="Perf. Host" name="perfHost" validation="nonEmpty" onchanged="configChanged();"/>
            <sm-et:txt title="Perf. Gen Port" name="perfGenPort" validation="port" onchanged="configChanged();"/>
            <sm-et:txt title="Perf. Svc Port" name="perfSvcPort" validation="port" onchanged="configChanged();"/>
            <sm-et:txt title="Perf. Sc Port" name="perfScPort" validation="port" onchanged="configChanged();"/>
            <sm-et:txt title="Sacc Port" name="saccPort" validation="port" onchanged="configChanged();"/>
            <sm-et:txt title="Sacc Host" name="saccHost" validation="nonEmpty"/>
            <sm-et:txt title="Connect Timeout" name="connect_timeout" validation="nonEmpty"
                       onchanged="configChanged();"/>
            <sm-et:txt title="Queue Length" name="queue_length" validation="nonEmpty" onchanged="configChanged();"/>
<%--        </table>--%>
        </sm-ep:properties>
    </sm-et:section>

    <sm-et:section title="Rule Engine" name="RuleEngine" type="table" >
        <sm-ep:properties id="RuleEngine" display="none">
<%--        <table id="RuleEngine" style="display:none">--%>
            <sm-et:txt title="Location" name="location" type="string" onchanged="configChanged();"/>
<%--        </table>--%>
        </sm-ep:properties>
    </sm-et:section>

    <sm-et:section title="Administration" name="admin" type="table" >
        <sm-ep:properties id="admin" display="none">
<%--        <table id="admin" style="display:none">--%>
            <sm-et:txt name="host" type="string" onchanged="configChanged();"/>
            <sm-et:txt name="port" type="int" validation="port" onchanged="configChanged();"/>
<%--        </table>--%>
        </sm-ep:properties>
    </sm-et:section>

    <sm-et:section title="SMPP" name="smpp" type="table" >
        <sm-ep:properties id="smpp" display="none">
            <sm-et:txt title="Host"                  name="host"                type="string" onchanged="configChanged();"/>
            <sm-et:txt title="Port"                  name="port"                type="int"    onchanged="configChanged();" validation="port"/>
            <sm-et:txt title="Inactivity Time"       name="inactivityTime"      type="int"    onchanged="configChanged();"/>
            <sm-et:txt title="Inactivity Time Out"   name="inactivityTimeOut"   type="int"    onchanged="configChanged();"/>
            <sm-et:txt title="Read Timeout"          name="readTimeout"         type="int"    onchanged="configChanged();"/>
            <sm-et:txt title="Max Sms PerSecond"     name="maxSmsPerSecond"     type="int"    onchanged="configChanged();"/>
            <sm-et:txt title="Transit Optional Tags" name="transitOptionalTags" type="string" onchanged="configChanged();"  validation="transitOptionalTags"
                       comments="config.section.txt.transit_optional_tags.comments"/>
            <tr><td colspan=2>
                <sm-et:section title="Core" name="core" type="table">
                    <sm-ep:properties id="core" display="none">
                        <sm-et:txt title="Event Queue Limit"        name="eventQueueLimit"        type="int" onchanged="configChanged();"/>
                        <sm-et:txt title="Protocol Id"              name="protocol_id"            type="int" onchanged="configChanged();"/>
                        <sm-et:txt title="State Machines Count"     name="state_machines_count"   type="int" onchanged="configChanged();"/>
                        <sm-et:txt title="Ussd Transaction Timeout" name="ussdTransactionTimeout" type="int" onchanged="configChanged();"/>

                        <sm-et:txt title="bindWaitTimeout (sec)"    name="bindWaitTimeout"        type="int" onchanged="configChanged();"/>
                        <sm-et:txt title="connectionsPerIp"         name="connectionsPerIp"       type="int" onchanged="configChanged();"/>
                        <sm-et:txt title="ipBlockingTime (sec)"     name="ipBlockingTime"         type="int" onchanged="configChanged();"/>
                        <sm-et:txt title="maxRWMultiplexersCount"   name="maxRWMultiplexersCount" type="int" onchanged="configChanged();"/>
                        <sm-et:txt title="socketsPerMultiplexer"    name="socketsPerMultiplexer"  type="int" onchanged="configChanged();"/>
                    </sm-ep:properties>
                </sm-et:section>
            </td></tr>
        </sm-ep:properties>
    </sm-et:section>

    <sm-et:section title="Http Transport" name="HttpTransport" type="table" >
        <sm-ep:properties id="HttpTransport" display="none">
            <sm-et:txt name="host" type="string" onchanged="configChanged();"/>
            <sm-et:txt name="port" type="int" validation="port" onchanged="configChanged();"/>
            <sm-et:txt title="Reader Sockets" name="readerSockets" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Writer Sockets" name="writerSockets" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Reader Pool Size" name="readerPoolSize" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Writer Pool Size" name="writerPoolSize" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Scag Pool Size" name="scagPoolSize" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Scag Queue Limit" name="scagQueueLimit" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Connection Timeout" name="connectionTimeout" type="int" onchanged="configChanged();"/>
        </sm-ep:properties>
    </sm-et:section>

    <sm-et:section title="Personalization" name="Personalization" type="table" >
        <sm-ep:properties id="Personalization" display="none">
            <sm-et:txt name="host"                    type="string" onchanged="configChanged();"/>
            <sm-et:txt name="port"  validation="port" type="int"    onchanged="configChanged();"/>
            <sm-et:txt name="pingTimeout"             type="int"    onchanged="configChanged();"/>
            <sm-et:txt name="ioTimeout"               type="int"    onchanged="configChanged();"/>
            <sm-et:txt name="reconnectTimeout"        type="int"    onchanged="configChanged();"/>
            <sm-et:txt name="maxWaitingRequestsCount" type="int"    onchanged="configChanged();"/>
            <sm-et:txt name="connections"             type="int"    onchanged="configChanged();"/>
        </sm-ep:properties>
    </sm-et:section>

    <sm-et:section title="Long Call Manager" name = "LongCallManager" type="table" >
<%--        <sm-et:properties>--%>
<%--        <table id="LongCallManager" style="display:none">--%>
        <sm-ep:properties id="LongCallManager" display="none">
            <sm-et:txt name="maxTasks" type="int" onchanged="configChanged();"/>
        </sm-ep:properties>
<%--        </table>        --%>
<%--        </sm-et:properties>--%>
    </sm-et:section>

</jsp:body>
</sm:page>