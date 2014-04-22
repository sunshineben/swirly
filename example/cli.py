#!/usr/bin/env python

from contextlib import contextmanager
from dbrpy import *
from threading import Thread
from uuid import *

import os, shlex, sys

TMOUT = 5000

def parse(lex, obj):
    toks = []
    while not obj.is_closed:
        tok = lex.get_token()
        if tok == lex.eof:
            log_info('end-of-file')
            break
        if tok == ';':
            if len(toks) > 0:
                try:
                    fn = getattr(obj, toks[0])
                    fn(*toks[1:])
                except Exception as e:
                    log_error('error: ' + str(e))
                toks = []
        else:
            toks.append(tok)

class RequestHandler(Handler):
    def __init__(self, clnt):
        super(RequestHandler, self).__init__()
        self.clnt = clnt

    def on_close(self):
        log_notice('on_close')

    def on_ready(self):
        log_notice('on_ready')

    def on_logon(self, req_id, uid):
        log_notice('on_logon: {0}: {1}'.format(req_id, uid))

    def on_logoff(self, req_id, uid):
        log_notice('on_logoff: {0}: {1}'.format(req_id, uid))

    def on_reset(self):
        log_notice('on_reset')

    def on_timeout(self, req_id):
        log_notice('on_timeout: {0}'.format(req_id))

    def on_status(self, req_id, num, msg):
        log_notice('on_status: {0}: {2} ({1})'.format(req_id, num, msg))

    def on_exec(self, req_id, exc):
        log_notice('on_exec: {0}: {1}'.format(req_id, exc))

    def on_posn(self, posn):
        log_notice('on_posn: {0}'.format(posn))

    def on_view(self, view):
        log_notice('on_view: {0}'.format(view))

    def on_flush(self):
        pass

    def on_async(self, fn):
        ret = None
        try:
            ret = fn(self.clnt)
        except Exception as e:
            ret = e
        return ret

def worker(ctx, uuid):
    log_setlevel(3)
    pool = Pool(8 * 1024 * 1024)
    clnt = Clnt(ctx, uuid, 'tcp://localhost:3270', 'tcp://localhost:3271',
                millis(), TMOUT, pool)
    handler = RequestHandler(clnt)
    while True:
        try:
            if not clnt.dispatch(TMOUT, handler):
                break
        except Error as e:
            log_error('error: ' + str(e))

class CloseRequest(object):
    def __call__(self, clnt):
        return clnt.close()

class AccntsRequest(object):
    @staticmethod
    def to_dict(arec):
        return {
            'id': arec.id,
            'mnem': arec.mnem,
            'display': arec.display,
            'email': arec.email
        }
    def __init__(self, *mnems):
        self.mnems = set(mnems)
    def __call__(self, clnt):
        accnts = []
        if self.mnems:
            for mnem in self.mnems:
                arec = clnt.find_rec_mnem(ENTITY_ACCNT, mnem)
                if not arec:
                    err_set(EINVAL, "no such account '{0}'".format(mnem))
                    raise Error()
                accnts.append(AccntsRequest.to_dict(arec))
        else:
            accnts = [AccntsRequest.to_dict(arec)
                      for arec in clnt.list_rec(ENTITY_ACCNT)]
        return accnts

class ContrsRequest(object):
    @staticmethod
    def to_dict(crec):
        return {
            'id': crec.id,
            'mnem': crec.mnem,
            'display': crec.display,
            'asset_type': crec.asset_type,
            'asset': crec.asset,
            'ccy': crec.ccy,
            'tick_numer': crec.tick_numer,
            'tick_denom': crec.tick_denom,
            'lot_numer': crec.lot_numer,
            'lot_denom': crec.lot_denom,
            'price_dp': crec.price_dp,
            'pip_dp': crec.pip_dp,
            'qty_dp': crec.qty_dp,
            'min_lots': crec.min_lots,
            'max_lots': crec.max_lots
        }
    def __init__(self, *mnems):
        self.mnems = set(mnems)
    def __call__(self, clnt):
        contrs = []
        if self.mnems:
            for mnem in self.mnems:
                crec = clnt.find_rec_mnem(ENTITY_CONTR, mnem)
                if not crec:
                    err_set(EINVAL, "no such contract '{0}'".format(mnem))
                    raise Error()
                contrs.append(ContrsRequest.to_dict(crec))
        else:
            contrs = [ContrsRequest.to_dict(crec)
                      for crec in clnt.list_rec(ENTITY_CONTR)]
        return contrs

