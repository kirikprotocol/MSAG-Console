
<jsp:useBean id="topMenu" class="ru.sibinco.scag.beans.menu.TopMenu" scope="session"/>

<!--link rel="stylesheet" type="text/css" href="<%=request.getContextPath()%>/styles/main_menu.css" /-->

<script type="text/javascript" language="javascript" src="/scripts/menu/jsdomenu.js"></script>
<script type="text/javascript" language="javascript" src="/scripts/menu/jsdomenubar.js"></script>

<SCRIPT type="text/javascript">
  <%renderTopMenu(session, request, out, "createjsDOMenu", getTopMenu());%>
</SCRIPT>

<%!

  void renderTopMenu(HttpSession session, HttpServletRequest request, JspWriter out, String menuName, java.util.Collection menu) throws java.io.IOException{
    setBrowser(session, request);
    out.println("function " + menuName + "() {");
      out.println("menuBar = new jsDOMenuBar(); //CREATE MAIN MENU ITEMS");

      int i = 0;
      for(java.util.Iterator it = menu.iterator(); it.hasNext();i++){
        ru.sibinco.scag.beans.menu.MenuItem menuItem = (ru.sibinco.scag.beans.menu.MenuItem)it.next();
        String menuItemName = "menu_"+i;
        if(menuItem.getSubMenu() != null){
          createMenu(request, out, menuItemName, ""+menuItem.getSubMenuWidth(), menuItem.getSubMenu());
          out.println("menuBar.addMenuBarItem(new menuBarItem(\"" + menuItem.getCaption() + "\", " + menuItemName + "));");
        }
      }
    out.println("}");
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

  void createMenu(HttpServletRequest request, JspWriter out, String menuName, String msize, java.util.Collection menu) throws java.io.IOException{
    out.println(menuName+"=new jsDOMenu("+msize+");");
//    out.println(menuName+"=new jsDOMenu(100);");
    int i = 0;
    for(java.util.Iterator it = menu.iterator(); it.hasNext();i++){
      ru.sibinco.scag.beans.menu.MenuItem menuItem = (ru.sibinco.scag.beans.menu.MenuItem)it.next();
      if(menuItem.getSubMenu() == null){
        String url = menuItem.getUri();
          if(menuItem.getUri().equals("")){
             out.println(menuName+".addMenuItem(new menuItem(\""+menuItem.getCaption()+"\", \"\", \""+ "code:window.open('" + request.getContextPath()+menuItem.getTarget()+"\" ));");
          }else{
            out.println(menuName+".addMenuItem(new menuItem(\""+menuItem.getCaption()+"\", \"\", \""+request.getContextPath()+((url != null)?url:"")+"\" ));");
          }
      }else{
        String subMenuName = menuName+"_"+i;
        createMenu(request, out, subMenuName, ""+menuItem.getSubMenuWidth(), menuItem.getSubMenu());
        out.println(menuName+".addMenuItem(new menuItem(\""+menuItem.getCaption()+"\", \""+("item"+i)+"\", \"\" ));");
        out.println(menuName+".items."+("item"+i)+".setSubMenu("+subMenuName+");");
      }
    }
  }

%>