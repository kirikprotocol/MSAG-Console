<%@ include file="/WEB-INF/inc/header.jspf" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>

<sm:page title="Routes" onLoad="enableDisableButtons();">
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbAdd" value="Add" title="Add new route"/>
            <sm-pm:item name="mbDelete" value="Delete" title="Delete route" onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
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
            <sm-pm:item name="mbApply" value="Apply" title="Apply changes"/>
            <sm-pm:item name="mbLoad" value="Restore" title="Undo changes"/>
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
                <sm:table columns="checkbox,name,providerName,categoryName,active,enabled,archived,notes"
                          names="c,name,provider,category,active,enabled,archived,notes" widths="1,39,39,20,20,20,39,39"
                          edit="name"/>
                <c:choose>
                    <c:when test="${bean.currentUser}">
                    </c:when>
                    <c:otherwise>
                    </c:otherwise>
                </c:choose>
            </c:when>
            <c:otherwise>
                <sm:table columns="checkbox,name,providerName,categoryName,active,enabled,archived,notes"
                          names="c,name,provider,category,active,enabled,archived,notes" widths="1,39,39,20,20,20,39,39"
                          edit="name"/>
            </c:otherwise>
        </c:choose>
    </jsp:body>
</sm:page>