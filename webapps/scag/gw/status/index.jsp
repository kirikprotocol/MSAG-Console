<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="Status" onLoad="enableDisableStartStopButtonsForSCAGStatusPage();">
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:space/>
            <sm-pm:item name="mbStart" value="Start" title="Start SCAG" isCheckSensible="false"/>
            <sm-pm:item name="mbStop" value="Stop" title="Stop SCAG" isCheckSensible="false"/>
        </sm-pm:menu>
    </jsp:attribute>

    <jsp:body>
        <div class=page_subtitle>Status log</div>
        <sm:scrolling columns="time, userLogin, category, message" names="time (GMT), user, category, description"
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
                window.setTimeout(enableDisableStartStopButtonsForSCAGStatusPage, 1000);
            }
        </script>
    </jsp:body>
</sm:page>