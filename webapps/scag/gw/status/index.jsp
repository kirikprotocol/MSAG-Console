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
        <table class="list" cellspacing="0">
            <col width="1px"/>
            <tr class="row0">
                <td><input class="check" type="checkbox" name="subj" id="chk_billing" value="billing"
                <c:if test="${!bean.billingChanged}"> disabled</c:if>></td>
                <td><label for="chk_billing">Route billing rules</label></td><td><sm:status
                    changed="${bean.billingChanged}"/></td>
            </tr>
        </table>
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