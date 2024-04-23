#include "CommonConfigDialog.h"
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>

CommonConfigItemList& CommonConfigItemList::addLabel(const QString& description) {
    CommonConfigItem item;
    item.description = description;
    item.type = CommonConfigItem::Label;
    append(item);
    return *this;
}

CommonConfigItemList& CommonConfigItemList::addString(const QString& key, const QString& description, const QString& tip, const QString& defaultValue) {
    CommonConfigItem item;
    item.key = key;
    item.description = description;
    item.tip = tip;
    item.value = defaultValue;
    append(item);
    return *this;
}

CommonConfigItemList& CommonConfigItemList::addInt(const QString& key, const QString& description, const QString& tip, int defaultValue, int min, int max) {
    CommonConfigItem item;
    item.key = key;
    item.description = description;
    item.tip = tip;
    item.value = defaultValue;
    item.min = min;
    item.max = max;
    item.type = CommonConfigItem::Int;
    append(item);
    return *this;
}

CommonConfigItemList& CommonConfigItemList::addDouble(const QString& key, const QString& description, const QString& tip, double defaultValue, double min, double max) {
    CommonConfigItem item;
    item.key = key;
    item.description = description;
    item.tip = tip;
    item.value = defaultValue;
    item.min = min;
    item.max = max;
    item.type = CommonConfigItem::Double;
    append(item);
    return *this;
}

CommonConfigItemList& CommonConfigItemList::addBool(const QString& key, const QString& description, const QString& tip, bool defaultValue) {
    CommonConfigItem item;
    item.key = key;
    item.description = description;
    item.tip = tip;
    item.value = defaultValue;
    item.type = CommonConfigItem::Bool;
    append(item);
    return *this;
}

CommonConfigItemList& CommonConfigItemList::addEnum(const QString& key, const QString& description, const QString& tip, const QList<QString>& enumList, const QString& defaultValue) {
    CommonConfigItem item;
    item.key = key;
    item.description = description;
    item.tip = tip;
    item.value = defaultValue;
    item.enumList = enumList;
    item.type = CommonConfigItem::Enum;
    append(item);
    return *this;
}

bool CommonConfigItemList::execDialog(QWidget* parent, const QString& title, std::function<void(CommonConfigItemList&)> onAccepted, std::function<void(CommonConfigItem&)> onEdited, bool hasDialogButton) {
    QDialog dialog(parent);
    dialog.setWindowTitle(title.isEmpty() ? "Common Config" : title);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    for (CommonConfigItem& item : *this) {
        if (item.type == CommonConfigItem::Label) {
            QLabel* label = new QLabel(item.description, &dialog);
            layout->addWidget(label);
        }
        else if (item.type == CommonConfigItem::String) {
            QLabel* label = new QLabel(item.description, &dialog);
            layout->addWidget(label);
            QLineEdit* edit = new QLineEdit(item.value.toString(), &dialog);
            edit->setToolTip(item.tip);
            layout->addWidget(edit);
            QObject::connect(edit, &QLineEdit::textChanged, [&item, onEdited](const QString& text) {
                item.value = text;
                if (onEdited) {
                    onEdited(item);
                }
                });
        }
        else if (item.type == CommonConfigItem::Int) {
            QLabel* label = new QLabel(item.description, &dialog);
            layout->addWidget(label);
            QSpinBox* spin = new QSpinBox(&dialog);
            spin->setToolTip(item.tip);
            spin->setRange(item.min.toInt(), item.max.toInt());
            spin->setValue(item.value.toInt());
            layout->addWidget(spin);
            QObject::connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), [&item, onEdited](int value) {
                item.value = value;
                if (onEdited) {
                    onEdited(item);
                }
                });
        }
        else if (item.type == CommonConfigItem::Double) {
            QLabel* label = new QLabel(item.description, &dialog);
            layout->addWidget(label);
            QDoubleSpinBox* spin = new QDoubleSpinBox(&dialog);
            spin->setToolTip(item.tip);
            spin->setRange(item.min.toDouble(), item.max.toDouble());
            spin->setValue(item.value.toDouble());
            layout->addWidget(spin);
            QObject::connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&item, onEdited](double value) {
                item.value = value;
                if (onEdited) {
                    onEdited(item);
                }
                });
        }
        else if (item.type == CommonConfigItem::Bool) {
            QCheckBox* check = new QCheckBox(item.description, &dialog);
            check->setToolTip(item.tip);
            check->setChecked(item.value.toBool());
            layout->addWidget(check);
            QObject::connect(check, &QCheckBox::toggled, [&item, onEdited](bool checked) {
                item.value = checked;
                if (onEdited) {
                    onEdited(item);
                }
                });
        }
        else if (item.type == CommonConfigItem::Enum) {
            QLabel* label = new QLabel(item.description, &dialog);
            layout->addWidget(label);
            QComboBox* combo = new QComboBox(&dialog);
            combo->setToolTip(item.tip);
            for (const QString& enumItem : item.enumList) {
                combo->addItem(enumItem);
            }
            combo->setCurrentText(item.value.toString());
            layout->addWidget(combo);
            QObject::connect(combo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), [&item, onEdited](const QString& text) {
                item.value = text;
                if (onEdited) {
                    onEdited(item);
                }
                });
        }
    }

    if (hasDialogButton) {
        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
        layout->addWidget(buttonBox);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, [&dialog, &onAccepted, this]() {
            if (onAccepted) {
                onAccepted(*this);
            }
            dialog.accept();
            });
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, [&dialog]() {
            dialog.reject();
            });
    }

    dialog.setLayout(layout);
    dialog.setMinimumSize(200, 200);

    return dialog.exec() == QDialog::Accepted;
}
