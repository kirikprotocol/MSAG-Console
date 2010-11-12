package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.controllers.InformerController;

import javax.faces.application.FacesMessage;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class ResendMessagesController extends InformerController{

  private CommonDeliveryResender resender;

  private String text;

  private boolean changeText;

  public ResendMessagesController() {
    Map<String, Object> m = getRequest();
    resender = (CommonDeliveryResender)m.get("resender");
  }

  public boolean isChangeText() {
    return changeText;
  }

  public void setChangeText(boolean changeText) {
    this.changeText = changeText;
  }

  public CommonDeliveryResender getResender() {
    return resender;
  }

  public void setResender(CommonDeliveryResender resender) {
    this.resender = resender;
  }

  public String getText() {
    return text;
  }

  public void setText(String text) {
    this.text = text;
  }

  public String send() {
    if(resender != null) {
      if(text != null && (text = text.trim()).length() == 0) {
        text = null;
      }
      if(changeText && text == null) {
        addLocalizedMessage(FacesMessage.SEVERITY_WARN, "informer.deliveries.messages.resend.text.empty");
        return null;
      }
      User u = getConfig().getUser(getUserName());
      try {
        resender.resend(u, getConfig(), text);
      } catch (AdminException e) {
        addError(e);
      }
    }
    return "MESSAGES";
  }
}
