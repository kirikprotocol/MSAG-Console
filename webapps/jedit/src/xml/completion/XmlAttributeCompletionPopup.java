package xml.completion;

//{{{ Imports
import org.gjt.sp.jedit.View;
import org.gjt.sp.jedit.GUIUtilities;
import org.gjt.sp.jedit.jEdit;
import org.gjt.sp.jedit.EditPane;
import org.gjt.sp.jedit.gui.KeyEventWorkaround;
import org.gjt.sp.jedit.textarea.JEditTextArea;

import javax.swing.*;
import javax.swing.event.ListDataListener;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.ListSelectionEvent;
import java.awt.*;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import sidekick.SideKickCompletion;
//}}}


/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 13.09.2005
 * Time: 18:21:46
 * To change this template use File | Settings | File Templates.
 */
public class XmlAttributeCompletionPopup extends JWindow
/*
 * XmlAttributeCompletionPopup.java - Completer popup
 * :tabSize=8:indentSize=8:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 *
 * Copyright (C) 2000, 2005 Slava Pestov , modifed Andrey Chernov
 *
 * The XML plugin is licensed under the GNU General Public License, with
 * the following exception:
 *
 * "Permission is granted to link this code with software released under
 * the Apache license version 1.1, for example used by the Xerces XML
 * parser package."
 */


