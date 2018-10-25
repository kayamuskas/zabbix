#
# This software is subject to the provisions of the Zope Public License,
# Version 2.1 (ZPL).  A copy of the ZPL should accompany this distribution.
#
import logging
from Products.ZenModel.BasicDataSource import BasicDataSource
from Products.ZenModel.ZenPackPersistence import ZenPackPersistence
from AccessControl import Permissions

from ZenPacks.oie.KannelMonitor.config import DATAPOINTS

LOG = logging.getLogger('KannelDataSource')

class KannelDataSource(ZenPackPersistence, BasicDataSource):
    """
    A command-plugin that calls the check_kannel nagios plugin to
    get input queue and throughput statistics from an SMPP bearer box.
    """
    KANNEL_MONITOR = 'KannelMonitor'
    ZENPACKID = 'ZenPacks.oie.KannelMonitor'

    sourcetypes = (KANNEL_MONITOR, 'COMMAND')
    #sourcetype = KANNEL_MONITOR
    sourcetype = 'COMMAND'

    eventClass = '/Status/Kannel'
        
    kannelServer = '${dev/id}'
    plugin = '${here/zKannelPlugin}'
    port = '${here/zKannelPort | string:13000}'

    # these are 'globals' because we cannot dynamically work them out here when
    # assembling the command ...
    secure = False
    password = ''
    timeout = 15

    parser = 'ZenPacks.oie.KannelMonitor.parsers.Kannel'

    _properties = BasicDataSource._properties + (
        {'id':'timeout',     'type':'int',       'mode':'w'},
        {'id':'plugin',      'type':'string',    'mode':'w'},
        {'id':'secure',      'type':'boolean',   'mode':'w'},
        {'id':'password',    'type':'string',    'mode':'w'},
        {'id':'port',        'type':'string',    'mode':'w'},
        )
        
    _relations = BasicDataSource._relations + (
        )

    factory_type_information = ( 
    { 
        'immediate_view' : 'editKannelMonitorDataSource',
        'actions'        :
        ( 
            { 'id'            : 'edit',
              'name'          : 'Data Source',
              'action'        : 'editKannelMonitorDataSource',
              'permissions'   : ( Permissions.view, ),
            },
        )
    },
    )


    def __init__(self, id, title=None, buildRelations=True):
        BasicDataSource.__init__(self, id, title, buildRelations)
        #self.addDataPoints()


    def getDescription(self):
        """ DEBUGSTR """
        if self.sourcetype == self.KANNEL_MONITOR:
            return self._cmd()
        return BasicDataSource.getDescription(self)


    def useZenCommand(self):
        return True

    def _cmd(self):
        cmd = self.plugin
        if self.timeout:
            cmd += ' --timeout=%s' % self.timeout
        if self.secure:
            cmd += ' --secure'
        if self.password:
            cmd += ' --password=%s' % self.password
        if self.port:
            cmd += ' --port=%s' % self.port
        return cmd

    def getCommand(self, context):
        """ use check_kannel to retrieve status info """

        return BasicDataSource.getCommand(self, context, self._cmd())


    def checkCommandPrefix(self, context, cmd):
        return cmd


    def addDataPoints(self):
        for tag in DATAPOINTS:
            if not hasattr(self.datapoints, tag):
                self.manage_addRRDDataPoint(tag)

    def zmanage_editProperties(self, REQUEST=None):
        '''validation, etc'''
        if REQUEST:
            # ensure default datapoint didn't go away
            self.addDataPoints()
            # and eventClass
            if not REQUEST.form.get('eventClass', None):
                REQUEST.form['eventClass'] = self.__class__.eventClass
        return BasicDataSource.zmanage_editProperties(self, REQUEST)





