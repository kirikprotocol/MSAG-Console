package xml.completion;

//{{{ Imports
import sidekick.SideKickCompletion;
import org.gjt.sp.jedit.View;
import org.gjt.sp.jedit.EditPane;
import org.gjt.sp.jedit.textarea.JEditTextArea;

import javax.swing.*;
import java.util.*;
//}}}

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 14.09.2005
 * Time: 14:24:16
 * To change this template use File | Settings | File Templates.
 */
public class XmlAttributeCompletion  extends SideKickCompletion
{
/*
 * XmlAttributeCompletion.java
 * :tabSize=8:indentSize=8:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 *
 * Copyright (C) 2003, 2005 Slava Pestov  , Andrey Chernov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


  /**
   * A code completion instance.<p>
   *
   * This is a wrapper around a collection of possible completions, with callbacks
   * for inserting and displaying the completions in a popup menu.
   *
   * @author Slava Pestov
   * @version $Id$
   * @since SideKick 0.3
   */
   //{{{ XmlAttributeCompletion constructor
   /**
    * @deprecated Use the other constructor instead.
    */
   public XmlAttributeCompletion() {}
   //}}}

   //{{{ SideKickCompletion constructor
   /**
    * @since SideKick 0.3.2
    */
   public XmlAttributeCompletion(View view, String text)
   {
    this.view = view;
    textArea = view.getTextArea();
    this.text = text;
   } //}}}

   //{{{ SideKickCompletion constructor
   /**
    * @since SideKick 0.3.2
    */
   public XmlAttributeCompletion(View view, String text, List items)
   {
    this(view,text);
    this.items = items;
   } //}}}

   //{{{ SideKickCompletion constructor
   /**
    * @since SideKick 0.3.2
    */
   public XmlAttributeCompletion(View view, String text, Object[] items)
   {
    this(view,text);
    this.items = Arrays.asList(items);
   } //}}}

   //{{{ size() method
   public int size()
   {
    return items.size();
   } //}}}

   //{{{ get() method
   public Object get(int index)
   {
    return items.get(index);
   } //}}}

   //{{{ getCompletionDescription() method
   public String getCompletionDescription(int index)
   {
    return null;
   } //}}}

   //{{{ isCompletionSelectable() method
   public boolean isCompletionSelectable(int index)
   {
    return true;
   } //}}}

   //{{{ updateInPlace() method
   /**
    * @return If this returns false, then we create a new completion
    * object after user input.
    */
   public boolean updateInPlace(EditPane editPane, int caret)
   {
    return false;
   } //}}}

   //{{{ getRenderer() method
   public ListCellRenderer getRenderer()
   {
    return new DefaultListCellRenderer();
   } //}}}

   //{{{ insert() method
   public void insert(int index)
   {
    String selected = String.valueOf(get(index));
    List names=view.getEditTag().getNames();
    names.remove(selected);
    String insert = selected.substring(text.length());
    insert=insert+" =\"\"";
    String closeTag=view.getEditTag().getCloseTag();//"/>";
   if (view.getEditTag().isBackCursor()) {
     int caret=textArea.getCaretPosition();
     textArea.setCaretPosition(caret-closeTag.length());
     textArea.getBuffer().setBooleanProperty("sidekick.keystroke-parse",true);
    }
    textArea.setSelectedText(insert);

    int caret=textArea.getCaretPosition();
    if (names.size()==0  ) {
      if (!view.getEditTag().isBackCursor()) {
        //String closeTag=view.getEditTag().getCloseTag();
        //closeTag="/>";// </"+view.getEditTag().getElementName()+">";
        textArea.setSelectedText(closeTag);
      }
      textArea.getBuffer().setBooleanProperty("sidekick.keystroke-parse",true);
      //view.setEdittag(false);view.setEditTag(null);
    }
     view.setKeyEventInterceptor(null);

    textArea.setCaretPosition(caret-1);
   } //}}}

   //{{{ getTokenLength() method
   /**
    * The length of the text being completed (popup will be positioned there).
    */
   public int getTokenLength()
   {
    return text.length();
   } //}}}

  //{{{ handleKeystroke() method
   /**
    * @param selectedIndex -1 if the popup is empty, otherwise the index of
    * the selected completion.
    * @param keyChar the character typed by the user.
    * @return True if completion should continue, false otherwise.
    * @since SideKick 0.3.2
    */
   public boolean handleKeystroke(int selectedIndex, char keyChar)
   { System.out.println("XMLAttributeCompletion.handleKeystroke start line 173");
    if(keyChar == '\t' || keyChar == '\n')
    {
     insert(selectedIndex);
     return false;
    }
    else if(keyChar == ' ')
    {
     insert(selectedIndex);
     textArea.userInput(' ');
     return false;
    }
    else
    {
     textArea.userInput(keyChar);
     return true;
    }
   } //}}}

  public void setItems(List items)
  {
    this.items = items;
  }

  protected View view;
   protected JEditTextArea textArea;
   protected String text;
   protected List items = new ArrayList();

}
