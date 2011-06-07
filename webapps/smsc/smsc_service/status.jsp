<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.admin.journal.Action,
                 ru.novosoft.smsc.admin.journal.Actions,
                 ru.novosoft.smsc.admin.journal.SubjectTypes,
                 ru.novosoft.smsc.jsp.smsc.Index,
                 ru.novosoft.smsc.util.SortedList,
                 java.text.DateFormat,
                 java.util.List" %>
<%!
    void printJournalEntries(JspWriter out, HttpServletRequest request, List journalEntries) throws IOException {
        out.println("<table border=1>");
        out.println("<col width='10%'>");
        //timestamp
        out.println("<col width='10%'>");
        //user
        //out.println("<col width='10%'>"); //session ID
        out.println("<col width='10%'>");
        //subject type
        out.println("<col width='25%'>");
        //subject ID
        out.println("<col width='10%'>");
        //action
        out.println("<col width='10%'>");
        //additional key
        out.println("<col width='25%'>");

        //additional value
        DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.MEDIUM, getLoc());

        for (Iterator i = journalEntries.iterator(); i.hasNext();) {
            Action action = (Action) i.next();
            final String rowspan = action.getAdditionalKeys().size() > 0
                    ? " rowspan=" + action.getAdditionalKeys().size()
                    : "";

            out.println("<tr>");
            out.println("<td" + rowspan + " nowrap>" + StringEncoderDecoder.encode(dateFormat.format(action.getTimestamp())) +
                    "</td>");
            out.println("<td" + rowspan + ">" + StringEncoderDecoder.encode(action.getUser()) + "</td>");
            //out.println("<td" + rowspan + ">" + StringEncoderDecoder.encode(action.getSessionId()) + "</td>");
            out.println("<td" + rowspan + ">" + StringEncoderDecoder.encode(SubjectTypes.typeToString(action.getSubjectType())) +
                    "</td>");
            out.println("<td" + rowspan + ">" + StringEncoderDecoder.encode(action.getSubjectId()) + "</td>");
            out.println("<td" + rowspan + ">" + StringEncoderDecoder.encode(Actions.actionToString(action.getAction())) + "</td>");

            if (action.getAdditionalKeys().size() > 0) {
                boolean firstRow = true;

                for (Iterator j = new SortedList(action.getAdditionalKeys()).iterator(); j.hasNext();) {
                    String additionalKey = (String) j.next();
                    String additionalValue = action.getAdditionalValue(additionalKey);

                    out.println((firstRow
                            ? ""
                            : "<tr>") + "<td align=right>" + StringEncoderDecoder.encode(additionalKey) + ":</td>\n<td>" +
                            StringEncoderDecoder.encode(additionalValue) + "</td>");

                    firstRow = false;
                }
            } else {
                out.println("<td>&nbsp;</td><td>&nbsp;</td>");
            }

            out.println("</tr>");
        }

        out.println("</table>");
    }

    void printOptionsString(JspWriter out, HttpServletRequest request, List journalEntries, int rowN, String checkboxValue,
                            boolean optionsChanged, String name) throws IOException {
        String journalRowId = checkboxValue + "JournalRow";

        out.println("<tr class=row" + (rowN & 1) + ">");

        final boolean isJournalHasEntries = journalEntries.size() > 0;

        out.println("  <td rowspan=1" + " valign=top>" + "<input class=check type=checkbox name=checks value=" + checkboxValue + ((!optionsChanged && !
                isJournalHasEntries)
                ? " disabled"
                : "") + ">" + "</td>");

        String onClick = isJournalHasEntries
                ? " class=clickable onClick='click" + journalRowId + "()'"
                : "";

        out.println("  <td " + onClick + "><div id=\"" + journalRowId + "_div\"" + (isJournalHasEntries
                ? "class=collapsing_list_closed"
                : "class=collapsing_list_empty") + ">" + getLocString(name) + "</div></td>");
        out.println("  <td " + onClick + ">" + (optionsChanged
                ? "<span class=Cf00>" + getLocString("common.util.changed") + "</span>"
                : getLocString("common.util.clear")) + "</td>");
        out.println("</tr>");

        if (isJournalHasEntries) {
            out.println("<tr id=" + journalRowId + "><td>&nbsp;</td><td colspan=3>");
            printJournalEntries(out, request, journalEntries);
            out.println("</td></tr>");
            out.println("<script>");
            out.println("function click" + journalRowId + "() {");
            out.println("  if (" + journalRowId + ".style.display == 'none') {");
            out.println("    " + journalRowId + ".style.display = '';");
            out.println("    " + journalRowId + "_div.className = 'collapsing_list_opened';");
            out.println("  } else {");
            out.println("    " + journalRowId + ".style.display = 'none';");
            out.println("    " + journalRowId + "_div.className = 'collapsing_list_closed';");
            out.println("  }");
            out.println("}");
            out.println(journalRowId + ".style.display = 'none';</script>");
        }
    }

