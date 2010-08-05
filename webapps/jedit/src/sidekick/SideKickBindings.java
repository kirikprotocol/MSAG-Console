/*
 * SideKickBindings.java
 * :tabSize=8:indentSize=8:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 *
 * Copyright (C) 2003 Slava Pestov
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

package sidekick;

import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.Component;
import org.gjt.sp.jedit.gui.KeyEventWorkaround;
import org.gjt.sp.jedit.GUIUtilities;
import org.gjt.sp.jedit.View;
import org.gjt.sp.jedit.jEdit;


import xml.XmlActions;

/**
 * Manages our key bindings.
 */
class SideKickBindings extends KeyAdapter
{
 //{{{ keyTyped() method
 public void keyTyped(KeyEvent evt)
 {
    //(SideKickPlugin.getParserForBuffer(GUIUtilities.getView((Component)evt.getSource()).getBuffer())).setParsingComplete(false);
    //System.out.println("sidekick.SideKickBinding keyTyped before parse line 40");
  evt = KeyEventWorkaround.processKeyEvent(evt);
  if(evt == null)
   return;

  char ch = evt.getKeyChar();
  if(ch == '\b') return;
    //System.out.println("sidekick.SideKickBinding keyTyped before parse line 48 key= "+evt.getKeyChar());
  View view = GUIUtilities.getView((Component)evt.getSource());
   //System.out.println("sidekick.SideKickBinding keyTyped before parse line 48 view= "+view+ " buffer= "+view.getBuffer());
    if (jEdit.getBooleanProperty("buffer.sidekick.keystroke-parse")){
        SideKickPlugin.parse(view,false);
    } else{
        if (jEdit.getBooleanProperty("buffer.sidekick.auto-parse")){
            SideKickPlugin.parseWithDelay(view);
        }
    }
  /*
  SideKickParser parser = SideKickPlugin.getParserForView(view);
   System.out.println("parser= "+parser);
   //if (view.isEdittag()) view.getBuffer().setBooleanProperty("sidekick.keystroke-parse",false);
/*    if (view.isEdittag() && ch==' ') {
     xml.EditTag dialog=view.getEditTag();
     dialog.updateTag();
   }
   else  if(parser != null && parser.supportsCompletion())
  {
   System.out.println("sidekick.SideKickBinding keyTyped before parse line 54");
      String parseKeys = parser.getParseTriggers();
   if(parseKeys != null && parseKeys.indexOf(ch) != -1)
      { System.out.println("sidekick.SideKickBinding keyTyped before parse line 57");
        SideKickPlugin.parse(view,false);
      }
      System.out.println("sidekick.SideKickBinding keyTyped before parse line 61");
   String instantKeys = parser.getInstantCompletionTriggers();
   if(instantKeys != null && instantKeys.indexOf(ch) != -1)
    SideKickActions.keyComplete(view);
   else if(parser.canCompleteAnywhere()) {
        System.out.println("sidekick.SideKickBinding keyTyped before parse line 65");
    SideKickActions.keyCompleteWithDelay(view); }
  } */
 } //}}}
}
