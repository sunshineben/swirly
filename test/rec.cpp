/*
 *  Copyright (C) 2013 Mark Aylett <mark.aylett@gmail.com>
 *
 *  This file is part of Doobry written by Mark Aylett.
 *
 *  Doobry is free software; you can redistribute it and/or modify it under the terms of the GNU
 *  General Public License as published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  Doobry is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program; if
 *  not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301 USA.
 */
#include "journ.hpp"
#include "model.hpp"
#include "test.hpp"

#include <dbrpp/ctx.hpp>
#include <dbrpp/pool.hpp>

#include <dbr/conv.h>

using namespace dbr;

template <int TypeN>
struct TypeTraits;

template <>
struct TypeTraits<DBR_CONTR> {
    typedef ContrRec TypeRec;
};

template <>
struct TypeTraits<DBR_TRADER> {
    typedef TraderRec TypeRec;
};

template <>
struct TypeTraits<DBR_ACCNT> {
    typedef AccntRec TypeRec;
};

template <int TypeN>
static typename TypeTraits<TypeN>::TypeRec
get_rec_id(Ctx& ctx, DbrIden id)
{
    auto it = ctx.recs<TypeN>().find(id);
    check(it != ctx.recs<TypeN>().end());
    return typename TypeTraits<TypeN>::TypeRec(*it);
}

template <int TypeN>
static typename TypeTraits<TypeN>::TypeRec
get_rec_mnem(Ctx& ctx, const char* mnem)
{
    auto it = ctx.recs<TypeN>().find(mnem);
    check(it != ctx.recs<TypeN>().end());
    return typename TypeTraits<TypeN>::TypeRec(*it);
}

TEST_CASE(find_contr)
{
    Pool pool;
    Journ journ(1);
    Model model(pool);
    Ctx ctx(pool, &journ, &model);

    check(ctx.crecs().find("BAD") == ctx.crecs().end());

    ContrRec crec = get_rec_mnem<DBR_CONTR>(ctx, "EURUSD");
    check(crec == get_rec_id<DBR_CONTR>(ctx, crec.id()));
    check(crec.mnem() == Mnem("EURUSD"));

    // Body.
    check(crec.display() == Display("EURUSD"));
    check(crec.asset_type() == Mnem("CURRENCY"));
    check(crec.asset() == Mnem("EUR"));
    check(crec.ccy() == Mnem("USD"));
    check(fequal(crec.price_inc(), 0.0001));
    check(fequal(crec.qty_inc(), 1e6));
    check(crec.price_dp() == 4);
    check(crec.pip_dp() == 4);
    check(crec.qty_dp() == 0);
    check(crec.min_lots() == 1);
    check(crec.max_lots() == 10);
}

TEST_CASE(find_trader)
{
    Pool pool;
    Journ journ(1);
    Model model(pool);
    Ctx ctx(pool, &journ, &model);

    check(ctx.trecs().find("BAD") == ctx.trecs().end());

    TraderRec trec = get_rec_mnem<DBR_TRADER>(ctx, "WRAMIREZ");
    check(trec == get_rec_id<DBR_TRADER>(ctx, trec.id()));
    check(trec.mnem() == Mnem("WRAMIREZ"));

    // Body.
    check(trec.display() == Display("Wayne Ramirez"));
    check(trec.email() == Email("wayne.ramirez@doobry.org"));
}

TEST_CASE(find_accnt)
{
    Pool pool;
    Journ journ(1);
    Model model(pool);
    Ctx ctx(pool, &journ, &model);

    check(ctx.arecs().find("BAD") == ctx.arecs().end());

    AccntRec arec = get_rec_mnem<DBR_ACCNT>(ctx, "DBRA");
    check(arec == get_rec_id<DBR_ACCNT>(ctx, arec.id()));
    check(arec.mnem() == Mnem("DBRA"));

    // Body.
    check(arec.display() == Display("Account A"));
    check(arec.email() == Email("dbra@doobry.org"));
}