%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
    ServiceIDForShowStatus = Constants.SMSC_SME_ID;

    if (request.isUserInRole("super-admin")) {
        TITLE = getLocString("home.confStatusTitle");
    }

    switch (bean.process(request)) {
        case Index.RESULT_DONE :
            response.sendRedirect(CPATH + "/index.jsp");
            return;
        case Index.RESULT_OK :
            //
            break;
        case Index.RESULT_ERROR :
            break;
    }

    MENU0_SELECTION = "MENU0_SMSC_Status";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbApply", "common.buttons.apply", "common.buttons.apply");
    page_menu_space(out);
    page_menu_end(out);
%>
<div class="content">
    <%
        if (request.isUserInRole("apply")) {
    %>
    <table cellspacing=0 cellpadding=0 class=list>
        <col width=1%>
        <col width="70%">
        <col width="30%" align=center>
        <col width="1%">
        <thead>
            <tr>
                <th>
                    &nbsp;
                </th>
                <th>
                    <%= getLocString("menu.smsc.configuration") %>
                </th>
                <th>
                    <%= getLocString("menu.smsc.status") %>
                </th>
            </tr>
        </thead>
        <tbody>
            <%
                int rowN = 0;

                if (request.isUserInRole("routes")) {
                    printOptionsString(out, request, bean.getJournalRoutes(), rowN++, "routes", bean.isRoutesChanged(),
                            "users.roles.routes");
                }
                
                if (request.isUserInRole("smsc_service")) {
                    printOptionsString(out, request, bean.getJournalSmsc(), rowN++, "smsc", bean.isSmscChanged(),
                            "users.roles.smscServ");
                }
                

                if (request.isUserInRole("subjects")) {
                    printOptionsString(out, request, bean.getJournalSubjects(), rowN++, "subjects", bean.isSubjectsChanged(),
                            "users.roles.subjects");
                }

/*                if (request.isUserInRole("aliases")) {
                    printOptionsString(out, request, bean.getJournalAliases(), rowN++, "aliases", bean.isAliasesChanged(),
                            "users.roles.aliases");
                }*/

                //if (request.isUserInRole("profiles"))      printOptionsString(out, request, bean.getJournalProfiles(), rowN++, "profiles", bean.isProfilesChanged(), "users.rules.profiles");
                if (request.isUserInRole("hosts")) {
                    printOptionsString(out, request, bean.getJournalHosts(), rowN++, "hosts", bean.isHostsChanged(),
                            "users.roles.hosts");
                }

                //if (request.isUserInRole("services"))      printOptionsString(out, request, bean.getJournalServices(), rowN++, "services", bean.isServicesChanged(), "users.roles.services");
                if (request.isUserInRole("users")) {
                    printOptionsString(out, request, bean.getJournalUsers(), rowN++, "users", bean.isUsersChanged(),
                            "users.roles.users");
                }

                if (request.isUserInRole("providers")) {
                    printOptionsString(out, request, bean.getJournalProviders(), rowN++, "providers", bean.isProvidersChanged(),
                            "users.roles.providers");
                }

                if (request.isUserInRole("categories")) {
                    printOptionsString(out, request, bean.getJournalCategories(), rowN++, "categories", bean.isCategoriesChanged(),
                            "users.roles.categories");
                }

                if (request.isUserInRole("snmp")) {
                    printOptionsString(out, request, bean.getJournalSnmp(), rowN++, "snmp", bean.isSNMPChanged(),
                            "users.roles.snmp");
                }

                //todo security-constraint ��� reschedule  if (request.isUserInRole("reschedule"))
                if (Constants.instMode == Constants.INST_MODE_SMSC)
                  printOptionsString(out, request, bean.getJournalReschedule(), rowN++, "reschedule", bean.isRescheduleChanged(),
                        "users.roles.reschedule");
            %>
        </tbody>
    </table>
    <%
        }
    %>
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbApply", "common.buttons.apply", "common.buttons.apply");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>
