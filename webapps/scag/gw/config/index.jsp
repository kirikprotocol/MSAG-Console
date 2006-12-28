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
            var items = opForm.all[itemId];
            for (var i = 0; i < items.length; i++) {
                items[i].disabled = isDisabled;
            }
        }

        function configChanged() {
            enableDisableByIdFunction('mbSave', false);
        }

        function enableDisableStartStopButtonsForSCAGStatusPage() {

            if (document.all.SCAGStatusSpan.innerText == ' <fmt:message>status.running</fmt:message>') {
                enableDisableByIdFunction('mbStart', document.all.SCAGStatusSpan.innerText == ' <fmt:message>status.running</fmt:message>');
                enableDisableByIdFunction('mbStop', document.all.SCAGStatusSpan.innerText == ' <fmt:message>status.stopped</fmt:message>');
            } else if (document.all.SCAGStatusSpan.innerText == ' <fmt:message>status.stopped</fmt:message>' || document.all.SCAGStatusSpan.innerText == ' <fmt:message>status.stopping</fmt:message>') {
                enableDisableByIdFunction('mbStop', document.all.SCAGStatusSpan.innerText == ' <fmt:message>status.stopped</fmt:message>');
                enableDisableByIdFunction('mbStart', document.all.SCAGStatusSpan.innerText == ' <fmt:message>status.running</fmt:message>');
            } else if (document.all.SCAGStatusSpan.innerText == ' <fmt:message>status.unknown</fmt:message>') {
                enableDisableByIdFunction('mbStop', document.all.SCAGStatusSpan.innerText == ' <fmt:message>status.unknown</fmt:message>');
                enableDisableByIdFunction('mbStart', document.all.SCAGStatusSpan.innerText == ' <fmt:message>status.unknown</fmt:message>');
            }
        }
    </script>
    <sm-et:section title="Billing Manager" name="BillingManager">
        <sm-et:properties>
            <sm-et:txt title="Config Dir" name="configDir" validation="nonEmpty" onchanged="configChanged();"/>
            <sm-et:txt title="Billing Server Host" name="BillingServerHost" validation="nonEmpty"
                       onchanged="configChanged();"/>
            <sm-et:txt title="Billing Server Port" name="BillingServerPort" validation="port"
                       onchanged="configChanged();"/>
            <sm-et:txt title="Billing Time Out" name="BillingTimeOut" validation="nonEmpty"
                       onchanged="configChanged();"/>
        </sm-et:properties>
    </sm-et:section>

    <sm-et:section title="Session Manager" name="SessionManager">
        <sm-et:properties>
            <sm-et:txt title="Location" name="location" type="string" onchanged="configChanged();"/>
            <sm-et:txt title="Expire Interval" name="expireInterval" type="int" onchanged="configChanged();"/>
        </sm-et:properties>
    </sm-et:section>

    <sm-et:section title="Statistics Manager" name="StatisticsManager">
        <sm-et:properties>
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
        </sm-et:properties>
    </sm-et:section>

    <sm-et:section title="Rule Engine" name="RuleEngine">
        <sm-et:properties>
            <sm-et:txt title="Location" name="location" type="string" onchanged="configChanged();"/>
        </sm-et:properties>
    </sm-et:section>

    <sm-et:section title="Administration" name="admin">
        <sm-et:properties>
            <sm-et:txt name="host" type="string" onchanged="configChanged();"/>
            <sm-et:txt name="port" type="int" validation="port" onchanged="configChanged();"/>
        </sm-et:properties>
    </sm-et:section>

    <sm-et:section title="SMPP" name="smpp">
        <sm-et:properties>
            <sm-et:txt title="Host" name="host" type="string" onchanged="configChanged();"/>
            <sm-et:txt title="Port" name="port" type="int" validation="port" onchanged="configChanged();"/>
            <sm-et:txt title="Inactivity Time" name="inactivityTime" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Inactivity Time Out" name="inactivityTimeOut" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Read Timeout" name="readTimeout" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Max Sms PerSecond" name="maxSmsPerSecond" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Transit Optional Tags" name="transitOptionalTags" type="string" validation="transitOptionalTags" onchanged="configChanged();" comments="config.section.txt.transit_optional_tags.comments"/>
        </sm-et:properties>
        <sm-et:section title="Core" name="core">
            <sm-et:properties>
                <sm-et:txt title="Event Queue Limit" name="eventQueueLimit" type="int" onchanged="configChanged();"/>
                <sm-et:txt title="Protocol Id" name="protocol_id" type="int"/>
                <sm-et:txt title="State Machines Count" name="state_machines_count" type="int"
                           onchanged="configChanged();"/>
                <sm-et:txt title="Ussd Transaction Timeout" name="ussdTransactionTimeout" type="int"
                           onchanged="configChanged();"/>
            </sm-et:properties>
        </sm-et:section>
    </sm-et:section>

    <sm-et:section title="Http Transport" name="HttpTransport">
        <sm-et:properties>
            <sm-et:txt name="host" type="string" onchanged="configChanged();"/>
            <sm-et:txt name="port" type="int" validation="port" onchanged="configChanged();"/>
            <sm-et:txt title="Reader Sockets" name="readerSockets" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Writer Sockets" name="writerSockets" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Reader Pool Size" name="readerPoolSize" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Writer Pool Size" name="writerPoolSize" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Scag Pool Size" name="scagPoolSize" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Scag Queue Limit" name="scagQueueLimit" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Connection Timeout" name="connectionTimeout" type="int" onchanged="configChanged();"/>
        </sm-et:properties>
    </sm-et:section>

    <sm-et:section title="Personalization" name="Personalization">
        <sm-et:properties>
            <sm-et:txt name="host" type="string" onchanged="configChanged();"/>
            <sm-et:txt name="port" type="int" validation="port" onchanged="configChanged();"/>
            <sm-et:txt title="Timeout" name="timeout" type="int" onchanged="configChanged();"/>
            <sm-et:txt title="Ping Timeout" name="pingTimeout" type="int" onchanged="configChanged();"/>
        </sm-et:properties>
    </sm-et:section>

    <sm-et:section title="Long Call Manager" name = "LongCallManager">
        <sm-et:properties>
            <sm-et:txt name="maxTasks" type="int" onchanged="configChanged();"/>
        </sm-et:properties>
    </sm-et:section>

</jsp:body>
</sm:page>