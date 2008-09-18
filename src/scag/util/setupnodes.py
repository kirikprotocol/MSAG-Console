#!/bin/env python

import sys
import os
import stat
import xml.sax
import xml.sax.xmlreader
import xml.sax.handler
import xml.sax.saxutils


class SCAGConfig :

    _types = {
    type({}): 'section', 
    type(''): 'string',
    type(u''): 'ustring',
    type(10): 'int',
    type(False): 'bool' }
    
    def __init__( self, root = None ) :
        self.root = root
        pass


    def type(self,elt) :
        return self._types[type(elt)]


    def dump(self) :
        indent = ''
        section = self.root
        self._dodump(indent,section)
        return


    def getsections(self,path) :
        elt = self._findnode(path)
        res = [ nm for nm in elt[''] if self.type(elt[nm]) == 'section' ]
        return res


    def getsubconfig(self,path) :
        return SCAGConfig(self._findnode(path))


    def __iter__(self) :
        for nm in self.root[''] :
            yield (nm,self.root[nm])
            pass
        return


    def __getitem__(self,x) :
        if x.find('.') == -1 :
            return self.root[x]
        else :
            return self._findnode(x)
        pass


    def _dodump(self,indent,section) :
        for nm in section[''] :
            elt = section[nm]
            tp = self.type(elt)
            if tp == 'section' :
                print '%s%s %s:' % (indent,tp,nm)
                self._dodump(indent+'  ',elt)
            else :
                print '%s%s %s = %s' % (indent,tp,nm,elt)
                pass
            pass
        return


    def _findnode(self,path) :
        elts = path.split('.')
        res = self.root
        for elt in elts :
            if elt == '' : continue
            res = res[elt]
            pass
        return res

    pass



class MyContentHandler(xml.sax.handler.ContentHandler) :
    
    def __init__( self ) :
        return xml.sax.handler.ContentHandler.__init__(self)


    def setConfig( self, scagcfg ) :
        self._cfg = scagcfg
        return


    def getConfig( self ) :
        return self._cfg


    def setDocumentLocator( self, loc ) :
        self._loc = loc
        return xml.sax.handler.ContentHandler.setDocumentLocator(self, loc)


    def startDocument( self ) :
        # print 'start document'
        self._stack = []
        self._elt = None
        return xml.sax.handler.ContentHandler.startDocument(self)


    def startElement( self, name, attrs ) :
        # print 'start element: %s, %s' % (name, attrs)
        self._stack.append(self._elt)
        if name == 'config' :
            self._cfg.root = {'':[]}
            self._elt = self._cfg.root
        else :
            nm = attrs.getValue('name')
            if self._elt.has_key(nm) :
                raise SAXParseException( 'element %s is mentioned twice' % name, None, self._loc)
            if name == 'section' :
                x = {'':[]}
            elif name == 'param' :
                ptype = attrs.getValue('type')
                if ptype not in ('int', 'bool', 'string') :
                    raise SAXParseException('wrong type %s of the element %s' % (ptype, name), None, self._loc)
                x = [ptype,'']
            else :
                raise SAXNotRecognized('wrong element %s' % name, None, self._loc)
            self._elt[nm] = x
            self._elt[''].append(nm)
            self._elt = x
            pass
        return xml.sax.handler.ContentHandler.startElement(self, name, attrs )


    def endElement( self, name ) :
        # print 'end element: %s' % name
        parent = self._stack.pop()
        if name == 'param' :
            assert(parent is not None)
            # post-processing param
            nm = parent[''][-1]
            ptype = self._elt[0]
            if ptype == 'bool' :
                try :
                    parent[nm] = int(self._elt[1]) and True or False
                except ValueError :
                    parent[nm] = False
                    pass
                pass
            elif ptype == 'int' :
                parent[nm] = int(self._elt[1])
            elif ptype == 'string' :
                parent[nm] = self._elt[1]
                pass
            # print parent
            pass
        self._elt = parent
        return xml.sax.handler.ContentHandler.endElement(self, name )


    def characters( self, contents ) :
        # print 'characters: %s' % contents
        if type(self._elt) == type([]) :
            # param
            self._elt[1] = self._elt[1] + contents
            pass
        return xml.sax.handler.ContentHandler.characters(self, contents)
    pass



def readconfig( url ) :
    parser = xml.sax.make_parser()
    parser.setFeature( xml.sax.handler.feature_external_ges, False )
    contents = MyContentHandler()
    parser.setContentHandler( contents )
    cfg = SCAGConfig()
    contents.setConfig( cfg )
    parser.parse( url )
    contents.setConfig( None )
    return cfg


