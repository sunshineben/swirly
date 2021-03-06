# The Restful Matching-Engine.
# Copyright (C) 2013, 2018 Swirly Cloud Limited.
#
# This program is free software; you can redistribute it and/or modify it under the terms of the
# GNU General Public License as published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with this program; if
# not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

from swift import *

class TestCase(RestTestCase):

  def test(self):
    self.maxDiff = None
    self.now = 1388534400000
    with DbFile() as dbFile:
      with Server(dbFile, self.now) as server:
        with Client() as client:
          client.setTime(self.now)

          self.checkAuth(client)

          self.createMarket(client)
          self.createMarketByInstr(client)
          self.createMarketByMarket(client)
          self.updateMarket(client)

  def checkAuth(self, client):
    client.setAuth(None, 0x1)

    resp = client.send('POST', '/markets')
    self.assertEqual(401, resp.status)
    self.assertEqual('Unauthorized', resp.reason)

    resp = client.send('POST', '/markets/USDJPY')
    self.assertEqual(401, resp.status)
    self.assertEqual('Unauthorized', resp.reason)

    resp = client.send('POST', '/markets/USDJPY/20140302')
    self.assertEqual(401, resp.status)
    self.assertEqual('Unauthorized', resp.reason)

    resp = client.send('PUT', '/markets/USDJPY/20140302')
    self.assertEqual(401, resp.status)
    self.assertEqual('Unauthorized', resp.reason)

    client.setAuth('ADMIN', ~0x1 & 0x7fffffff)

    resp = client.send('POST', '/markets')
    self.assertEqual(403, resp.status)
    self.assertEqual('Forbidden', resp.reason)

    resp = client.send('POST', '/markets/USDJPY')
    self.assertEqual(403, resp.status)
    self.assertEqual('Forbidden', resp.reason)

    resp = client.send('POST', '/markets/USDJPY/20140302')
    self.assertEqual(403, resp.status)
    self.assertEqual('Forbidden', resp.reason)

    resp = client.send('PUT', '/markets/USDJPY/20140302')
    self.assertEqual(403, resp.status)
    self.assertEqual('Forbidden', resp.reason)

  def createMarket(self, client):
    client.setAdmin()
    resp = client.send('POST', '/markets',
                       instr = 'EURUSD',
                       settl_date = 20140302,
                       state = 1)

    self.assertEqual(200, resp.status)
    self.assertEqual('OK', resp.reason)
    self.assertDictEqual({
      u'bid_count': [None, None, None],
      u'bid_lots': [None, None, None],
      u'bid_ticks': [None, None, None],
      u'instr': u'EURUSD',
      u'id': 82255,
      u'last_lots': None,
      u'last_ticks': None,
      u'last_time': None,
      u'offer_count': [None, None, None],
      u'offer_lots': [None, None, None],
      u'offer_ticks': [None, None, None],
      u'settl_date': 20140302,
      u'state': 1
    }, resp.content)

  def createMarketByInstr(self, client):
    client.setAdmin()
    resp = client.send('POST', '/markets/GBPUSD',
                       settl_date = 20140302,
                       state = 1)

    self.assertEqual(200, resp.status)
    self.assertEqual('OK', resp.reason)
    self.assertDictEqual({
      u'bid_count': [None, None, None],
      u'bid_lots': [None, None, None],
      u'bid_ticks': [None, None, None],
      u'instr': u'GBPUSD',
      u'id': 147791,
      u'last_lots': None,
      u'last_ticks': None,
      u'last_time': None,
      u'offer_count': [None, None, None],
      u'offer_lots': [None, None, None],
      u'offer_ticks': [None, None, None],
      u'settl_date': 20140302,
      u'state': 1
    }, resp.content)

  def createMarketByMarket(self, client):
    client.setAdmin()
    resp = client.send('POST', '/markets/USDJPY/20140302',
                       state = 1)

    self.assertEqual(200, resp.status)
    self.assertEqual('OK', resp.reason)
    self.assertDictEqual({
      u'bid_count': [None, None, None],
      u'bid_lots': [None, None, None],
      u'bid_ticks': [None, None, None],
      u'instr': u'USDJPY',
      u'id': 278863,
      u'last_lots': None,
      u'last_ticks': None,
      u'last_time': None,
      u'offer_count': [None, None, None],
      u'offer_lots': [None, None, None],
      u'offer_ticks': [None, None, None],
      u'settl_date': 20140302,
      u'state': 1
    }, resp.content)

  def updateMarket(self, client):
    client.setAdmin()
    resp = client.send('PUT', '/markets/USDJPY/20140302',
                       state = 2)
    self.assertEqual(200, resp.status)
    self.assertEqual('OK', resp.reason)
    self.assertDictEqual({
      u'bid_count': [None, None, None],
      u'bid_lots': [None, None, None],
      u'bid_ticks': [None, None, None],
      u'instr': u'USDJPY',
      u'id': 278863,
      u'last_lots': None,
      u'last_ticks': None,
      u'last_time': None,
      u'offer_count': [None, None, None],
      u'offer_lots': [None, None, None],
      u'offer_ticks': [None, None, None],
      u'settl_date': 20140302,
      u'state': 2
    }, resp.content)
