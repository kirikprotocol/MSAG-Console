header {
package ru.novosoft.smsc.admin.console.parser;

/*
 * Parser code generated by ANTLR
 */

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

import ru.novosoft.smsc.admin.console.commands.*;

}


/*
{
import java.io.*;

class Main {
	public static void main(String[] args) {
		try {
			CommandLexer lexer = new CommandLexer(new DataInputStream(System.in));
			CommandParser parser = new CommandParser(lexer);
			CommandContext ctx = new CommandContext(null);
			parser.parse(ctx);
		} catch(Exception e) {
			System.err.println("exception: "+e);
			e.printStackTrace(System.err);   // so we can get stack trace
		}
	}
}
}
*/


class CommandParser extends Parser;
options {
	importVocab=CommandLexer; // use vocab generated by lexer
	defaultErrorHandler=false;
}

/* ----------------------- Top level parser ----------------------- */
parse returns [Command cmd] {
    cmd = null;
}
	:	ACT_ADD 	cmd = add 
	| 	ACT_DELETE 	cmd = del
	| 	ACT_ALTER 	cmd = alt
	| 	ACT_LIST 	cmd = lst 
	| 	ACT_VIEW	cmd = view
	|	ACT_APPLY     { cmd = new ApplyCommand(); }
	;
	
/* ----------------------- Add action parser ---------------------- */
add returns [Command cmd] {
    cmd = null;
}
	:	TGT_ROUTE 	cmd = addroute
	|	TGT_ALIAS 	cmd = addalias
	|	TGT_SUBJECT	cmd = addsubject
	|	TGT_PROFILE	cmd = addprofile
	;
	
/* ----------------------- Del action parser ---------------------- */
del returns [Command cmd] {
    cmd = null;
}
	:	TGT_ROUTE 	cmd = delroute
	|	TGT_ALIAS 	cmd = delalias
	|	TGT_SUBJECT	cmd = delsubject
	;
/* ----------------------- Alt action parser ---------------------- */
alt returns [Command cmd] {
    cmd = null;
}
	:	TGT_ROUTE   	cmd = altroute
	|	TGT_ALIAS   	cmd = altalias
	|	TGT_SUBJECT 	cmd = altsubject
	|	TGT_PROFILE 	cmd = altprofile
	;
/* ----------------------- Lst action parser ---------------------- */
lst returns [Command cmd] {
    cmd = null;
}
	:	TGT_ROUTE 	{ cmd = new RouteListCommand();   }
	|	TGT_ALIAS 	{ cmd = new AliasListCommand();   }
	|	TGT_SUBJECT	{ cmd = new SubjectListCommand(); }
	;
/* ----------------------- View action parser --------------------- */
view returns [Command cmd] {
    cmd = null;
}
	:	TGT_ROUTE	cmd = viewroute
	|	TGT_ALIAS 	cmd = viewalias
	|	TGT_SUBJECT	cmd = viewsubject
	|	TGT_PROFILE	cmd = viewprofile
	;
	
	
/* ----------------------- Common names parser ------------------------- */
getnameid[String msg] returns [String out] {
    out = null; 
}	:	(qname:QSTR {
		    out = qname.getText().trim();
		    out = out.substring(1,out.length()-1);
		}) 
		|(name:STR {
		    out = name.getText();
		})
	;
	exception
	catch [RecognitionException ex] {
           throw new RecognitionException(ex.getMessage()+". "+
					  msg+" expected. ");
	}

/* ----------------------- Common routes parsers ----------------------- */
srcdef[RouteGenCommand cmd] { // Special command required !!!
    RouteSrcDef def = new RouteSrcDef();
}
	:	( (OPT_SUBJ { 
		    def.setType(RouteSrcDef.TYPE_SUBJECT);
		    def.setSrc(getnameid("Subject name"));
		  }) 
		| (OPT_MASK addr:STR {
		    def.setType(RouteSrcDef.TYPE_MASK); 
		    def.setSrc(addr.getText());
		  })
		) 
		({ 
		    cmd.addSrcDef(def);	
		})
	;
	       	
