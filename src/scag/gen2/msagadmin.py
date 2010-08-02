#!/usr/bin/env python

import sys
import optparse
import socket
import struct

class MsagAdmin( object ) :

    def __init__( self, argv ) :

        parser = optparse.OptionParser( usage = 'Usage: %prog [options] host port' )

        parser.add_option( '-v', '--verbose',
        action = 'store_true', dest = 'verbose', default = False )

        parser.add_option( '-e', '--exec', action = 'append', dest = 'doexec', default = [] )

        # parser.add_option( '-h', '--host', action = 'store', dest = 'host', type = 'string' )
        # parser.add_option( '-p', '--port', action = 'store', dest = 'port', type = 'int' )

        self.opts, self.args = parser.parse_args( argv[1:] )
        if self.opts.verbose :
            print '# opts,args =',self.opts, self.args
            pass
        if len(self.args) != 2 :
            parser.error('Incorrect number of arguments')
        else :
            try :
                self.port = int(self.args[1])
            except ValueError :
                parser.error('second argument (port) must be integer, was %s' % self.args[1])
                pass
            pass
        return


    def connect( self ) :
        verbose = self.opts.verbose
        if verbose : print '# connecting to', self.args
        self.hostip = socket.gethostbyname(self.args[0])
        if verbose : print '# ip:', self.hostip
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.hostip,self.port))
        self.sock.settimeout(5.0) # 5 sec timeout
        if verbose : print '# connected.'
        return


    def send( self, xml ) :
        xmllen = len(xml)
        verbose = self.opts.verbose
        if verbose :
            print '# sending xml of len', xmllen
            print xml
            print '# ---'
            pass
        buflen = struct.pack('>I',xmllen)
        self.sock.sendall(buflen)
        self.sock.sendall(xml)
        result = ''
        while len(result) < 4 :
            result += self.sock.recv(1024)
            if verbose : print '# res:', repr(result)
            pass
        xmllen = struct.unpack('>I',result)[0]
        result = result[4:]
        if verbose : print '# len received:', xmllen
        if verbose : print '# res(%s):', len(result), repr(result)
        while len(result) < xmllen :
            result += self.sock.recv(1024)
            if verbose : print '# res(%s):' % len(result), repr(result)
            pass
        if verbose : print '# recv:', xmllen, repr(result)
        return result

    pass


class CmdEngine( object ) :

    def __init__( self, verbose ) :
        self.verbose = verbose
        return

    def process( self, cmd ) :
        args = cmd.split(' ')
        if len(args) == 0 : return ''
        if args[0] == '?' :
            return self._printHelp()
        elif args[0] == 'updateRule' :
            return self._updateRule(args)
        raise ValueError, "command %s is not supported" % args[0]


    def _printHelp( self ) :
        print 'Supported commands are:'
        print 'updateRule ID [TRANSPORT]'
        return ''

    def _updateRule( self, args ) :
        def _usage() :
            raise ValueError, 'usage: updateRule ID [TRANSPORT]'
        if len(args) < 2 : _usage()
        elif len(args) > 3 : _usage()
        transport = 'SMPP'
        try :
            theid = int(args[1])
        except ValueError :
            raise ValueError, "updateRule requires integer ID"
        if len(args) == 3 :
            if args[2] not in ('SMPP','HTTP') : _usage()
            transport = args[2]
            pass
        xml = '''<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE command SYSTEM "command_gw.dtd">
<command name="updateRule" returnType="stringlist">
<param name="serviceId" type="string">%s</param>
<param name="transport" type="string">%s</param>
</command>''' % (theid,transport)
        return xml

    pass


def main() :
    admin = MsagAdmin( sys.argv )
    admin.connect()

    doexec = list(admin.opts.doexec)
    cmdEngine = CmdEngine( admin.opts.verbose )

    while True :

        if len(doexec) > 0 :
            cmd = doexec.pop(0)
        elif len(admin.opts.doexec) > 0 :
            print '# all done.'
            break
        else :
            try :
                cmd = raw_input('cmd or ?>')
            except EOFError :
                break
            pass

        try :
            xml = cmdEngine.process(cmd)
        except ValueError, msg :
            print 'Wrong command "%s": %s' % (cmd, msg)
            continue

        if len(xml) == 0 : continue

        print '----- request -----'
        print xml
        print '-------------------'
        result = admin.send(xml)
        print '----- response -----'
        print result
        print '--------------------'
        pass

    return


if __name__ == '__main__' :
    main()
    pass
