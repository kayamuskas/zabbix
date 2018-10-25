#
# This software is subject to the provisions of the Zope Public License,
# Version 2.1 (ZPL).  A copy of the ZPL should accompany this distribution.
#
import os, logging, transaction
from lbn.zenoss import packutils
from ZenPacks.lbn.Base import ZenPack as ZenPackBase
from Products.CMFCore.DirectoryView import registerDirectory
from config import *
import setuphandlers

logger = logging.getLogger(PROJECTNAME)
logger.info('Installing KannelMonitor')

registerDirectory(SKINS_DIR, GLOBALS)

class ZenPack(ZenPackBase):
    """ Zenoss eggy thing """
    packZProperties = [
        ('zKannelPlugin', '/usr/lib64/nagios/plugins/contrib/check_kannel', 'string'),
        ('zKannelPort', '13000', 'int'),
        ]

    def install(self, zport):
        """
        Set the collector plugin
        """
        ZenPackBase.install(self, zport)
	setuphandlers.install(zport, self)

def initialize(context):
    """ Zope Product """
    
    zport = packutils.zentinel(context)
    if zport and not packutils.hasZenPack(zport, __name__):
        logger.info('Installing into ZenPackManager')
        transaction.begin()
	zpack = ZenPack(__name__)
        zpack = packutils.addZenPack(zport, zpack)
        transaction.commit()
    else:
        logger.info('Already in ZenPackManager')