dstdef[RouteGenCommand cmd] { // Special command required !!!
    RouteDstDef def = new RouteDstDef();
}
	:	( (OPT_SUBJ { 
		    def.setType(RouteDstDef.TYPE_SUBJECT);
		    def.setDst(getnameid("Subject name"));
		  }) 
		| (OPT_MASK addr:STR { 
		    def.setType(RouteDstDef.TYPE_MASK); 
		    def.setDst(addr.getText());
		  })
		)
		({
		    def.setSmeId(getnameid("SME System id"));
		    cmd.addDstDef(def);
		})
	;

dstdef_x[RouteGenCommand cmd] { // Special command required !!!
    RouteDstDef def = new RouteDstDef();
}
	:	( (OPT_SUBJ { 
		    def.setType(RouteDstDef.TYPE_SUBJECT);
		    def.setDst(getnameid("Subject name"));
		  }) 
		| (OPT_MASK addr:STR { 
		    def.setType(RouteDstDef.TYPE_MASK); 
		    def.setDst(addr.getText());
		  })
		)
		({
		    cmd.addDstDef(def);
		})
	;

/* ----------------------- Route command parsers ----------------------- */

route_src[RouteGenCommand cmd]
	:	(OPT_SRC (srcdef[cmd])+)
	;
	exception
	catch [RecognitionException ex] {
           throw new RecognitionException("Route srcdef missed or invalid. Syntax: src (subj <subject_name>|mask <mask>)+");
	}

route_dst[RouteGenCommand cmd]
	:	(OPT_DST (dstdef[cmd])+)
	;
	exception
	catch [RecognitionException ex] {
           throw new RecognitionException("Route dstdef missed or invalid. Syntax: dst (subj <subject_name>|mask <mask> <systemid>)+");
	}

route_dst_x[RouteGenCommand cmd]
	:	(OPT_DST (dstdef_x[cmd])+)
	;
	exception
	catch [RecognitionException ex] {
           throw new RecognitionException("Route dstdef missed or invalid. Syntax: dst (subj <subject_name>|mask <mask>)+");
	}

addroute returns [RouteAddCommand cmd] {
    cmd = new RouteAddCommand();
}
	:	({
		    cmd.setRoute(getnameid("Route name"));
		})
		addroute_flags[cmd]
		(OPT_SVCID num:STR {
		    try {
			cmd.setServiceId(Integer.parseInt(num.getText()));
		    } catch (NumberFormatException ex) {
			throw new NumberFormatException("Expecting integer value for <serviceid>");
		    }
		})
		(OPT_PRI pri:STR {
		    try {
			cmd.setPriority(Integer.parseInt(pri.getText()));		    
		    } catch (NumberFormatException ex) {
			throw new NumberFormatException("Expecting integer value for <priority>");
		    }
		})
		route_src[cmd]
		route_dst[cmd]
	;
addroute_flags[RouteAddCommand cmd]
	:	( OPT_BILL   { cmd.setBill(true);   }
		| OPT_NOBILL { cmd.setBill(false);  })
		( OPT_ARCH   { cmd.setArc(true);    }
		| OPT_NOARCH { cmd.setArc(false);   })
	        ( OPT_ALLOW  { cmd.setAllow(true);  }
		| OPT_DENY   { cmd.setAllow(false); })
	;
	exception
	catch [RecognitionException ex] {
           throw new RecognitionException("Route flags expected. Syntax: (bill|nobill) (arc|noarc) (allow|deny)");
	}

delroute returns [RouteDeleteCommand cmd] {
    cmd = new RouteDeleteCommand();
}
	:	({
		    cmd.setRoute(getnameid("Route name"));
		})
	;

