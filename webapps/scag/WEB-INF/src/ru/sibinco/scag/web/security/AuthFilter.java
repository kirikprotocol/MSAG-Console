package ru.sibinco.scag.web.security;

import ru.sibinco.scag.web.WebContext;

import javax.servlet.*;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.io.IOException;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

public class AuthFilter implements Filter {

  protected static final org.apache.log4j.Category cat = org.apache.log4j.Category.getInstance(AuthFilter.class.getName());

  private Pattern passBy;
  public static final String USER_LOGIN_DATA = "userLoginData";
  public static final String PASSWORD = "password";
  public static final String USERNAME = "username";

  private String loginPage;
  private static String accessDeniedPage;
  private static String welcomePage;

  @Override
  public void init(FilterConfig config) throws ServletException {
    String s = config.getInitParameter("pass-by");
    if (s != null) {
      passBy = Pattern.compile(s);
      cat.info("uri's matches regular pattern " + s + " are unprotected");
    }

    loginPage = config.getInitParameter("login-page");
    if (loginPage == null) {
      cat.error("login-page is not defined in config");
      throw new ServletException("login-page is not defined in config");
    }

    accessDeniedPage = config.getInitParameter("access-denied-page");
    if (accessDeniedPage == null) {
      cat.error("access-denied-page is not defined in config");
      throw new ServletException("access-denied-page is not defined in config");
    }

    welcomePage = config.getInitParameter("welcome-page");
    if (welcomePage == null) {
      cat.error("welcome-page is not defined in config");
      throw new ServletException("welcome-page is not defined in config");
    }
  }

  @Override
  public void doFilter(ServletRequest request, ServletResponse response, FilterChain chain) throws IOException, ServletException {
    HttpServletRequest req = (HttpServletRequest) request;
    HttpServletResponse resp = (HttpServletResponse) response;
    HttpSession session = req.getSession();
    String uri = req.getRequestURI();
    String shortUri = uri.substring(req.getContextPath().length());
    if (cat.isDebugEnabled()) cat.debug("uri: '"+uri+"', shortUri: '"+shortUri+"'");

    if (passBy != null && passBy.matcher(shortUri).matches()) {
      //cat.debug("Resource with uri=" + shortUri + " allowed due to pass-by");
      chain.doFilter(request, response);
      return;
    }

    RoleMapper roleMapper = WebContext.getRoleMapper();

    UserLoginData userLoginData = (UserLoginData) session.getAttribute(USER_LOGIN_DATA);
    if (userLoginData != null) {
      if (cat.isDebugEnabled()) cat.debug("Found in session "+userLoginData+".");

      Set<String> userRoles = userLoginData.getRoles();
      Set<String> rolesAllowedForURI = roleMapper.getRolesAllowedForURI(shortUri);
      boolean uriPermitted = false;
      for(String userRole : userRoles) {
        if (rolesAllowedForURI.contains(userRole)) {
          if (cat.isDebugEnabled()) cat.debug("User role '"+userRole+"' is allowed for short uri '"+shortUri+"'.");
          uriPermitted = true;
          break;
        }
      }

      if(uriPermitted || shortUri.equals(welcomePage) || shortUri.equals("/")){
        chain.doFilter(req, resp);
        return;
      } else {
        request.getRequestDispatcher(accessDeniedPage).forward(request, response);
        cat.warn("Access to page uri='" + shortUri + "' has been denied.");
        return;
      }
    }

    if(shortUri.equals(welcomePage)){
      chain.doFilter(req, resp);
      return;
    }

    String username = request.getParameter(USERNAME);
    String password = request.getParameter(PASSWORD);
    if (cat.isDebugEnabled()) cat.debug("Found request parameters user: '"+username+"', password='"+password+"'.");

    if (username == null || password == null) {
      if (cat.isDebugEnabled()) cat.debug("Couldn't find user name or password request parameters.");
      request.setAttribute("uri", uri);
      request.getRequestDispatcher(loginPage).forward(request, response);
      if (cat.isDebugEnabled()) cat.debug("Access to page uri='" + shortUri + "' has been denied.");
      return;
    }

    Authenticator authenticator = WebContext.getAuthenticator();
    if (!authenticator.authenticate(username, password)){
      request.setAttribute("uri", uri);
      request.setAttribute("showLanguageSelector", "true");
      request.getRequestDispatcher(loginPage).forward(request, response);
      cat.debug("access to page uri=" + shortUri + " denied");
      return;
    }

    Set<String> roles = roleMapper.getUserRoles(username);

    userLoginData = new UserLoginData(username, roles);
    session.setAttribute(USER_LOGIN_DATA, userLoginData);
    if (cat.isDebugEnabled()) cat.debug("Set attribute '"+USER_LOGIN_DATA+"' to session.");

    if(userLoginData.isURIPermitted(shortUri) || shortUri.equals(welcomePage) || shortUri.equals("/")){
      chain.doFilter(req, resp);
    } else {
      request.getRequestDispatcher(accessDeniedPage).forward(request, response);
      cat.debug("Access to page uri='" + shortUri + "' has been denied");
    }
  }

  public void destroy() {

  }

}
