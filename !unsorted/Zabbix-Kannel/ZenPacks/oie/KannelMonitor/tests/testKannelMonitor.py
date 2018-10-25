# 
# This software is subject to the provisions of the Zope Public License,
# Version 2.1 (ZPL).  A copy of the ZPL should accompany this distribution.
#
import os, sys
if __name__ == '__main__':
    execfile(os.path.join(sys.path[0], 'framework.py'))

from Products.ZenTestCase.BaseTestCase import BaseTestCase

#from ZenPacks.oie.KannelMonitor import KannelMonitor

class TestKannelMonitor(BaseTestCase):
    def setUp(self):
        BaseTestCase.setUp(self)

    def testZenPack(self):
	from ZenPacks.oie.KannelMonitor import ZenPack
	self.failUnless(1)

    def testEventsSetup(self):
        self.failUnless(self.dmd.Events.Status.Kannel)

    def testTemplatesSetup(self):
        self.failUnless(self.dmd.Devices.rrdTemplates.KannelServer)

    def testCmd(self):
        device_class = self.dmd.Devices
	device = device_class.addDevice('test.box')
	datasource = device_class.rrdTemplates.KannelServer.datasources.kannel
	self.assertEqual(datasource.getCommand(device), '')


if __name__ == '__main__':
    framework()
else:
    def test_suite():
        from unittest import TestSuite, makeSuite
        suite = TestSuite()
        suite.addTest(makeSuite(TestKannelMonitor))
        return suite
