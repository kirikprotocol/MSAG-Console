<%@ tag import="ru.sibinco.scag.web.security.AuthFilter" %>
<%@ tag import="ru.sibinco.scag.web.security.UserLoginData" %>
<%--<jsp:useBean id="topMenu" class="ru.sibinco.scag.beans.menu.TopMenu" scope="session"/>--%>

<!--link rel="stylesheet" type="text/css" href="<%=request.getContextPath()%>content/styles/main_menu.css" /-->

<script type="text/javascript" language="javascript" src="content/scripts/menu/jsdomenu.js"></script>
<script type="text/javascript" language="javascript" src="content/scripts/menu/jsdomenubar.js"></script>

<SCRIPT type="text/javascript">
  <%renderTopMenu(session, request, out, "createjsDOMenu", getTopMenu());%>
</SCRIPT>

<%!
  void renderTopMenu(HttpSession session, HttpServletRequest request, JspWriter out, String menuName, java.util.Collection menu) throws java.io.IOException{
    java.util.ResourceBundle rb = ru.sibinco.scag.util.LocaleMessages.getInstance().getBundle(session);
    setBrowser(session, request);
    out.println("function " + menuName + "() {");
      out.println("menuBar = new jsDOMenuBar(); //CREATE MAIN MENU ITEMS");

      int i = 0;
      for(java.util.Iterator it = menu.iterator(); it.hasNext();i++){
        ru.sibinco.scag.beans.menu.MenuItem menuItem = (ru.sibinco.scag.beans.menu.MenuItem)it.next();

        if (session != null && session.getAttribute(AuthFilter.USER_LOGIN_DATA) != null) {
          UserLoginData userLoginData = (UserLoginData) session.getAttribute(AuthFilter.USER_LOGIN_DATA);
          if (!isUserInRoleForMenu(menuItem, userLoginData)) continue;
        }

        String menuItemName = "menu_"+i;
        if(menuItem.getSubMenu() != null){
          createMenu(request, out, menuItemName, ""+menuItem.getSubMenuWidth(rb), menuItem.getSubMenu(),rb);
          out.println("menuBar.addMenuBarItem(new menuBarItem(\"" + menuItem.getCaption(rb) + "\", " + menuItemName + "));");
        }
      }
    out.println("}");
  }

  boolean isUserInRoleForMenu(ru.sibinco.scag.beans.menu.MenuItem menuItem, UserLoginData userLoginData) {
          boolean isUserInRoleForMenu = false;
          String[] menuItemRoles =  menuItem.getRoles();
          for (int j = 0; j < menuItemRoles.length; j++) {
            if (userLoginData.isUserInRole(menuItemRoles[j])) {
              isUserInRoleForMenu = true;
              break;
            }
          }
          return isUserInRoleForMenu;
  }

 java.util.Collection getTopMenu() throws java.io.IOException{
    return ru.sibinco.scag.beans.menu.Menu.getInstance().getMenu();
 }
  /**
   * Method for set brawser type in the session
   * @param session current session
   * @param request current request
   */
  public void setBrowser(HttpSession session, HttpServletRequest request)throws java.io.IOException{
      String browserType = null;

      Cookie[] cookies = request.getCookies();
      if(cookies != null)
      for (int i = 0; i < cookies.length; i++) {
          if(cookies[i].getName().equals("BROWSER_TYPE")){
                 browserType = cookies[i].getValue();
          }
      }
      session.setAttribute("BROWSER_TYPE", browserType);
  }

  void createMenu(HttpServletRequest request, JspWriter out, String menuName, String msize, java.util.Collection menu, java.util.ResourceBundle rb) throws java.io.IOException{
    out.println(menuName+"=new jsDOMenu("+msize+");");
//    out.println(menuName+"=new jsDOMenu(100);");
    int i = 0;
    for(java.util.Iterator it = menu.iterator(); it.hasNext();i++){
      ru.sibinco.scag.beans.menu.MenuItem menuItem = (ru.sibinco.scag.beans.menu.MenuItem)it.next();

      HttpSession session = request.getSession();
      if (session != null && session.getAttribute(AuthFilter.USER_LOGIN_DATA) != null) {
        UserLoginData userLoginData = (UserLoginData) session.getAttribute(AuthFilter.USER_LOGIN_DATA);
          if (!isUserInRoleForMenu(menuItem, userLoginData)) continue;
      }

      if(menuItem.getSubMenu() == null){
        String url = menuItem.getUri();
          if(menuItem.getUri().equals("")){
             out.println(menuName+".addMenuItem(new menuItem(\""+menuItem.getCaption(rb)+"\", \"\", \""+ "code:window.open('" + request.getContextPath()+menuItem.getTarget()+"\" ));");
          }else{
            out.println(menuName+".addMenuItem(new menuItem(\""+menuItem.getCaption(rb)+"\", \"\", \""+request.getContextPath()+((url != null)?url:"")+"\" ));");
          }
      }else{
        String subMenuName = menuName+"_"+i;
        createMenu(request, out, subMenuName, ""+menuItem.getSubMenuWidth(rb), menuItem.getSubMenu(),rb);
        out.println(menuName+".addMenuItem(new menuItem(\""+menuItem.getCaption(rb)+"\", \""+("item"+i)+"\", \"\" ));");
        out.println(menuName+".items."+("item"+i)+".setSubMenu("+subMenuName+");");
      }
    }
  }

%>