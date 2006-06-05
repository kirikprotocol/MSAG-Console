<%@ include file="/WEB-INF/inc/header.jspf" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>

<sm:page title="Routes" onLoad="enableDisableButtons();">
    <jsp:attribute name="menu">
    </jsp:attribute>

    <jsp:body>
        <script>
            function loadSaved() {
                return "Date of saved file is ${bean.restoreDate}. Are you sure to load this file?";
            }
            function enableDisableButtonsById(itemId, isDisabled){

                var items = document.getElementsByName(itemId);

                for (var i = 0; i < items.length; i++) {
                    items[i].disabled = isDisabled;
                }
            }
            function enableDisableButtons(){
                enableDisableButtonsById('mbApply', ${(!bean.routesChanged)});
                enableDisableButtonsById('mbLoad', ${(!bean.routesRestored)} );
                enableDisableButtonsById('mbSave', false);
                enableDisableButtonsById('mbLoadSaved', ${(!bean.routesLoaded)});

                enableDisableButtonsById('mbApplyHttp', ${(!bean.httpRoutesChanged)});
                enableDisableButtonsById('mbLoadHttp', ${(!bean.httpRoutesRestored)});
                enableDisableButtonsById('mbSaveHttp', false);
                enableDisableButtonsById('mbLoadSavedHttp', ${(!bean.httpRoutesLoaded)});
            }
        </script>

        <table class=properties_list cellspacing=0 cellpadding=0>
            <col width="15%">
            <col width="100%">
            <c:set var="prop_rowN" value="0" scope="request"/>
            <sm-ep:list title="Transport" onChange="changeTransportId();" name="transportId"
                        values="${fn:join(bean.transportIds, ',')}"
                        valueTitles="${fn:join(bean.transportTitles, ',')}"/>
        </table>
        <br>

        <c:choose>
            <c:when test="${bean.transportId == 1}">
                <div class=page_subtitle>SMPP Routes status log</div>
                <sm:scrolling columns="time, userLogin, category, message"
                              names="time (GMT), user, category, description"
                              widths="30,10,20,40"/>
                <sm-pm:menu>
                    <sm-pm:item name="mbApply" value="Apply" title="Apply changes"/>
                    <sm-pm:item name="mbLoad" value="Restore Apply" title="Undo changes"/>
                    <sm-pm:space/>
                    <sm-pm:item name="mbSave" value="Save current" title="Save current routing configuration"/>
                    <sm-pm:item name="mbLoadSaved" value="Load saved" title="Load saved routing configuration"
                                onclick="confirm(loadSaved());"/>
                </sm-pm:menu>
            </c:when>
            <c:when test="${bean.transportId == 2}">
                <div class=page_subtitle>HTTP Routes status log</div>
                <sm:scrolling columns="time, userLogin, category, message"
                              names="time (GMT), user, category, description"
                              widths="30,10,20,40"/>
                <sm-pm:menu>
                    <sm-pm:item name="mbApplyHttp" value="Apply" title="Apply changes"/>
                    <sm-pm:item name="mbLoadHttp" value="Restore Apply" title="Undo changes"/>
                    <sm-pm:space/>
                    <sm-pm:item name="mbSaveHttp" value="Save current" title="Save current routing configuration"/>
                    <sm-pm:item name="mbLoadSavedHttp" value="Load saved" title="Load saved routing configuration"
                                onclick="confirm(loadSaved());"/>
                </sm-pm:menu>
            </c:when>
        </c:choose>
        <div class=page_subtitle>&nbsp;</div>
    </jsp:body>
</sm:page>