class LogonRequest(object):
    def __init__(self, umnem):
        self.umnem = umnem
    def __call__(self, clnt):
        urec = clnt.find_rec_mnem(ENTITY_ACCNT, self.umnem)
        if not urec:
            err_set(EINVAL, "no such account '{0}'".format(self.umnem))
            raise Error()
        return clnt.logon(urec)

class LogoffRequest(object):
    def __init__(self, umnem):
        self.umnem = umnem
    def __call__(self, clnt):
        urec = clnt.find_rec_mnem(ENTITY_ACCNT, self.umnem)
        if not urec:
            err_set(EINVAL, "no such account '{0}'".format(self.umnem))
            raise Error()
        return clnt.logoff(urec)

class UsersRequest(object):
    @staticmethod
    def to_dict(memb):
        return {
            'uid': memb.uid,
            'gid': memb.gid
        }
    def __init__(self, umnem):
        self.umnem = umnem
    def __call__(self, clnt):
        membs = []
        urec = clnt.find_rec_mnem(ENTITY_ACCNT, self.umnem)
        if not urec:
            err_set(EINVAL, "no such account '{0}'".format(self.umnem))
            raise Error()
        accnt = clnt.accnt(urec)
        membs = [UsersRequest.to_dict(memb)
                 for memb in accnt.list_user()]
        return membs

class GroupsRequest(object):
    @staticmethod
    def to_dict(memb):
        return {
            'uid': memb.uid,
            'gid': memb.gid
        }
    def __init__(self, umnem):
        self.umnem = umnem
    def __call__(self, clnt):
        membs = []
        urec = clnt.find_rec_mnem(ENTITY_ACCNT, self.umnem)
        if not urec:
            err_set(EINVAL, "no such account '{0}'".format(self.umnem))
            raise Error()
        accnt = clnt.accnt(urec)
        membs = [GroupsRequest.to_dict(memb)
                 for memb in accnt.list_group()]
        return membs

class OrdersRequest(object):
    @staticmethod
    def to_dict(order):
        return {
            'id': order.id,
            'uid': order.uid,
            'gid': order.gid,
            'cid': order.cid,
            'settl_date': order.settl_date,
            'ref': order.ref,
            'state': order.state,
            'action': order.action,
            'ticks': order.ticks,
            'lots': order.lots,
            'resd': order.resd,
            'exc': order.exc,
            'last_ticks': order.last_ticks,
            'last_lots': order.last_lots,
            'min_lots': order.min_lots,
            'created': order.created,
            'modified': order.modified
        }
    def __init__(self, umnem):
        self.umnem = umnem
    def __call__(self, clnt):
        orders = []
        urec = clnt.find_rec_mnem(ENTITY_ACCNT, self.umnem)
        if not urec:
            err_set(EINVAL, "no such account '{0}'".format(self.umnem))
            raise Error()
        accnt = clnt.accnt(urec)
        orders = [OrdersRequest.to_dict(order)
                  for order in accnt.list_order()]
        return orders

class TradesRequest(object):
    @staticmethod
    def to_dict(exc):
        return {
            'id': exc.id,
            'order': exc.order,
            'uid': exc.uid,
            'gid': exc.gid,
            'cid': exc.cid,
            'settl_date': exc.settl_date,
            'ref': exc.ref,
            'state': exc.state,
            'action': exc.action,
            'ticks': exc.ticks,
            'lots': exc.lots,
            'resd': exc.resd,
            'exc': exc.exc,
            'last_ticks': exc.last_ticks,
            'last_lots': exc.last_lots,
            'min_lots': exc.min_lots,
            'match': exc.match,
            'role': exc.role,
            'cpty': exc.cpty,
            'created': exc.created
        }
    def __init__(self, umnem):
        self.umnem = umnem
    def __call__(self, clnt):
        trades = []
        urec = clnt.find_rec_mnem(ENTITY_ACCNT, self.umnem)
        if not urec:
            err_set(EINVAL, "no such account '{0}'".format(self.umnem))
            raise Error()
        accnt = clnt.accnt(urec)
        trades = [TradesRequest.to_dict(exc)
                  for exc in accnt.list_trade()]
        return trades

