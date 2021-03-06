#include "mediawithicondelegate.h"

void MediaWithIconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString title = index.data(UserRoleTitle).toString();

    painter->save();
    QRect r = option.rect;

    if (index.data(UserRoleHeader).toBool()) {
        QColor activeColor = QMaemo5Style::standardColor("ActiveTextColor");
        painter->setPen(QPen(activeColor));
        painter->drawText(r, Qt::AlignVCenter|Qt::AlignCenter, title);
    }

    else {
        QString songLength = mmss_len(index.data(UserRoleSongDuration).toInt());

        if (option.state & QStyle::State_Selected)
            QStyledItemDelegate::paint(painter, option, QModelIndex());

        QFontMetrics fm(painter->font());

        int titleWidth;
        if (!songLength.isEmpty()) {
            r.setRight(r.right()-12);
            painter->drawText(r, Qt::AlignVCenter|Qt::AlignRight, songLength, &r);
            titleWidth = r.left();
            r = option.rect;
        } else
            titleWidth = r.width();

        int isz = option.decorationSize.width(); // icon size
        if (isz < 0) isz = 48;

        titleWidth = titleWidth - (15+isz+15) - 15;
        title = fm.elidedText(title, Qt::ElideRight, titleWidth);
        painter->drawText(15+isz+15, r.top(), titleWidth, r.height(), Qt::AlignVCenter|Qt::AlignLeft, title);

        if (index.data(Qt::DecorationRole).type() == QVariant::Icon)
            painter->drawPixmap(15, r.top()+(70-isz)/2, isz, isz,
                                qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(isz));
    }

    painter->restore();
}

QSize MediaWithIconDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(400, 70);
}
