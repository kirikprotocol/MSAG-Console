<%@ include file="/WEB-INF/inc/header.jspf" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>

<sm:page title="routes_status.title" onLoad="enableDisableButtons();">
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
                enableDisableButtonsById('mbLoad',  ${(bean.routesRestored)} );
                enableDisableButtonsById('mbSave',  false);
                enableDisableButtonsById('mbLoadSaved', ${(!bean.routesLoaded)});

                enableDisableButtonsById('mbApplyHttp', ${(!bean.httpRoutesChanged)});
                enableDisableButtonsById('mbLoadHttp',  ${(!bean.httpRoutesRestored)});
                enableDisableButtonsById('mbSaveHttp',  false);
                enableDisableButtonsById('mbLoadSavedHttp', ${(!bean.httpRoutesLoaded)});
            }

            function changeTransportId() {
//                var transport = opForm.all.transportId.options[opForm.all.transportId.selectedIndex].value;
                var opForm = getElementByIdUni("opForm");
                var transportId = getElementByIdUni("transportId");
                var transport = transportId.options[transportId.selectedIndex].value;
                if (location.href.indexOf("transportId")!=-1) {
                  var action = location.href.substring(0,location.href.indexOf("transportId")-1);
                  opForm.action=action;
                }
                opForm.submit();
                return true;
            }
        </script>

        <table class=properties_list cellspacing=0 cellpadding=0>
            <col width="15%">
            <col width="100%">
            <c:set var="prop_rowN" value="0" scope="request"/>
            <sm-ep:list title="routes_status.list.transportid" onChange="changeTransportId();" name="transportId"
                        values="${fn:join(bean.transportIds, ',')}"
                        valueTitles="${fn:join(bean.transportTitles, ',')}"/>
        </table>
        <br>

        <c:choose>
            <c:when test="${bean.transportId == 1}">
                <div class=page_subtitle><fmt:message>routes_status.label.smpp</fmt:message></div>
                <sm:scrolling columns="time, userLogin, category, message"
                              names="routes_status.scrolling.names.time, routes_status.scrolling.names.userlogin, routes_status.scrolling.names.category, routes_status.scrolling.names.message"
                              widths="30,10,20,40"/>
                <sm-pm:menu>
                    <sm-pm:item name="mbApply" value="routes_status.item.mbapply.value" title="routes_status.item.mbapply.title"/>
                    <sm-pm:item name="mbLoad" value="routes_status.item.mbload.value" title="routes_status.item.mbload.title"/>
                    <sm-pm:space/>
                    <sm-pm:item name="mbSave" value="routes_status.item.mbsave.value" title="routes_status.item.mbsave.title"/>
                    <sm-pm:item name="mbLoadSaved" value="routes_status.item.mbloadsaved.value" title="routes_status.item.mbloadsaved.title"
                                onclick="confirm(loadSaved());"/>
                </sm-pm:menu>
            </c:when>
            <c:when test="${bean.transportId == 2}">
                <div class=page_subtitle><fmt:message>routes_status.label.http</fmt:message></div>
                <sm:scrolling columns="time, userLogin, category, message"
                              names="routes_status.scrolling.names.time, routes_status.scrolling.names.userlogin, routes_status.scrolling.names.category, routes_status.scrolling.names.message"
                              widths="30,10,20,40"/>
                <sm-pm:menu>
                    <sm-pm:item name="mbApplyHttp" value="routes_status.item.mbapply.value" title="routes_status.item.mbapply.title"/>
                    <sm-pm:item name="mbLoadHttp" value="routes_status.item.mbload.value" title="routes_status.item.mbload.title"/>
                    <sm-pm:space/>
                    <sm-pm:item name="mbSaveHttp" value="routes_status.item.mbsave.value" title="routes_status.item.mbsave.title"/>
                    <sm-pm:item name="mbLoadSavedHttp" value="routes_status.item.mbloadsaved.value" title="routes_status.item.mbloadsaved.title"
                                onclick="confirm(loadSaved());"/>
                </sm-pm:menu>
            </c:when>
        </c:choose>
        <div class=page_subtitle>&nbsp;</div>
    </jsp:body>
</sm:page>