class PosnsRequest(object):
    @staticmethod
    def to_dict(posn):
        return {
            'gid': posn.gid,
            'cid': posn.cid,
            'settl_date': posn.settl_date,
            'buy_licks': posn.buy_licks,
            'buy_lots': posn.buy_lots,
            'sell_licks': posn.sell_licks,
            'sell_lots': posn.sell_lots
        }
    def __init__(self, umnem):
        self.umnem = umnem
    def __call__(self, clnt):
        posns = []
        urec = clnt.find_rec_mnem(ENTITY_ACCNT, self.umnem)
        if not urec:
            err_set(EINVAL, "no such account '{0}'".format(self.umnem))
            raise Error()
        accnt = clnt.accnt(urec)
        posns = [PosnsRequest.to_dict(posn)
                 for posn in accnt.list_posn()]
        return posns

class ViewsRequest(object):
    @staticmethod
    def level_dict(level):
        return {
            'ticks': level.ticks,
            'lots': level.lots,
            'count': level.count
        }
    @staticmethod
    def to_dict(view):
        return {
            'cid': view.cid,
            'settl_date': view.settl_date,
            'list_bid': [ViewsRequest.level_dict(level) for level in view.list_bid],
            'list_offer': [ViewsRequest.level_dict(level) for level in view.list_offer]
        }
    def __init__(self, mnems, settl_dates):
        self.mnems = set(mnems)
        self.settl_dates = {int(x) for x in settl_dates}
    def __call__(self, clnt):
        cids = set()
        for mnem in self.mnems:
            crec = clnt.find_rec_mnem(ENTITY_CONTR, mnem)
            if crec:
                cids.add(crec.id)
        views = []
        if cids:
            if self.settl_dates:
                # Cids and settl_dates.
                for cid in cids:
                    for settl_date in self.settl_dates:
                        view = clnt.find_view(crec.id, settl_date)
                        if view:
                            views.append(ViewsRequest.to_dict(view))
            else:
                # Cids only.
                views = [ViewsRequest.to_dict(view) for view in clnt.list_view()
                         if view.cid in cids]
        else:
            if self.settl_dates:
                # Settl_dates only.
                views = [ViewsRequest.to_dict(view) for view in clnt.list_view()
                         if view.settl_date in self.settl_dates]
            else:
                # Neither cids nor settl_dates.
                views = [ViewsRequest.to_dict(view) for view in clnt.list_view()]
        return views

class PlaceRequest(object):
    def __init__(self, umnem, gmnem, cmnem, settl_date, ref, action, ticks, lots, min_lots):
        self.umnem = umnem
        self.gmnem = gmnem
        self.cmnem = cmnem
        self.settl_date = settl_date
        self.ref = ref
        self.action = action
        self.ticks = ticks
        self.lots = lots
        self.min_lots = min_lots
    def __call__(self, clnt):
        urec = clnt.find_rec_mnem(ENTITY_ACCNT, self.umnem)
        if not urec:
            err_set(EINVAL, "no such account '{0}'".format(self.umnem))
            raise Error()
        user = clnt.accnt(urec)
        grec = clnt.find_rec_mnem(ENTITY_ACCNT, self.gmnem)
        if not grec:
            err_set(EINVAL, "no such account '{0}'".format(self.gmnem))
            raise Error()
        group = clnt.accnt(grec)
        crec = clnt.find_rec_mnem(ENTITY_CONTR, self.cmnem)
        if not crec:
            err_set(EINVAL, "no such contract '{0}'".format(self.cmnem))
            raise Error()
        return clnt.place(user, group, crec, self.settl_date, self.ref,
                          self.action, self.ticks, self.lots, self.min_lots)

class ReviseRequest(object):
    def __init__(self, umnem, id, lots):
        self.umnem = umnem
        self.id = id
        self.lots = lots

    def __call__(self, clnt):
        urec = clnt.find_rec_mnem(ENTITY_ACCNT, self.umnem)
        if not urec:
            err_set(EINVAL, "no such account '{0}'".format(self.umnem))
            raise Error()
        user = clnt.accnt(urec)
        return clnt.revise_id(user, self.id, self.lots)

class CancelRequest(object):
    def __init__(self, umnem, *ids):
        self.umnem = umnem
        self.ids = set(ids)

    def __call__(self, clnt):
        urec = clnt.find_rec_mnem(ENTITY_ACCNT, self.umnem)
        if not urec:
            err_set(EINVAL, "no such account '{0}'".format(self.umnem))
            raise Error()
        user = clnt.accnt(urec)
        arr = []
        for id in self.ids:
            arr.append(clnt.cancel_id(user, int(id)))
        return arr