altroute returns [RouteAlterCommand cmd] {
    cmd = new RouteAlterCommand();
}
	:	({
		    cmd.setRoute(getnameid("Route name"));
		})
		altroute_flags[cmd]
		(OPT_SVCID num:STR {
		    try {
			cmd.setServiceId(Integer.parseInt(num.getText()));
		    } catch (NumberFormatException ex) {
			throw new NumberFormatException("Expecting integer value for <serviceid>");
		    }
		}) ?
		(OPT_PRI pri:STR {
		    try {
			cmd.setPriority(Integer.parseInt(pri.getText()));		    
		    } catch (NumberFormatException ex) {
			throw new NumberFormatException("Expecting integer value for <priority>");
		    }
		}) ?
		((ACT_ADD    { cmd.setAction(RouteAlterCommand.ACTION_ADD); }
		  ( route_src[cmd] { cmd.setTarget(RouteAlterCommand.TARGET_SRC); }
		  | route_dst[cmd] { cmd.setTarget(RouteAlterCommand.TARGET_DST); })
		 )
		|(ACT_DELETE { cmd.setAction(RouteAlterCommand.ACTION_DEL); }
		 ( route_src[cmd]   { cmd.setTarget(RouteAlterCommand.TARGET_SRC); }
		 | route_dst_x[cmd] { cmd.setTarget(RouteAlterCommand.TARGET_DST); })
		 )
		)?
	;
altroute_flags[RouteAlterCommand cmd]
	:	( OPT_BILL   { cmd.setBill(true);   } 
		| OPT_NOBILL { cmd.setBill(false);  })?
		( OPT_ARCH   { cmd.setArc(true);    } 
		| OPT_NOARCH { cmd.setArc(false);   })?
	        ( OPT_ALLOW  { cmd.setAllow(true);  }
		| OPT_DENY   { cmd.setAllow(false); })?
	;
	exception
	catch [RecognitionException ex] {
           throw new RecognitionException("Route flags expected. Syntax: [bill|nobill] [arc|noarc] [allow|deny]");
	}

viewroute returns [RouteViewCommand cmd] {
    cmd = new RouteViewCommand();
}
	:	({
		    cmd.setRoute(getnameid("Route name"));
		})
	;

/* ----------------------- Alias command parsers ----------------------- */

addalias returns [AliasAddCommand cmd] {
    cmd = new AliasAddCommand();
}
	:	(mask:STR     { cmd.setAlias(mask.getText());   })
		(addr:STR     { cmd.setAddress(addr.getText());	})
		( OPT_HIDE    { cmd.setHide(true);  }
		| OPT_NOHIDE  { cmd.setHide(false); })?
	;
	exception[mask]
	catch [RecognitionException ex] {
	    throw new RecognitionException("Alias mask expected");
	}
	exception[addr]
	catch [RecognitionException ex] {
	    throw new RecognitionException("Alias address expected");
	}
delalias returns [AliasDeleteCommand cmd] {
    cmd = new AliasDeleteCommand();
}
	:	(mask:STR     { cmd.setAlias(mask.getText()); })
	;
	exception[mask]
	catch [RecognitionException ex] {
	    throw new RecognitionException("Alias mask expected");
	}
altalias returns [AliasAlterCommand cmd] {
    cmd = new AliasAlterCommand();
}
	:	(mask:STR     { cmd.setAlias(mask.getText());   })
		(addr:STR     { cmd.setAddress(addr.getText()); })
		( OPT_HIDE    { cmd.setHide(true);  }
		| OPT_NOHIDE  { cmd.setHide(false); })?
	;
	exception[mask]
	catch [RecognitionException ex] {
	    throw new RecognitionException("Alias mask expected");
	}
	exception[addr]
	catch [RecognitionException ex] {
	    throw new RecognitionException("Alias address expected");
	}
