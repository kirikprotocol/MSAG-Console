/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 15, 2002
 * Time: 4:21:23 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import ru.novosoft.smsc.admin.console.parser.CommandLexer;
import ru.novosoft.smsc.admin.console.parser.CommandParser;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.util.auth.AuthenticatorProxy;

import java.io.*;
import java.net.Socket;
import java.net.SocketException;
import java.security.Principal;

public abstract class Session extends Thread
{
    private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

    private final static String COMMAND_QUIT = "quit";

    protected final static int CONSOLE_AUTH_FAIL_SLEEP = 3000;
    protected final static int CONSOLE_AUTH_FAIL_TRIES = 3;

    private Socket socket;
    private Console owner;

    private InputStream  is = null;
    private OutputStream os = null;
    private Principal  user = null;

    public Session(Console owner, Socket socket) {
        this.owner = owner;
        this.socket = socket;
    }

    protected boolean userAuthorized() {
        return (user != null);
    }
    protected boolean authorizeUser(String login, String password) {
        user = AuthenticatorProxy.getInstance().
                authenticate(Constants.TomcatRealmName, login, password);
        return userAuthorized();
    }
    protected boolean userInRole(String role) {
        return (user != null) ? AuthenticatorProxy.getInstance().
                hasRole(Constants.TomcatRealmName, user, role) : false;
    }
    protected boolean commandAllowed(String command) {
        if (userAuthorized()) {
            String roles[] = owner.getCommandRoles(command);
            for (int i=0; roles != null && i<roles.length; i++) {
                if (roles[i] != null &&
                        userInRole(roles[i].trim())) return true;
            }
        }
        return false;
    }

    protected void greeting(PrintWriter writer) {};
    protected void farewell(PrintWriter writer) {};
    protected void prompt(PrintWriter writer) {};

    protected abstract boolean authorize(BufferedReader reader, PrintWriter writer)
            throws Exception;
    protected abstract void display(PrintWriter writer, CommandContext ctx);

    private final static int ESC_IAC = 255;
    private final static int ESC_SB = 250;
    private final static int ESC_SE = 240;
    private final static int ESC_CR = 13;
    private final static int ESC_LF = 10;
    private final static int ESC_BS = 8;
    private final static int ESC_NUL = 0;

    protected String readTelnetLine(PrintWriter writer, boolean echo)
        throws IOException
    {
        int b = -1;
        StringBuffer sb = new StringBuffer();
        boolean escape = false;
        boolean parameter = false;
        boolean typeofop = false;
        while( (b=is.read()) != -1 ) {
            if( typeofop ) {
                //System.out.println("Got TGT="+b);
                escape = false;
                typeofop = false;
                continue;
            }
            if( parameter ) {
                if( b == ESC_SE ) {
                    //System.out.println("Got SE");
                    escape = false;
                    parameter = false;
                }
                continue;
            }
            if( escape ) {
                if( b == ESC_SB ) {
                    //System.out.println("Got SB");
                    parameter = true;
                } else {
                    //System.out.println("Got CMD="+b);
                    typeofop = true;
                }
                continue;
            }
            if( b == ESC_IAC ) {
                //System.out.println("Got IAC");
                escape = true;
                continue;
            }

            if ( b == ESC_BS ) {
                if (sb.length() > 0) {
                    sb.deleteCharAt(sb.length()-1);
                    if (echo) {
                        os.write(ESC_BS); os.write(' ');
                        os.write(ESC_BS); os.flush();
                    }
                }
                continue;
            }

            b = (b == ESC_NUL) ? ESC_LF:b;
            //System.out.println("Got CHR="+(char)b+" code "+b);

            if( echo ) {
                writer.write(b); writer.flush();
            }

            if( b == ESC_LF ) break;
            if( b == ESC_CR ) continue;
            byte bytes[] = {(byte)b};
            sb.append( new String(bytes,System.getProperty("file.encoding")));
        }
        return sb.toString();
    }

    private void process()
        throws Exception
    {
        BufferedReader reader = new BufferedReader(new InputStreamReader(is));
        PrintWriter writer = new PrintWriter(os);

        if (!authorize(reader, writer)) return;

        greeting(writer);
        while (!isStopping && !writer.checkError())
        {
            prompt(writer);
            String input = readTelnetLine(writer, true);
            if (input == null || input.length() == 0) continue;
            if (input.equalsIgnoreCase(COMMAND_QUIT)) {
                farewell(writer); sleep(1000); break;
            }
            CommandContext ctx = new CommandContext(owner.getSmsc());
            try
            {
                CommandLexer lexer = new CommandLexer(new StringReader(input));
                CommandParser parser = new CommandParser(lexer);
                Command cmd = parser.parse();
                if (commandAllowed(cmd.getId())) {
                    cmd.process(ctx);
                } else {
                    ctx.setMessage("Not enough rights to execute specified command");
                    ctx.setStatus(CommandContext.CMD_AUTH_ERROR);
                }
            }
            catch (Exception e) {
                //e.printStackTrace();
                ctx.setMessage(e.getMessage());
                ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
            }
            display(writer, ctx);
        }
    }

    protected Object closeSemaphore = new Object();
    protected boolean isStopping = false;

    public void run()
    {
        try {
            is = socket.getInputStream();
            os = socket.getOutputStream();
            process();
        }
        catch (SocketException e) {
            logger.debug("Client "+socket.getInetAddress().getHostAddress()+" disconnected");
        } catch (IOException e) {
            logger.warn("I/O error occured for "+socket.getInetAddress().getHostAddress(), e);
        } catch (Exception e) {
            logger.error("Unexpected error occured for "+socket.getInetAddress().getHostAddress(), e);
        } finally {
            if( is != null ) try { is.close();} catch (Exception ee){};
            if( os != null ) try { os.close();} catch (Exception ee){};
            if( socket != null ) try { socket.close();} catch (Exception ee){};
        }

        owner.removeSession(this);
        synchronized(closeSemaphore) {
            closeSemaphore.notifyAll();
        }
    }

    public void close() {
        synchronized(closeSemaphore) {
            isStopping = true;
            try {
               closeSemaphore.wait();
            } catch (InterruptedException e) {}
        }
    }

    protected void sendBytes( byte data[]) throws IOException {
        os.write( data ); os.flush();
    }
}
