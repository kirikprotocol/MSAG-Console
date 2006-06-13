<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="status.title" onLoad="enableDisableStartStopButtonsForSCAGStatusPage();">
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:space/>
            <sm-pm:item name="mbStart" value="status.item.mbstart.value" title="status.item.mbstart.title" isCheckSensible="false"/>
            <sm-pm:item name="mbStop" value="status.item.mbstop.value" title="status.item.mbstop.title" isCheckSensible="false"/>
        </sm-pm:menu>
    </jsp:attribute>

    <jsp:body>
        <div class=page_subtitle><fmt:message>status.label.status_log</fmt:message></div>
        <sm:scrolling columns="time, userLogin, category, message" names="status.scrolling.names.time, status.scrolling.names.user, status.scrolling.names.category, status.scrolling.names.description"
                      widths="30,10,20,40"/>
        <div class=page_subtitle>&nbsp;</div>
        <script>
            function enableDisableByIdFunction(itemId, isDisabled) {
                var items = opForm.all[itemId];
                for (var i = 0; i < items.length; i++) {
                    items[i].disabled = isDisabled;
                }
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
                window.setTimeout(enableDisableStartStopButtonsForSCAGStatusPage, 1000);
            }
        </script>
    </jsp:body>
</sm:page>