viewalias returns [AliasViewCommand cmd] {
    cmd = new AliasViewCommand();
}
	:	(mask:STR     { cmd.setAlias(mask.getText()); })
	;
	exception[mask]
	catch [RecognitionException ex] {
	    throw new RecognitionException("Alias mask expected");
	}

/* ----------------------- Subject command parsers --------------------- */

addsubj_mask[SubjectGenCommand cmd]
	:	(mask:STR     { cmd.addMask(mask.getText()); })
	;
addsubj_masks[SubjectGenCommand cmd]
	:	(addsubj_mask[cmd] (COMMA addsubj_mask[cmd])*)
	;
	exception
	catch [RecognitionException ex] {
	    throw new RecognitionException("Subject mask list missed or invalid. Syntax: <subject_mask>(,<subject_mask>)*");
	}

addsubject returns [SubjectAddCommand cmd] {
    cmd = new SubjectAddCommand();
}
	:	({
		    cmd.setSubject(getnameid("Subject name"));
		})
		({
		    cmd.setDefaultSmeId(getnameid("SME id"));
		})
		addsubj_masks[cmd]
	;
altsubject returns [SubjectAlterCommand cmd] {
    cmd = new SubjectAlterCommand();
}
	:	({
		    cmd.setSubject(getnameid("Subject name"));
		})
		(((ACT_ADD {
		    cmd.setActionAdd();
		} | ACT_DELETE {
		    cmd.setActionDelete();
		}) addsubj_masks[cmd]) | 
		(OPT_DEFSME {
		    cmd.setDefaultSmeId(getnameid("SME id"));
		}))
	;
delsubject returns [SubjectDeleteCommand cmd] {
    cmd = new SubjectDeleteCommand();
}
	:	({
		    cmd.setSubject(getnameid("Subject name"));
		})
	;
viewsubject returns [SubjectViewCommand cmd] {
    cmd = new SubjectViewCommand();
}
	:	({
		    cmd.setSubject(getnameid("Subject name"));
		})
	;

/* ----------------------- Profile command parsers --------------------- */

addprofile returns [ProfileAddCommand cmd] {
    cmd = new ProfileAddCommand();
}
	:	(mask:STR  { cmd.setMask(mask.getText());    })
		(OPT_REPORT (VAL_FULL { cmd.setFullReport(); }
			   | VAL_NONE { cmd.setNoneReport(); } ))
		(OPT_ENCODE (VAL_DEF  { cmd.setGsm7Encoding(); }
			   | VAL_UCS2 { cmd.setUcs2Encoding(); } ))?
	;
	exception[mask]
	catch [RecognitionException ex] {
	    throw new RecognitionException("Profile mask expected");
	}
altprofile returns [ProfileAlterCommand cmd] {
    cmd = new ProfileAlterCommand();
}
	:	(addr:STR  { cmd.setAddress(addr.getText()); })
		(OPT_REPORT (VAL_FULL { cmd.setFullReport(); }
			   | VAL_NONE { cmd.setNoneReport(); } ))
		(OPT_ENCODE (VAL_DEF  { cmd.setGsm7Encoding();  }
			   | VAL_UCS2 { cmd.setUcs2Encoding();  } ))?
	;
	exception[addr]
	catch [RecognitionException ex] {
	    throw new RecognitionException("Profile address expected");
	}
delprofile returns [ProfileDeleteCommand cmd] {
    cmd = new ProfileDeleteCommand();
}
	:	(mask:STR  { cmd.setMask(mask.getText()); })
	;
	exception[mask]
	catch [RecognitionException ex] {
	    throw new RecognitionException("Profile mask expected");
	}
viewprofile returns [ProfileViewCommand cmd] {
    cmd = new ProfileViewCommand();
}
	:	(addr:STR  { cmd.setAddress(addr.getText()); })
	;
	exception[addr]
	catch [RecognitionException ex] {
	    throw new RecognitionException("Profile address expected");
	}
	