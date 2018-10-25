#
# This software is subject to the provisions of the Zope Public License,
# Version 2.1 (ZPL).  A copy of the ZPL should accompany this distribution.
#
import os, sys
if __name__ == '__main__':
    execfile(os.path.join(sys.path[0], 'framework.py'))

import pkg_resources
pkg_resources.get_distribution('ZenPacks.oie.KannelMonitor')

from Products.ZenTestCase.BaseTestCase import BaseTestCase
from Products.ZenRRD.tests.BaseParsersTestCase import Object
from Products.ZenRRD.CommandParser import ParsedResults
from ZenPacks.oie.KannelMonitor.parsers import Kannel

#
# normal check_kannel and our extended check_kannel feature(s)
#

RESULT = "Kannel OK - running, uptime 17d 21h 43m 50s, 1/1 SMSBOX, 1/1 WAPBOX, 12/13 SMSC, 10/100 SMS queued in/out, 0 DLR queued (internal), store-file 123456 bytes"

RESULT_X = "Kannel OK - running, uptime 19d 20h 12m 57s, 1/1 SMSBOX, 1/1 WAPBOX, 12/13 SMSC, 10/100 SMS queued in/out, 16/6701 SMS delivered in/out, 0 DLR queued (internal), store-file 123456 bytes"


class TestKannelParser(BaseTestCase):

    def setUp(self):
        self.cmd = Object()
        deviceConfig = Object()
        deviceConfig.device = 'localhost'
        self.cmd.deviceConfig = deviceConfig

        self.cmd.parser = "Kannel"
        self.cmd.result = Object()
        self.cmd.result.exitCode = 2
        self.cmd.severity = 2
        self.cmd.command = "testKannelPlugin"
        self.cmd.eventKey = "kannelKey"
        self.cmd.eventClass = "/Cmd"
        self.cmd.component = "zencommand"
        self.parser = Kannel.Kannel()
        self.results = ParsedResults()
        self.dpdata = dict(processName='someJob a b c',
                           ignoreParams=False,
                           alertOnRestart=True,
                           failSeverity=3)

    def testRegExps(self):
        self.failUnless(Kannel.RecvSendQueue.search(RESULT))
        self.failIf(Kannel.RecvSendDeliver.search(RESULT))
        self.failUnless(Kannel.StoreSize.search(RESULT))

    def testRegExpsExtended(self):
        self.failUnless(Kannel.RecvSendQueue.search(RESULT_X))
        self.failUnless(Kannel.RecvSendDeliver.search(RESULT_X))
        self.failUnless(Kannel.StoreSize.search(RESULT_X))

    def testGood(self):
        p1 = Object()
        p1.id = 'recvqueue'
        p1.data = self.dpdata

        p2 = Object()
        p2.id = 'sentqueue'
        p2.data = self.dpdata

        p3 = Object()
        p3.id = 'storesize'
        p3.data = self.dpdata

        p4 = Object()
        p4.id = 'recv'
        p4.data = self.dpdata

        p5 = Object()
        p5.id = 'sent'
        p5.data = self.dpdata

        self.cmd.points = [p1, p2, p3, p4, p5]
        self.cmd.result.output = RESULT_X

        self.parser.processResults(self.cmd, self.results)

        #self.assertEquals( len(self.results.values), 3)
        self.assertEquals('recvqueue',  self.results.values[0][0].id)
        self.assertEquals(10,  self.results.values[0][1])
        self.assertEquals('sentqueue',  self.results.values[1][0].id)
        self.assertEquals(100,  self.results.values[1][1])
        self.assertEquals('storesize',  self.results.values[2][0].id)
        self.assertEquals(123456,  self.results.values[2][1])
        self.assertEquals('recv',  self.results.values[3][0].id)
        self.assertEquals(16,  self.results.values[3][1])
        self.assertEquals('sent',  self.results.values[4][0].id)
        self.assertEquals(6701,  self.results.values[4][1])



if __name__ == '__main__':
    framework()
else:
    def test_suite():
        from unittest import TestSuite, makeSuite
        suite = TestSuite()
        suite.addTest(makeSuite(TestKannelParser))
        return suite
