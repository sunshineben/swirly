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
#ifndef SWIRLYUI_MARKETMODEL_HXX
#define SWIRLYUI_MARKETMODEL_HXX

#include "Market.hxx"
#include "TableModel.hxx"

namespace swirly {
namespace ui {

class MarketModel
: public TableModel<Id64, Market, unbox(market::Column::CheckState), market::ColumnCount> {
  public:
    MarketModel(QObject* parent = nullptr);
    ~MarketModel() override;

    QVariant data(const QModelIndex& index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Market find(Id64 id) const;

    int indexOf(Id64 id) const;

    void removeRow(const Market& market) { TableModel::removeRow(market.id()); }
    void updateRow(std::uint64_t tag, const Market& market)
    {
        TableModel::updateRow(market.id(), tag, market);
    }

  private:
    QVariant header_[market::ColumnCount];
};

} // namespace ui
} // namespace swirly

#endif // SWIRLYUI_MARKETMODEL_HXX
