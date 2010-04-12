#!/bin/env python

import getopt
import sys
import os
from datetime import datetime, timedelta

#
# Creates deliveries file(s) with given speed
#

class MsgMaker(object) :

    def __init__(self) :
        opts, args = getopt.getopt(sys.argv[1:],
        "hi:s:o:m:",
        [ 'help', 'input=', 'speed=', 'output=', 'messages=' ])

        def usage() :
            print 'Usage: %s [opts]' % sys.argv[0]
            print 'Options:'
            print ' -i --input INPUT  - specify input file with ABNT,REG,MSG'
            print ' -s --speed SPEED  - specify required speed'
            print ' -o --output DIR   - specify output directory'
            print ' -m --messages N   - specify the number of messages'
            return

        self.srcfile = None
        self.speed = 0
        self.output = None
        self.messages = 0
        for opt,arg in opts :
            if opt == '-h' or opt == '--help' :
                usage()
                sys.exit(0)
            elif opt == '-i' or opt == '--input' :
                self.srcfile = arg
            elif opt == '-s' or opt == '--speed' :
                self.speed = int(arg)
            elif opt == '-o' or opt == '--output' :
                self.output = arg
            elif opt == '-m' or opt == '--messages' :
                self.messages = int(arg)
                pass
            pass

        if not self.srcfile :
            raise ValueError, 'Please specify a file name with subscriber+message'
        elif self.speed <= 0 :
            raise ValueError, 'Please specify speed >= 1'
        elif self.messages <= 0 :
            raise ValueError, 'Please specify number of messages >= 1'
        elif not self.output :
            raise ValueError, 'Please specify output directory'
        elif not os.path.isdir(self.output) :
            raise ValueError, 'Output directory %s must exist' % self.output

        fd = open(self.srcfile,'r')
        lines = [ x.strip() for x in fd.readlines()]
        fd.close()
        # print 'lines=',lines
        self.whoregmsg = [ tuple(y.strip() for y in x.split(',',2)) for x in lines ]
        # print self.whoregmsg
        # check that all abonents have messages
        badusers = filter( lambda x : len(x) != 3, self.whoregmsg )
        if badusers :
            raise ValueError, 'Invalid messages found: %s' % badusers
        elif not self.whoregmsg :
            raise ValueError, 'File %s contains no messages' % self.srcfile
        return


    def createTaskMessages( self, output ) :
        total = 0
        persec = 0
        next = datetime.now() + timedelta(0,20,0)
        onesecond = timedelta(0,1,0)
        prevhour = next.hour
        def __timestem() : return '%02u%02u%02u' % (next.hour, next.minute, next.second)
        timestem = __timestem()
        fd = None
        while total < self.messages :
            for abnt, reg, msg in self.whoregmsg :
                if not fd :
                    # opening a new file, writing a header
                    datestem = '%02u%02u%02u' % (next.year % 100, next.month, next.day)
                    dirname = os.path.join(self.output,datestem)
                    if not os.path.isdir(dirname):
                        os.mkdir(dirname)                        
                        pass
                    fn = os.path.join(dirname,'%02s.csv' % next.hour)
                    fd = open(fn,'w')
                    fd.write('STATE,DATE,ABONENT,REGION,USERDATA,MESSAGE\n')
                    pass
                fd.write('0,%s%s,%s,%s,,"%s"\n' % (datestem,timestem,abnt,reg,msg))
                total += 1
                if total >= self.messages : break
                persec += 1
                if persec >= self.speed :
                    # this second is filled
                    persec = 0
                    next += onesecond
                    timestem = __timestem()
                    if next.hour != prevhour :
                        # next hour reached
                        fd.close()
                        fd = None
                        prevhour = next.hour
                        pass
                    pass
                pass
            pass
        if fd : fd.close()
        return

    pass


def main() :
    maker = MsgMaker()
    maker.createTaskMessages( maker.output )
    return


if __name__ == '__main__' :
    main()
    pass
