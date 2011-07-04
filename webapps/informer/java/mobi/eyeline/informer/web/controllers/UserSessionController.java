package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.admin.AdminException;

import javax.faces.application.FacesMessage;
import javax.faces.context.FacesContext;
import javax.servlet.http.HttpSession;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

/**
 * Контроллер сеанса пользователя
 *
 * @author Aleksandr Khalitov
 */
public class UserSessionController extends InformerController {

  public boolean isLogined() {
    return getUserPrincipal() != null;
  }

  public boolean isArchiveDaemonDeployed() {
    return getConfig().isArchiveDaemonDeployed();
  }

  public boolean isBlacklistEnabled() {
    try{
      return getConfig().isBlackListEnabled();
    }catch (AdminException e) {
      addError(e);
      return false;
    }
  }

  public String logout() {
    HttpSession session = getSession(false);
    if (session != null) {
      session.invalidate();
    }
    return "INDEX";
  }

  public List<FacesMessage> getMessages() {
    List<FacesMessage> result = new LinkedList<FacesMessage>();
    Iterator<FacesMessage> i=  FacesContext.getCurrentInstance().getMessages();
    while(i.hasNext()) {
      result.add(i.next());
    }
    return result;
  }

  public boolean isHasMessages() {
    return FacesContext.getCurrentInstance().getMessages().hasNext();
  }

  public void adminGuide(FacesContext context, OutputStream out) throws IOException {
    downloadFile(context,
        "AdminGuide.pdf",
        out
    );
  }

  public void userGuide(FacesContext context, OutputStream out) throws IOException {
    downloadFile(context,
        "UserGuide.pdf",
        out
    );
  }

  public void downloadFile(FacesContext context, String file, OutputStream out) throws IOException{
      BufferedInputStream bis = null;
      BufferedOutputStream bos = null;
      try{
        bis = new BufferedInputStream(context.getExternalContext().getResourceAsStream(file));
        bos = new BufferedOutputStream(out);
        byte[] buffer = new byte[4096];
        int count;
        while((count = bis.read(buffer, 0, buffer.length)) != -1) {
          bos.write(buffer, 0, count);
        }
        bos.flush();
      }finally {
        if(bis != null) {
          try{
            bis.close();
          }catch (Exception ignored){}
        }
        if(bos != null) {
          try{
            bos.close();
          }catch (Exception ignored){}
        }
      }


  }
}
