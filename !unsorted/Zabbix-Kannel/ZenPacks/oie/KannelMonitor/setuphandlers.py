#
# This software is subject to the provisions of the Zope Public License,
# Version 2.1 (ZPL).  A copy of the ZPL should accompany this distribution.
#
from Acquisition import aq_base
from lbn.zenoss.packutils import addZenPackObjects
from Products.ZenEvents.EventClass import manage_addEventClass
from Products.ZenModel.RRDTemplate import manage_addRRDTemplate
from datasources import KannelDataSource
from config import DATAPOINTS, GRAPHPOINTS

def setDataPoints(graphdef, prefix, tags):
    pretty_tags = {}
    for tag in tags:
        if tag.find('_') != -1:
            pretty = tag.replace('_', ' ').capitalize()
        else:
            pretty = tag
        pretty_tags[pretty] = tag

    graphdef.manage_addDataPointGraphPoints(pretty_tags.keys())
    for dp in graphdef.graphPoints():
        # seems reload gets confused ...
        id = dp.getId()
        if pretty_tags.has_key(id):
            dp.dpName = '%s_%s' % (prefix, pretty_tags[id])

def install(zport, zenpack):
    """
    Set the collector plugin
    """
    dmd = zport.dmd

    if not getattr(aq_base(dmd.Events.Status), 'Kannel', None):
        manage_addEventClass(dmd.Events.Status, 'Kannel')

    tpls = dmd.Devices.Server.rrdTemplates

    if not getattr(aq_base(tpls), 'KannelServer', None):
        manage_addRRDTemplate(tpls, 'KannelServer')
        tpl = tpls.KannelServer

        tpl.manage_changeProperties(description='Monitors Kannel SMPP Servers',
                                    targetPythonClass='Products.ZenModel.Device')
                                
        tpl.manage_addRRDDataSource('kannel', 'KannelDataSource.KannelDataSource')

        dsk = tpl.datasources.kannel
        map(lambda x: dsk.manage_addRRDDataPoint(x), DATAPOINTS)

        # hmmm - recv/sent are accumulated totals, we just want to track the diffs
        for dp_name in ('recv', 'sent'):
            dp = dsk.datapoints._getOb(dp_name)
            if dp.rrdtype != 'DERIVE':
                # hmmm - we seem to be required to provide a max if we provide a min
                dp.manage_changeProperties(rrdtype='DERIVE', rrdmin='0', rrdmax='10000000')

        gdq = dsk.manage_addGraphDefinition('Kannel SMSC Queue')
        setDataPoints(gdq, 'kannel',  GRAPHPOINTS['smsc'])

        gds = dsk.manage_addGraphDefinition('Kannel Store Size')
        setDataPoints(gds, 'kannel', GRAPHPOINTS['store'])

    addZenPackObjects(zenpack, (zport.dmd.Events.Status.Kannel, tpls.KannelServer))

def uninstall(zport):
    
    for parent, id in ((zport.dmd.Events.Status, 'Kannel'),
                       (zport.dmd.Devices.Server.rrdTemplates, 'KannelServer')):
        if getattr(aq_base(parent), id, None):
            parent._delObject(id)
                       
