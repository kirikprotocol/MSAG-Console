<%@ include file="/WEB-INF/inc/header.jspf" %>

<script src="tree.js"></script>

<sm:page title="Edit SCAG configuration" onLoad="enableDisableStartStopButtonsForSCAGStatusPage();">
<jsp:attribute name="menu">
    <sm-pm:menu>
        <sm-pm:item name="mbSave" value="Save" enabled="false" title="Save conig"/>
        <sm-pm:item name="mbCancel" value="Cancel" title="Cancel config editing" onclick="clickCancel()"/>
        <sm-pm:space/>
        <sm-pm:item name="mbStart" value="Start" title="Start SCAG" isCheckSensible="false"/>
        <sm-pm:item name="mbStop" value="Stop" title="Stop SCAG" isCheckSensible="false"/>
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

            if (document.all.SCAGStatusSpan.innerText == ' running') {
                enableDisableByIdFunction('mbStart', document.all.SCAGStatusSpan.innerText == ' running');
                enableDisableByIdFunction('mbStop', document.all.SCAGStatusSpan.innerText == ' stopped');
            } else if (document.all.SCAGStatusSpan.innerText == ' stopped' || document.all.SCAGStatusSpan.innerText == ' stopping') {
                enableDisableByIdFunction('mbStop', document.all.SCAGStatusSpan.innerText == ' stopped');
                enableDisableByIdFunction('mbStart', document.all.SCAGStatusSpan.innerText == ' running');
            } else if (document.all.SCAGStatusSpan.innerText == ' unknown') {
                enableDisableByIdFunction('mbStop', document.all.SCAGStatusSpan.innerText == ' unknown');
                enableDisableByIdFunction('mbStart', document.all.SCAGStatusSpan.innerText == ' unknown');
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
        </sm-et:properties>
    </sm-et:section>

</jsp:body>
</sm:page>