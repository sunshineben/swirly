/*
 * The Restful Matching-Engine.
 * Copyright (C) 2013, 2018 Swirly Cloud Limited.
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include "Order.hpp"

#include <swirly/util/Date.hpp>
#include <swirly/util/Stream.hpp>

namespace swirly {
inline namespace fin {
using namespace std;

static_assert(sizeof(Order) <= 6 * 64, "no greater than specified cache-lines");

Order::~Order() = default;

Order::Order(Order&&) = default;

void Order::toDsv(ostream& os, char delim) const
{
    OStreamJoiner osj{os, delim};
    osj << accnt_    //
        << marketId_ //
        << instr_;
    if (settlDay_ != 0_jd) {
        osj << jdToIso(settlDay_);
    } else {
        osj << ""sv;
    }
    osj << id_;
    if (!ref_.empty()) {
        osj << ref_;
    } else {
        osj << ""sv;
    }
    osj << state_    //
        << side_     //
        << lots_     //
        << ticks_    //
        << resdLots_ //
        << execLots_ //
        << execCost_;
    if (lastLots_ != 0_lts) {
        osj << lastLots_ //
            << lastTicks_;
    } else {
        osj << ""sv
            << ""sv;
    }
    if (minLots_ != 0_lts) {
        osj << minLots_;
    } else {
        osj << ""sv;
    }
    osj << created_ //
        << modified_;
}

void Order::toJson(ostream& os) const
{
    os << "{\"accnt\":\""sv << accnt_        //
       << "\",\"market_id\":"sv << marketId_ //
       << ",\"instr\":\""sv << instr_        //
       << "\",\"settl_date\":"sv;
    if (settlDay_ != 0_jd) {
        os << jdToIso(settlDay_);
    } else {
        os << "null"sv;
    }
    os << ",\"id\":"sv << id_ //
       << ",\"ref\":"sv;
    if (!ref_.empty()) {
        os << '"' << ref_ << '"';
    } else {
        os << "null"sv;
    }
    os << ",\"state\":\""sv << state_      //
       << "\",\"side\":\""sv << side_      //
       << "\",\"lots\":"sv << lots_        //
       << ",\"ticks\":"sv << ticks_        //
       << ",\"resd_lots\":"sv << resdLots_ //
       << ",\"exec_lots\":"sv << execLots_ //
       << ",\"exec_cost\":"sv << execCost_;
    if (lastLots_ != 0_lts) {
        os << ",\"last_lots\":"sv << lastLots_ //
           << ",\"last_ticks\":"sv << lastTicks_;
    } else {
        os << ",\"last_lots\":null,\"last_ticks\":null"sv;
    }
    os << ",\"min_lots\":"sv;
    if (minLots_ != 0_lts) {
        os << minLots_;
    } else {
        os << "null"sv;
    }
    os << ",\"created\":"sv << created_   //
       << ",\"modified\":"sv << modified_ //
       << '}';
}

OrderRefSet::~OrderRefSet()
{
    set_.clear_and_dispose([](const Order* ptr) { ptr->release(); });
}

OrderRefSet::OrderRefSet(OrderRefSet&&) = default;

OrderRefSet& OrderRefSet::operator=(OrderRefSet&&) = default;

OrderRefSet::Iterator OrderRefSet::insert(const ValuePtr& value) noexcept
{
    Iterator it;
    bool inserted;
    tie(it, inserted) = set_.insert(*value);
    if (inserted) {
        // Take ownership if inserted.
        value->addRef();
    }
    return it;
}

OrderRefSet::Iterator OrderRefSet::insertHint(ConstIterator hint, const ValuePtr& value) noexcept
{
    auto it = set_.insert(hint, *value);
    // Take ownership.
    value->addRef();
    return it;
}

OrderRefSet::Iterator OrderRefSet::insertOrReplace(const ValuePtr& value) noexcept
{
    Iterator it;
    bool inserted;
    tie(it, inserted) = set_.insert(*value);
    if (!inserted) {
        // Replace if exists.
        ValuePtr prev{&*it, false};
        set_.replace_node(it, *value);
        it = Set::s_iterator_to(*value);
    }
    // Take ownership.
    value->addRef();
    return it;
}

OrderList::~OrderList()
{
    list_.clear_and_dispose([](const Order* ptr) { ptr->release(); });
}

OrderList::OrderList(OrderList&&) = default;

OrderList& OrderList::operator=(OrderList&&) = default;

OrderList::Iterator OrderList::insertBack(const OrderPtr& value) noexcept
{
    list_.push_back(*value);
    // Take ownership.
    value->addRef();
    return List::s_iterator_to(*value);
}

OrderList::Iterator OrderList::insertBefore(const OrderPtr& value, const Order& next) noexcept
{
    auto it = list_.insert(List::s_iterator_to(next), *value);
    // Take ownership.
    value->addRef();
    return it;
}

OrderList::ValuePtr OrderList::remove(const Order& ref) noexcept
{
    ValuePtr value;
    list_.erase_and_dispose(List::s_iterator_to(ref), [&value](Order* ptr) {
        value = ValuePtr{ptr, false};
    });
    return value;
}

} // namespace fin
} // namespace swirly