{
 //{{{ fitInScreen() method
 public static Point fitInScreen(Point p, Window w, int lineHeight)
 {
  Rectangle screenSize = w.getGraphicsConfiguration().getBounds();
  if(p.y + w.getHeight() >= screenSize.height)
   p.y = p.y - w.getHeight() - lineHeight;
  return p;
 } //}}}

 //{{{ SideKickCompletionPopup constructor
 public XmlAttributeCompletionPopup(View view, //SideKickParser parser,
                                    int caret, SideKickCompletion complete)
 {
  super(view);

  this.view = view;
 // this.parser = parser;
  this.textArea = view.getTextArea();

  list = new JList();

  list.addMouseListener(new MouseHandler());
  list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

  // stupid scrollbar policy is an attempt to work around
  // bugs people have been seeing with IBM's JDK -- 7 Sep 2000
  JScrollPane scroller = new JScrollPane(list,
   JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
   JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);

  getContentPane().add(scroller, BorderLayout.CENTER);

  KeyHandler keyHandler = new KeyHandler();
  addKeyListener(keyHandler);
  getRootPane().addKeyListener(keyHandler);
  list.addKeyListener(keyHandler);
  list.addListSelectionListener(new ListHandler());
  view.setKeyEventInterceptor(keyHandler);

  GUIUtilities.requestFocus(this,list);

  this.complete = complete;
  updateListModel();

  Point location = textArea.offsetToXY(caret - complete.getTokenLength());
  location.y += textArea.getPainter().getFontMetrics().getHeight();

  SwingUtilities.convertPointToScreen(location,
   textArea.getPainter());

  setLocation(fitInScreen(location,this,
   textArea.getPainter().getFontMetrics()
   .getHeight()));
  show();
 } //}}}

 //{{{ dispose() method
 public void dispose()
 {
  view.setKeyEventInterceptor(null);
  super.dispose();
  SwingUtilities.invokeLater(new Runnable()
  {
   public void run()
   {
    view.getTextArea().requestFocus();
   }
  });
 } //}}}

//{{{ Private members

 //{{{ Instance variables
 private int mode;
 private View view;
 private JEditTextArea textArea;
 private JList list;
 //private SideKickParser parser;
 private SideKickCompletion complete;
 //}}}

  //{{{ updateListModel() method
 private void updateListModel()
 {
  if(complete == null || complete.size() == 0)
  {
   list.setListData(new String[] {
    jEdit.getProperty("sidekick-complete.none")
   });
   list.setCellRenderer(new DefaultListCellRenderer());
   list.setVisibleRowCount(1);
  }
  else
  {
   setListModel(complete);
   list.setCellRenderer(complete.getRenderer());
   list.setVisibleRowCount(Math.min(8,complete.size()));
   list.setFixedCellHeight(list.getCellBounds(0,0).height);
  }

  pack();
 } //}}}

 //{{{ setListModel() method
 private void setListModel(final SideKickCompletion items)
 {
  ListModel model = new ListModel()
  {
   public int getSize()
   {
    return items.size();
   }

   public Object getElementAt(int index)
   {
    return items.get(index);
   }

   public void addListDataListener(ListDataListener l) {}
   public void removeListDataListener(ListDataListener l) {}
  };

  list.setModel(model);
  list.setSelectedIndex(0);
 } //}}}

 //{{{ updateSelection() method
 void updateSelection()
 {
  int index = list.getSelectedIndex();
  if(index == -1)
   return;

  String description = complete.getCompletionDescription(index);
  view.getStatus().setMessageAndClear(description);
 } //}}}

 //}}}

 //{{{ KeyHandler class
 class KeyHandler extends KeyAdapter
 {
  //{{{ keyPressed() method
  public void keyPressed(KeyEvent evt)
  {
   evt = KeyEventWorkaround.processKeyEvent(evt);
   if(evt == null)
    return;

   switch(evt.getKeyCode())
   {
   case KeyEvent.VK_ENTER:
   // keyTyped('\n');
    evt.consume();
    break;
   case KeyEvent.VK_TAB:
    //keyTyped('\t');
    evt.consume();
    break;
   case KeyEvent.VK_ESCAPE:
    textArea.getBuffer().setBooleanProperty("sidekick.keystroke-parse",true);
    dispose();
    evt.consume();
    break;
   case KeyEvent.VK_UP:
    int selected = list.getSelectedIndex();

    if(selected == 0)
     selected = list.getModel().getSize() - 1;
    else if(getFocusOwner() == list)
     return;
    else
     selected = selected - 1;

    list.setSelectedIndex(selected);
    list.ensureIndexIsVisible(selected);

    evt.consume();
    break;
   case KeyEvent.VK_DOWN:
    /* int */ selected = list.getSelectedIndex();

    if(selected == list.getModel().getSize() - 1)
     selected = 0;
    else if(getFocusOwner() == list)
     return;
    else
     selected = selected + 1;

    list.setSelectedIndex(selected);
    list.ensureIndexIsVisible(selected);

    evt.consume();
    break;
   case KeyEvent.VK_SPACE:
     keyTyped(' ');
     dispose();
    evt.consume();
     break;
   case KeyEvent.VK_BACK_SPACE:
   case KeyEvent.VK_DELETE:
    if(view.getEditTag().isBackCursor()) {
        int _caret=textArea.getCaretPosition();
        textArea.setCaretPosition(_caret-view.getEditTag().getCloseTag().length());
       }
     textArea.getBuffer().setBooleanProperty("sidekick.keystroke-parse",true);
    dispose();
    view.processKeyEvent(evt,true);
    break;
   default:
    // from DefaultInputHandler
    if(!(evt.isControlDown() || evt.isAltDown() || evt.isMetaDown()))
    {
     if(!evt.isActionKey())
     {
      break;
     }
    }

    dispose();
    view.processKeyEvent(evt,true);
    break;
   }
  } //}}}

  //{{{ keyTyped() method
  public void keyTyped(KeyEvent evt)
  {
   evt = KeyEventWorkaround.processKeyEvent(evt);
   if(evt == null)
    return;

   char ch = evt.getKeyChar();
   if(ch == '\b')
    return;
    System.out.println("XmlAttributeCompletionPopup.keyTyped(KeyEvent evt) ona tut !!!!");
   keyTyped(ch);

   evt.consume();
  } //}}}

  //{{{ keyTyped() method
  private void keyTyped(char ch)
  {
   if(complete == null)
   {
     System.out.println("XmlAttributeCompletionPopup.KeyHandler.keyTyped suda zalezla !!!!");
    view.getTextArea().userInput(ch);
    dispose();
   }
   else  if(complete.handleKeystroke(
    list.getSelectedIndex(),ch))
   {
    EditPane editPane = view.getEditPane();
    int caret = editPane.getTextArea()
     .getCaretPosition();
 /*   if(!complete.updateInPlace(editPane,caret))
     complete = parser.complete(editPane,caret);
   */ updateListModel();
   }
   else
     dispose();

  } //}}}
 } //}}}

 //{{{ ListHandler class
 class ListHandler implements ListSelectionListener
 {
  public void valueChanged(ListSelectionEvent evt)
  {
   updateSelection();
  }
 } //}}}

 //{{{ MouseHandler class
 class MouseHandler extends MouseAdapter
 {
  public void mouseClicked(MouseEvent evt)
  {
   // XXX
   if(complete != null)
    complete.handleKeystroke(list.getSelectedIndex(),'\n');
   dispose();
  }
 } //}}}
}
