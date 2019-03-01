/*****************************************************************************
 * Copyright (C) 2003 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef KONFIGURATORITEMS_H
#define KONFIGURATORITEMS_H

// QtCore
#include <QObject>
#include <QString>
#include <QList>
// QtGui
#include <QPixmap>
#include <QFont>
// QtWidgets
#include <QBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QComboBox>

#include <KIOWidgets/KUrlRequester>

#include "../GUI/krlistwidget.h"

#define FIRST_PAGE        0

class KonfiguratorExtension : public QObject
{
    Q_OBJECT

public:
  KonfiguratorExtension(QObject *obj, QString cfgGroup, QString cfgName, bool restartNeeded = false,
                        int page = FIRST_PAGE);

  virtual void loadInitialValue();
  virtual bool apply();
  virtual void setDefaults();
  virtual bool isChanged();
  virtual void setSubPage(int page) { subpage = page; }
  virtual int subPage() { return subpage; }

  inline QObject *object() { return objectPtr; }

  inline QString getConfigGroup() { return configGroup; }
  inline QString getConfigName() { return configName; }

public slots:
    void    setChanged()               {
        emit sigChanged(changed = true);
    }
    void    setChanged(bool chg)     {
        emit sigChanged(changed = chg);
    }

signals:
    void    applyManually(QObject *, QString, QString);
    void    applyAuto(QObject *, QString, QString);
    void    setDefaultsManually(QObject *);
    void    setDefaultsAuto(QObject *);
    void    setInitialValue(QObject *);
    void    sigChanged(bool);

protected:
    QObject *objectPtr;

    bool    applyConnected;
    bool    setDefaultsConnected;
    bool    changed;
    bool    restartNeeded;
    int     subpage;

    QString configGroup;
    QString configName;

    virtual void connectNotify(const QMetaMethod &signal) Q_DECL_OVERRIDE;
};


// KonfiguratorCheckBox class
///////////////////////////////

class KonfiguratorCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    KonfiguratorCheckBox(QString configGroup, QString name, bool defaultValue, QString text,
                         QWidget *parent = 0, bool restart = false, int page = FIRST_PAGE);
    ~KonfiguratorCheckBox();

    inline KonfiguratorExtension *extension() { return ext; }

    // indicate that a checkobox is dependent of this,
    // meaning that dep is only available if this box is checked
    void addDep(KonfiguratorCheckBox *dep);

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, QString, QString);
    void slotSetDefaults(QObject *);

protected:
    virtual void checkStateSet() Q_DECL_OVERRIDE;
    virtual void nextCheckState() Q_DECL_OVERRIDE;
    void updateDeps();

    bool  defaultValue;
    KonfiguratorExtension *ext;
    QList<KonfiguratorCheckBox*> deps;
};

// KonfiguratorSpinBox class
///////////////////////////////

class KonfiguratorSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    KonfiguratorSpinBox(QString configGroup, QString configName, int defaultValue, int min, int max,
                        QWidget *parent = 0, bool restartNeeded = false, int page = FIRST_PAGE);
    ~KonfiguratorSpinBox();

    inline KonfiguratorExtension *extension() { return ext; }

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, QString, QString);
    void slotSetDefaults(QObject *);

protected:
    int  defaultValue;
    KonfiguratorExtension *ext;
};

// KonfiguratorCheckBoxGroup class
///////////////////////////////

class KonfiguratorCheckBoxGroup : public QWidget
{
public:
    explicit KonfiguratorCheckBoxGroup(QWidget *parent = 0) : QWidget(parent){}

    void                    add(KonfiguratorCheckBox *);
    int                     count() {
        return checkBoxList.count();
    };
    KonfiguratorCheckBox *  find(int index);
    KonfiguratorCheckBox *  find(QString name);

private:
    QList<KonfiguratorCheckBox *>  checkBoxList;
};

// KonfiguratorRadioButtons class
///////////////////////////////

class KonfiguratorRadioButtons : public QWidget
{
    Q_OBJECT

public:
    KonfiguratorRadioButtons(QString configGroup, QString name, QString defaultValue,
                             QWidget *parent = 0, bool restart = false, int page = FIRST_PAGE);
    ~KonfiguratorRadioButtons();

    inline KonfiguratorExtension *extension() { return ext; }

    void addRadioButton(QRadioButton *radioWidget, QString name, QString value);

    void selectButton(QString value);

    int count() { return radioButtons.count(); }
    QString selectedValue();
    QRadioButton *find(int index);
    QRadioButton *find(QString name);

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, QString, QString);
    void slotSetDefaults(QObject *);

protected:
    QList<QRadioButton *>  radioButtons;
    QList<QString>   radioValues;
    QList<QString>   radioNames;

    QString         defaultValue;

    KonfiguratorExtension *ext;
};

// KonfiguratorEditBox class
///////////////////////////////

class KonfiguratorEditBox : public QLineEdit
{
    Q_OBJECT

public:
    KonfiguratorEditBox(QString configGroup, QString name, QString defaultValue,
                        QWidget *parent = 0, bool restart = false, int page = FIRST_PAGE);
    ~KonfiguratorEditBox();

    inline KonfiguratorExtension *extension() { return ext; }

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, QString, QString);
    void slotSetDefaults(QObject *);

protected:
    QString  defaultValue;
    KonfiguratorExtension *ext;
};


// KonfiguratorURLRequester class
///////////////////////////////

class KonfiguratorURLRequester : public KUrlRequester
{
    Q_OBJECT

public:
    KonfiguratorURLRequester(QString configGroup, QString name, QString defaultValue,
                             QWidget *parent = 0, bool restart = false, int page = FIRST_PAGE,
                             bool expansion = true);
    ~KonfiguratorURLRequester();

    inline KonfiguratorExtension *extension() { return ext; }

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, QString, QString);
    void slotSetDefaults(QObject *);

protected:
    QString  defaultValue;
    KonfiguratorExtension *ext;
    bool expansion;
};

// KonfiguratorFontChooser class
///////////////////////////////

class KonfiguratorFontChooser : public QWidget
{
    Q_OBJECT

public:
    KonfiguratorFontChooser(QString configGroup, QString name, QFont defaultValue,
                            QWidget *parent = 0, bool restart = false, int page = FIRST_PAGE);
    ~KonfiguratorFontChooser();

    inline KonfiguratorExtension *extension() { return ext; }

public slots:
    virtual void    loadInitialValue();
    void            slotApply(QObject *, QString, QString);
    void            slotSetDefaults(QObject *);
    void            slotBrowseFont();

protected:
    QFont           defaultValue;
    QFont           font;
    KonfiguratorExtension *ext;

    QLabel        * pLabel;
    QToolButton   * pToolButton;

    void            setFont();
};

// KONFIGURATOR_NAME_VALUE_PAIR structure
///////////////////////////////

struct KONFIGURATOR_NAME_VALUE_PAIR {
    QString text;
    QString value;
};

// KONFIGURATOR_NAME_VALUE_TIP structure
///////////////////////////////

struct KONFIGURATOR_NAME_VALUE_TIP {
    QString text;
    QString value;
    QString tooltip;
};

// KonfiguratorComboBox class
///////////////////////////////

class KonfiguratorComboBox : public QComboBox
{
    Q_OBJECT

public:
    KonfiguratorComboBox(QString configGroup, QString name, QString defaultValue,
                         KONFIGURATOR_NAME_VALUE_PAIR *listIn, int listInLen, QWidget *parent = 0,
                         bool restart = false, bool editable = false, int page = FIRST_PAGE);
    ~KonfiguratorComboBox();

    inline KonfiguratorExtension *extension() { return ext; }

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, QString, QString);
    void slotSetDefaults(QObject *);

protected:
    QString                       defaultValue;
    KONFIGURATOR_NAME_VALUE_PAIR *list;
    int                           listLen;
    KonfiguratorExtension        *ext;

    void                          selectEntry(QString entry);
};


// KonfiguratorColorChooser class
///////////////////////////////

typedef struct {
    QString name;
    QColor  color;
    QString value;
} ADDITIONAL_COLOR;

class KonfiguratorColorChooser : public QComboBox
{
    Q_OBJECT

public:
    KonfiguratorColorChooser(QString configGroup, QString name, QColor defaultValue,
                             QWidget *parent = 0, bool restart = false,
                             ADDITIONAL_COLOR *addColPtr = 0, int addColNum = 0, int page = FIRST_PAGE);
    ~KonfiguratorColorChooser();

    inline KonfiguratorExtension *extension()   {
        return ext;
    }

    void          setDefaultColor(QColor dflt);
    void          setDefaultText(QString text);
    QColor        getColor();
    void          changeAdditionalColor(int num, QColor color);
    QString       getValue();
    bool          isValueRGB();
    void          setValue(QString value);

public slots:
    virtual void  loadInitialValue();
    void          slotApply(QObject *, QString, QString);
    void          slotSetDefaults(QObject *);
    void          slotCurrentChanged(int number);

signals:
    void          colorChanged();

private:
    void          addColor(QString text, QColor color);
    QPixmap       createPixmap(QColor color);

protected:
    QColor                          defaultValue;
    QColor                          customValue;
    QList<QColor>                   palette;
    QList<ADDITIONAL_COLOR>         additionalColors;
    KonfiguratorExtension          *ext;
    bool                            disableColorChooser;
};

// KonfiguratorListBox class
///////////////////////////////

class KonfiguratorListBox : public KrListWidget
{
    Q_OBJECT

public:
    KonfiguratorListBox(QString configGroup, QString name, QStringList defaultValue,
                        QWidget *parent = 0, bool restart = false, int page = FIRST_PAGE);
    ~KonfiguratorListBox();

    inline KonfiguratorExtension *extension() { return ext; }

    void addItem(const QString &);
    void removeItem(const QString &);

public slots:
    virtual void loadInitialValue();
    void slotApply(QObject *, QString, QString);
    void slotSetDefaults(QObject *);

protected:
    QStringList list();
    void setList(QStringList);

    QStringList defaultValue;
    KonfiguratorExtension *ext;
};

#endif /* __KONFIGURATOR_ITEMS_H__ */