def setupnodes( opts ) :

    inputnames = sys.argv[1:]

    if len(inputnames) < 2 :
        print >> sys.stderr, "USAGE: %s config-ctrl.xml config-msagXX.xml ..."
        sys.exit(1)
        pass

    # reading the configuration of controller
    ctrlfn = inputnames.pop(0)
    cfg = readconfig(ctrlfn)
    ctrlnodes = cfg[ 'Controller.msagnodes' ]
    totstores = cfg['Controller.storages']

    names = []
    cfgs = []
    alllocs = {}
    nodelocs = []

    wasnodes = 0

    for fn in inputnames :

        cfg = readconfig(fn)

        # first collect the old locations
        oldlocs = 0
        try :
            loccfg = cfg.getsubconfig( 'SessionManager.oldlocations' )
            wasnodes += 1
        except KeyError :
            loccfg = []
            pass
        for k,v in loccfg :
            if not os.path.isabs(v) :
                print >> sys.stderr, "Path %s in %s is not absolute"
                sys.exit(1)
                pass
            if not os.path.exists(v) :
                continue
            elif not os.path.isdir(v) or os.path.islink(v) :
                continue
            oldlocs += 1
            if alllocs.get(v,-1) != -1 :
                continue
            alllocs[v] = -1
            pass

        locs = []

        # get actual locations
        try :
            loccfg = cfg.getsubconfig('SessionManager.locations')
        except KeyError :
            loccfg = []
            pass
        for k,v in loccfg :
            if not os.path.isabs(v) :
                print >> sys.stderr, "Path %s in %s is not absolute"
                sys.exit(1)
                pass
            if not os.path.exists(v) :
                print >> sys.stderr, "ERROR: location %s does not exist (config %s)" % (v,fn)
                sys.exit(1)
            elif not os.path.isdir(v) or os.path.islink(v) :
                print >> sys.stderr, "ERROR: location %s is not a directory (config %s)" % (v,fn)
                sys.exit(1)
                pass

            locs.append(v)
            # a check for cross-msag locs
            crossmsag = alllocs.get(v,len(cfgs))
            if crossmsag not in (-1, len(cfgs)) :
                print >> sys.stderr, "WARNING: location %s belongs to MSAG #%s and #%s" % (v,alllocs[v],len(cfgs))
                pass
            alllocs[v] = len(cfgs)
            pass

        if len(locs) > 0 :
            # check the node number
            mynodes = cfg['General.nodes']
            if mynodes != ctrlnodes :
                print >> sys.stderr, "Config mismatch: nodes in %s and %s differ" % (ctrlfn,fn)
                sys.exit(1)
                pass

            storages2 = cfg['General.storages']
            if storages2 != totstores :
                print >> sys.stderr, "Config mismatch: number of storages in %s and %s differ" % (ctrlfn,fn)
                sys.exit(1)
                pass

            mynode = cfg['General.node']
            if mynode != len(cfgs) :
                print >> sys.stderr, "Config order mismatch: wrong node number (%d) in %s" % (mynode,fn)
                sys.exit(1)
                pass
            names.append(fn)
            cfgs.append(cfg)
            nodelocs.append(locs)
            pass
        else :
            print >> sys.stderr, "config %s has no locations (old config assumed, skipped)" % fn
            if oldlocs == 0 :
                print >> sys.stderr, "ERROR: it also has no old locations, no sense?"
                sys.exit(1)
                pass
            pass

        pass

    # print alllocs
    # print cfgs

    nodes = len(cfgs)
    if nodes < 1 :
        print >> sys.stderr, "ERROR: you have to specify at least one existing MSAG config file"
        sys.exit(1)
        pass

    if nodes != ctrlnodes :
        print >> sys.stderr, "ERROR: Controller nodes number (%d) does not match number of MSAG configs (%d)" % (
        ctrlnodes, nodes )
        sys.exit(1)
        pass

    print 'Total number of nodes: %d' % nodes
    print 'Unique locations (%d):' % len(alllocs)
    tmp = [ (v,k) for k,v in alllocs.iteritems()]
    tmp.sort()
    for k,v in tmp :
        print '%2d. %s' % (k,v)
        pass

    # collect existing storages
    storages = {}
    for n in range(totstores) :

        sn = "%03d" % n

        for l in alllocs.keys() :

            dn = os.path.join(l,sn)
            if not os.path.exists(dn) :
                continue
            elif not os.path.isdir(dn) or os.path.islink(dn) :
                print >> sys.stderr, "ERROR: %s is not a dir" % dn
                sys.exit(1)
                pass
            # check that it is not found somewhere else
            if storages.get(n) is not None :
                oldstat = os.stat(storages.get(n))
                newstat = os.stat(dn)
                if oldstat[stat.ST_DEV] != newstat[stat.ST_DEV] or \
                oldstat[stat.ST_INO] != newstat[stat.ST_INO] :
                    print >> sys.stderr, "ERROR: storage #%d is found in two locations:\n  %s\n  %s" % (n,storages[n],dn)
                    sys.exit(1)
                    pass
                # the same file, ok
                continue
            storages[n] = dn
            pass
        pass

    print 'Existing storages (%d):' % len(storages)
    for n in range(totstores) :
        if not storages.get(n) : continue
        print "%3d. %s" % (n,storages[n])
        pass

    # moving storages
    print 'Shuffling storages:'
    created = 0
    moved = 0
    failed = 0
    for n in range(totstores) :
        sn = '%03d' % n
        node = n % nodes
        locs = nodelocs[node]
        lidx = (n / nodes) % len(locs)
        loc = locs[lidx]
        newpath = os.path.join( loc, sn )
        try :
            if storages.get(n) is None :
                print 'node=%d, loc=%d, making %s' % (node,lidx,newpath)
                os.mkdir( newpath )
                created += 1
            elif storages.get(n) != newpath :
                # move
                print 'node=%d, loc=%d, move %s -> %s' % (node,lidx,storages.get(n),newpath)
                fd = os.popen( 'mv %s %s 2>&1' % (storages.get(n),newpath), 'r' )
                l = fd.readlines()
                res = fd.close()
                if res != None :
                    print >> sys.stderr, "WARNING: mv problem:\n", l
                    failed += 1
                else :
                    moved += 1
                    pass
                pass
            pass
        except OSError :
            failed += 1
            pass
        pass

    print "created: %d" % created
    print "moved:   %d" % moved
    print "failed:  %d" % failed
    return


def main() :

    # wrapper around sys.stdout
    if sys.stdout.encoding is None :
        # to file
        import locale
        import codecs
        lang, enc = locale.getdefaultlocale()
        Writer = codecs.lookup(enc)[3]
        sys.stdout = Writer( sys.stdout )
        pass

    setupnodes( None )
    return


if __name__ == '__main__' :
    main()
    pass
