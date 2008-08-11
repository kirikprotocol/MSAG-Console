package ru.novosoft.smsc.util;

import org.apache.log4j.Category;
import java.util.List;
import java.util.ArrayList;

public class AdvancedStringTokenizer {
  private static Category logger = Category.getInstance(AdvancedStringTokenizer.class);

  private List tokens;
  int currenttoken = 0;
    
  public AdvancedStringTokenizer(String text, String delimiters) {
    tokens = new ArrayList(20);
    String src = text.trim();
    int len        = src.length();
    StringBuffer curtok = new StringBuffer(250);
    boolean quotes = false;
    boolean escape = false;
    boolean wasquotes = false;
    for( int i = 0; i < len; i++ ) {
      char curchar = src.charAt( i );
      if( escape ) {
        curtok.append( curchar );
        escape = false;
        continue;
      }
      if( curchar == '\\' ) {
        if( i+1 == len ) {
          logger.warn( "Escape is last char: "+src);
          continue;
        }
        if( src.charAt( i+1 ) == 'x' || src.charAt( i+1 ) == 'X') {
          i++;
          if( i+2 >= len ) {
            logger.warn( "Not enough symbols left in source for hex symbol: "+src);
            i+=2;
            continue;
          }
          String hexv = src.substring( i+1, i+1+2 );
          i+=2;
          try {
            curtok.append( (char)(Integer.valueOf( hexv, 16 ).intValue()) );
          } catch (NumberFormatException ex) {
            logger.warn( "Invalid escape sequence value in string: "+src);
          }
        } else if( src.charAt( i+1 ) == 'u' || src.charAt( i+1 ) == 'U' ) {
          i++;
          if( i+4 >= len ) {
            logger.warn( "Not enough symbols left in source for unicode symbol: "+src);
            i+=4;
            continue;
          }
          String hexv = src.substring( i+1, i+1+4 );
          i+=4;
          try {
            curtok.append( (char)(Integer.valueOf( hexv, 16 ).intValue()) );
          } catch (NumberFormatException ex) {
            logger.warn( "Invalid escape sequence value in string: "+src);
          }
        } else if(src.charAt( i+1 ) == 'n') {
          i++;
          curtok.append('\n');
        } else if(src.charAt( i+1 ) == 'r') {
          i++;
          curtok.append('\r');
        } else if(src.charAt( i+1 ) == 'r') {
          i++;
          curtok.append('\r');
        } else {
          escape = true;
        }
        continue;
      }
      if( quotes ) {
        if( curchar == '"' ) {
          // track if CSV quotes
          if( i+1 < len && src.charAt(i+1) == '"') {
            curtok.append('"');
            i++;
          } else quotes = false;
          continue;
        } else {
          curtok.append(curchar);
          continue;
        }
      }

      if( curchar == '"' && curtok.length() == 0 ) {
        quotes = true;
        wasquotes = true;
        continue;
      }
      if( delimiters.indexOf( curchar ) != -1 ) {
        String ss = curtok.toString();
        if( ss.length() > 0 || wasquotes )
          tokens.add(ss);
        curtok.setLength(0);
        wasquotes = false;
        continue;
      }
      curtok.append(curchar);
    }
    if( curtok.length() > 0 || wasquotes )
      tokens.add(curtok.toString());
  }

  public boolean hasMoreTokens() {
    return currenttoken<tokens.size();
  }

  public String nextToken() {
    String rc = (String)tokens.get( currenttoken );
    currenttoken++;
    return rc;
  }

}
