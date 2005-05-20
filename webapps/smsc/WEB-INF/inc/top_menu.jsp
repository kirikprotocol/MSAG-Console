<%@ page import="java.io.IOException,
                 java.util.Collection,
                 ru.novosoft.util.menu.MenuItem,
                 java.util.Iterator,
                 ru.novosoft.util.menu.TopMenu,
                 ru.novosoft.util.menu.Menu,
                 ru.novosoft.smsc.util.LocaleMessages,
                 ru.novosoft.util.menu.Common"%>
<jsp:useBean id="topMenu" class="ru.novosoft.util.menu.TopMenu" scope="session"/>

<!--link rel="stylesheet" type="text/css" href="<%=request.getContextPath()%>/styles/main_menu.css" /-->

<script type="text/javascript" language="javascript" src="/scripts/menu/jsdomenu.js"></script>
<script type="text/javascript" language="javascript" src="/scripts/menu/jsdomenubar.js"></script>

<SCRIPT type="text/javascript">
  <%renderTopMenu(request, out, "createjsDOMenu", getTopMenu());%>
  <%setBrowser(session, request);%>
</SCRIPT>

<%!

  void renderTopMenu(HttpServletRequest request, JspWriter out, String menuName, Collection menu) throws IOException{

    Common.messages = LocaleMessages.getResourceBundle(getLoc());

    out.println("function "+menuName+"() {");
      out.println("menuBar = new jsDOMenuBar(); //CREATE MAIN MENU ITEMS");

      int i = 0;
      for(Iterator it = menu.iterator(); it.hasNext();i++){
        MenuItem menuItem = (MenuItem)it.next();
        String menuItemName = "menu_"+i;
        if(menuItem.getSubMenu() != null){
          createMenu(request, out, menuItemName, ""+menuItem.getSubMenuWidth(), menuItem.getSubMenu());
          out.println("menuBar.addMenuBarItem(new menuBarItem(\""+menuItem.getCaption()+"\", "+menuItemName+"));");
        }
      }
    out.println("}");
  }

 Collection getTopMenu() throws IOException{
    return Menu.getInstance().getMenu();
 }
  /**
   * Method for set brawser type in the session
   * @param session current session
   * @param request current request
   */
  public void setBrowser(HttpSession session, HttpServletRequest request){
      String brawserType = null;

      Cookie[] cookies = request.getCookies();
      if(cookies != null)
      for (int i = 0; i < cookies.length; i++) {
          if(cookies[i].getName().equals("BRAWSER_TYPE")){
                 brawserType = cookies[i].getValue();
          }
      }
      session.setAttribute("BRAWSER_TYPE", brawserType);
  }

  void createMenu(HttpServletRequest request, JspWriter out, String menuName, String msize, Collection menu) throws IOException{
    out.println(menuName+"=new jsDOMenu("+msize+");");
//    out.println(menuName+"=new jsDOMenu(100);");
    int i = 0;
    for(Iterator it = menu.iterator(); it.hasNext();i++){
      MenuItem menuItem = (MenuItem)it.next();
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