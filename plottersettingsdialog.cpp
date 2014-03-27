#include "plottersettingsdialog.h"

#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include "plottersettings.h"

PlotterSettingsDialog::PlotterSettingsDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent),
      m_settings(nullptr)
{
    m_heightSpinBox = new QSpinBox;
    m_heightSpinBox->setMinimum(0);
    m_heightSpinBox->setMaximum(PlotterSettings::MaximumHeight);

    m_widthSpinBox = new QSpinBox;
    m_widthSpinBox->setMinimum(0);
    m_widthSpinBox->setMaximum(PlotterSettings::MaximumWidth);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Height"), m_heightSpinBox);
    formLayout->addRow(tr("Width"), m_widthSpinBox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                       | QDialogButtonBox::Discard
                                                       | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    QPushButton *discardButton = buttonBox->button(QDialogButtonBox::Discard);
    connect(discardButton, SIGNAL(clicked(bool)),
            this, SLOT(loadValues()));

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addLayout(formLayout);
    vlayout->addWidget(buttonBox);
    setLayout(vlayout);
}

void PlotterSettingsDialog::accept()
{
    saveValues();
    QDialog::accept();
}

void PlotterSettingsDialog::loadValues()
{
    auto size = m_settings->imageSize();
    m_heightSpinBox->setValue(size.first);
    m_widthSpinBox->setValue(size.second);
}

void PlotterSettingsDialog::saveValues()
{
    auto h = m_heightSpinBox->value();
    auto w = m_widthSpinBox->value();
    m_settings->setImageSize(qMakePair(h, w));
}

void PlotterSettingsDialog::setPlotterSettings(PlotterSettings *settings)
{
    Q_ASSERT(settings != nullptr);
    Q_ASSERT(m_settings == nullptr);

    m_settings = settings;
    loadValues();
}

#include "plottersettingsdialog.moc"
