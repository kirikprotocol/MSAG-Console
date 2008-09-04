<%@ include file="/WEB-INF/inc/sme_menu.jsp" %>
<%
    sme_menu_begin(out);
    sme_menu_button(out, "mbMenu", "apply", "Home", "");
    sme_menu_button(out, "mbMenu", "options", "Options", "Global EMailSme options");
    sme_menu_button(out, "mbMenu", "profiles", "Profiles", "User profiles");
    sme_menu_button(out, "mbMenu", "stats", "Statistic", "Statistic");
    sme_menu_space(out);
    if (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0) {
        sme_menu_button(out, "mbStart", "Start", "Start", "Start EMailSme", "", !bean.isServiceOnline());
        sme_menu_button(out, "mbStop", "Stop", "Stop", "Stop EMailSme", "", bean.isServiceOnline());
    }
    sme_menu_end(out);
    if (ServiceIDForShowStatus != null && ServiceIDForShowStatus.length() > 0) {
%><script>
    function refreshEmailSmeStartStopButtonsStatus()
    {
        var status = document.getElementById('RUNNING_STATUSERVICE_<%=ServiceIDForShowStatus%>').innerText;
        document.getElementById('mbStart').disabled = (status != "<%= getLocString("common.statuses.offline") %>");
        document.getElementById('mbStop').disabled = (status == "<%= getLocString("common.statuses.offline") %>");
        window.setTimeout(refreshEmailSmeStartStopButtonsStatus, 5000);
    }
    refreshEmailSmeStartStopButtonsStatus();
</script><%
    }
%>
