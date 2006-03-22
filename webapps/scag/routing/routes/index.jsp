<%@ include file="/WEB-INF/inc/header.jspf" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>

<sm:page title="Routes" onLoad="enableDisableButtons();">
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbApply" value="Apply" title="Apply changes"/>
            <sm-pm:item name="mbLoad" value="Restore" title="Undo changes"/>
            <sm-pm:space/>
            <sm-pm:space/>
            <sm-pm:space/>
            <sm-pm:space/>
            <sm-pm:space/>
            <sm-pm:space/>
            <sm-pm:space/>
            <sm-pm:space/>
            <sm-pm:space/>
            <sm-pm:item name="mbSave" value="Save current" title="Save current routing configuration"/>
            <sm-pm:item name="mbLoadSaved" value="Load saved" title="Load saved routing configuration"
                        onclick="confirm(loadSaved());"/>
            <sm-pm:space/>
        </sm-pm:menu>
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
                enableDisableButtonsById('mbSave', ${(!bean.routesSaved)});
                enableDisableButtonsById('mbLoadSaved', ${(!bean.routesLoaded)});
                enableDisableButtonsById('mbApply', ${(!bean.routesChanged)});
                enableDisableButtonsById('mbLoad', ${(!bean.routesRestored)} );
            }
        </script>
        <c:choose>
            <c:when test="${bean.routesChanged}">
                <table class="list" cellspacing="0">
                    <col width="1px"/>
                    <tr class="row1">
                        <td>
                        </td>
                        <td><label>Routes and subjects</label>
                        </td>
                        <td>
                            <span style="color:red">changed by user:</span> ${fn:escapeXml(bean.changeByUser)}
                        </td>
                    </tr>
                </table>
                <br/>
            </c:when>
            <c:otherwise>
            </c:otherwise>
        </c:choose>
    </jsp:body>
</sm:page>