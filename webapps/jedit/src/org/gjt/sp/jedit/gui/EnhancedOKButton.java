package org.gjt.sp.jedit.gui;

import org.gjt.sp.jedit.*;

import javax.swing.*;

import errorlist.ErrorSourceUpdate;

/**
 * Created by IntelliJ IDEA.           
 * User: dym
 * Date: 13.01.2006
 * Time: 17:04:21
 * To change this template use File | Settings | File Templates.
 */
public class EnhancedOKButton extends EnhancedButton implements EBComponent
{
  private View view;
  public EnhancedOKButton(String text, Icon icon,String toolTip, String action,
     ActionContext context, View view)
  {
      super(text,icon,toolTip,action,context);
      this.view = view;
      //System.out.println("OK button added to Edit Bus!");
      EditBus.addToBus(this);
  }

  public void handleMessage(EBMessage message)
  {
     if (message instanceof ErrorSourceUpdate)
     {
         handleButtonMessage((ErrorSourceUpdate)message);
     }
  }
  private void handleButtonMessage(ErrorSourceUpdate message)
  {
    Boolean visible = message.getEnabled(view);
    if (visible !=null)
      setEnabled(visible.booleanValue());
  }

  public View getView()
  {
     return view;
  }
  public static class ButtonVisibilitySetter extends EBMessage
  {
     private View view;
     private boolean visible;
     public ButtonVisibilitySetter(boolean visible, View view)
     {
         super(null);
         this.visible = visible;
         this.view = view;
     }
     public Boolean getEnabled(View view)
     {
         if (view!=null && this.view == view )
         {
             //System.out.println(" view == view  !!!!!!!!!!!!!!!      ");
             return new Boolean(visible);
         }
         else return null;
     }
  }
}
