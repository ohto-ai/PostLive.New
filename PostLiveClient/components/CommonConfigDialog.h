#pragma once

#include <QDialog>
#include <QVariant>

struct CommonConfigItem {
    QString key;
    QVariant value;
    QString description;
    QString tip;

    QVariant min, max;
    QList<QString> enumList;

    enum Type {
        Label,
        String,
        Int,
        Double,
        Bool,
        Enum,
    } type = String;
};

struct CommonConfigItemList : public QList<CommonConfigItem> {
private:
public:
    CommonConfigItemList() = default;

    CommonConfigItemList& addLabel(const QString& description);

    CommonConfigItemList& addString(const QString& key, const QString& description, const QString& tip = "", const QString& defaultValue = "");

    CommonConfigItemList& addInt(const QString& key, const QString& description, const QString& tip = "", int defaultValue = 0, int min = INT_MIN, int max = INT_MAX);

    CommonConfigItemList& addDouble(const QString& key, const QString& description, const QString& tip = "", double defaultValue = 0, double min = -DBL_MAX, double max = DBL_MAX);

    CommonConfigItemList& addBool(const QString& key, const QString& description, const QString& tip = "", bool defaultValue = false);

    CommonConfigItemList& addEnum(const QString& key, const QString& description, const QString& tip = "", const QList<QString>& enumList = {}, const QString& defaultValue = "");

    bool execDialog(QWidget* parent, const QString& title, std::function<void(CommonConfigItemList&)> onAccepted = {}, std::function<void(CommonConfigItem&)> onEdited = {}, bool hasDialogButton = true);
};