class AckTradesRequest(object):
    def __init__(self, umnem, *ids):
        self.umnem = umnem
        self.ids = set(ids)

    def __call__(self, clnt):
        urec = clnt.find_rec_mnem(ENTITY_ACCNT, self.umnem)
        if not urec:
            err_set(EINVAL, "no such account '{0}'".format(self.umnem))
            raise Error()
        user = clnt.accnt(urec)
        arr = []
        for id in self.ids:
            arr.append(clnt.ack_trade(user, int(id)))
        return arr

class AsyncClnt(object):
    def __init__(self):
        uuid = uuid1()
        log_notice('uuid: {0}'.format(uuid))
        self.env = {}
        self.ctx = ZmqCtx()
        self.thread = Thread(target = worker, args = (self.ctx, uuid))
        self.thread.start()
        self.async = Async(self.ctx, uuid)
        self.is_closed = False

    def close(self):
        if not self.is_closed:
            self.async.sendAndRecv(CloseRequest())
            log_notice('joining')
            self.thread.join()
            self.is_closed = True

    def accnts(self, *mnems):
        print(self.async.sendAndRecv(AccntsRequest(*mnems)))

    def contrs(self, *mnems):
        print(self.async.sendAndRecv(ContrsRequest(*mnems)))

    def logon(self):
        umnem = self.env['user']
        self.async.sendAndRecv(LogonRequest(umnem))

    def logoff(self):
        umnem = self.env['user']
        self.async.sendAndRecv(LogoffRequest(umnem))

    def users(self):
        umnem = self.env['user']
        print(self.async.sendAndRecv(UsersRequest(umnem)))

    def groups(self):
        umnem = self.env['user']
        print(self.async.sendAndRecv(GroupsRequest(umnem)))

    def orders(self):
        umnem = self.env['user']
        print(self.async.sendAndRecv(OrdersRequest(umnem)))

    def trades(self):
        umnem = self.env['user']
        print(self.async.sendAndRecv(TradesRequest(umnem)))

    def posns(self):
        umnem = self.env['user']
        print(self.async.sendAndRecv(PosnsRequest(umnem)))

    def view(self):
        log_notice('view')

    def buy(self, ticks, lots):
        umnem = self.env['user']
        gmnem = self.env['group']
        cmnem = self.env['contr']
        settl_date = int(self.env['settl_date'])
        print(self.async.sendAndRecv(PlaceRequest(umnem, gmnem, cmnem, settl_date, '',
                                                  ACTION_BUY, int(ticks), int(lots), 0)))

    def sell(self, ticks, lots):
        umnem = self.env['user']
        gmnem = self.env['group']
        cmnem = self.env['contr']
        settl_date = int(self.env['settl_date'])
        print(self.async.sendAndRecv(PlaceRequest(umnem, gmnem, cmnem, settl_date, '',
                                                  ACTION_SELL, int(ticks), int(lots), 0)))

    def revise(self, id, lots):
        umnem = self.env['user']
        print(self.async.sendAndRecv(ReviseRequest(umnem, id, lots)))

    def cancel(self, *ids):
        umnem = self.env['user']
        print(self.async.sendAndRecv(CancelRequest(umnem, *ids)))

    def ack_trades(self, *args):
        umnem = self.env['user']
        print(self.async.sendAndRecv(AckTradesRequest(umnem, *ids)))

    def echo(self, *args):
        log_notice('echo: ' + ','.join(args))

    def penv(self):
        for k, v in self.env.iteritems():
            print('{0}: {1}'.format(k, v))

    def set(self, name, value):
        self.env[name] = value

    def unset(self, name):
        del self.env[name]

    def quit(self):
        self.close()

    def test(self):
        self.buy('WRAMIREZ', 'DBRA', 'EURUSD', 20140420, '', 12345, 1, 0)

@contextmanager
def async_clnt():
    ac = AsyncClnt()
    try:
        yield ac
    finally:
        log_notice('closing')
        ac.close()

if __name__ == '__main__':
    try:
        lex = shlex.shlex(sys.stdin)
        home = os.environ['HOME']
        rcfile = os.path.join(home, '.dbr_clirc')
        if os.path.isfile(rcfile):
            lex.push_source(open(rcfile, 'r'), rcfile)
        with async_clnt() as ac:
            parse(lex, ac)
        log_notice('exiting')
    except KeyboardInterrupt as e:
        log_warn('interrupted')
    except Error as e:
        log_error('error: ' + str(e))
