#include "plottersettingsdialog.h"

#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFormLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include "plottersettings.h"

PlotterSettingsDialog::PlotterSettingsDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent),
      m_settings(nullptr)
{
    QGroupBox *titlesGroupBox = setupTitlesWidgets();

    m_axisGroupBoxes[0] = setupAxisWidgets('x');
    m_axisGroupBoxes[1] = setupAxisWidgets('y');
    m_axisGroupBoxes[2] = setupAxisWidgets('z');

    QGroupBox *pageNimgDimGroupBox = setupPageNimgDimWidgets();

    m_moreGroupBox = setupMoreWidgets();
    m_moreGroupBox->hide();

    QPushButton *moreButton = new QPushButton;
    moreButton->setText(tr("&More"));
    moreButton->setToolTip(tr("Show more options."));
    moreButton->setCheckable(true);
    moreButton->setAutoDefault(false);
    connect(moreButton, SIGNAL(toggled(bool)),
            m_moreGroupBox, SLOT(setVisible(bool)));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply |
                                                       QDialogButtonBox::Discard |
                                                       QDialogButtonBox::Reset |
                                                       QDialogButtonBox::RestoreDefaults);
    buttonBox->addButton(moreButton, QDialogButtonBox::ActionRole);
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(accept()));
    connect(buttonBox->button(QDialogButtonBox::Discard), SIGNAL(clicked()),
            this, SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            this, SLOT(loadValues()));
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked(bool)),
            this, SLOT(resetValues()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(titlesGroupBox);
    layout->addWidget(m_axisGroupBoxes[0]);
    layout->addWidget(m_axisGroupBoxes[1]);
    layout->addWidget(m_axisGroupBoxes[2]);
    layout->addWidget(pageNimgDimGroupBox);
    layout->addWidget(m_moreGroupBox);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

/**
 * @brief Saves values and closes dialog with Accept-Role.
 */
void PlotterSettingsDialog::accept()
{
    saveValues();
    QDialog::accept();
}

/**
 * @brief Load Values from m_settings.
 */
void PlotterSettingsDialog::loadValues()
{
    // Title Lines:
    for (int i = 0; i < 4; ++i)
        m_titleLines[i]->setText(m_settings->title(i + 1));

    // Axis Labels & Spans:
    int axis = m_settings->axis();
    for (int i = 0; i < 3; ++i) {
        m_axisTitles[i]->setHidden(i >= axis);
        m_axisTitles[i]->setText(m_settings->axisTitle(
                                     static_cast<PlotterSettings::Axis>(i)));
        m_axisGroupBoxes[i]->setHidden(i >= axis);
    }

    m_xSpinBoxes[0]->setValue(m_settings->axisSpan(PlotterSettings::X_Axis).first);
    m_xSpinBoxes[1]->setValue(m_settings->axisSpan(PlotterSettings::X_Axis).second);

    m_ySpinBoxes[0]->setValue(m_settings->axisSpan(PlotterSettings::Y_Axis).first);
    m_ySpinBoxes[1]->setValue(m_settings->axisSpan(PlotterSettings::Y_Axis).second);

    m_zSpinBoxes[0]->setValue(m_settings->axisSpan(PlotterSettings::Z_Axis).first);
    m_zSpinBoxes[1]->setValue(m_settings->axisSpan(PlotterSettings::Z_Axis).second);

    // Page & Image Dimensions:
    m_pageComboBox->blockSignals(true);
    auto p = m_settings->page();
    int pageIndex = m_pageComboBox->findData(p );
    Q_ASSERT(pageIndex != -1);
    m_pageComboBox->setCurrentIndex(pageIndex);
    m_pageComboBox->blockSignals(false);

    auto size = m_settings->imageSize();
    m_heightSpinBox->setValue(size.first);
    m_widthSpinBox->setValue(size.second);
    if ((size.first == PlotterSettings::ImageStandardHeight) &&
            (size.second == PlotterSettings::ImageStandardWidth) &&
            (m_pageComboBox->currentText().endsWith("Portrait"))) {

        swapImageDimension();
    }

    // Additional Options:
    m_fontComboBox->setCurrentIndex(m_settings->fontIndex());
    m_pageBorderCheckBox->setChecked(m_settings->pageBorder());
}

/**
 * @brief Changes which need to be done when changing from Portrait to
 *        Lanscape.
 * @param text current selected Page-Format
 */
void PlotterSettingsDialog::pageChanged(const QString &text)
{
    PlotterSettings::PageType page = m_settings->page();
    int index = m_pageComboBox->findData(page);
    Q_ASSERT(index != -1);
    bool l1 = m_pageComboBox->itemText(index).endsWith("Landscape");
    bool l2 = text.endsWith("Landscape");
    if (l1 != l2)
        swapImageDimension();
}

/**
 * @brief Restores values in m_settings by calling @c delete and @c new.
 */
void PlotterSettingsDialog::resetValues()
{
    ContourPlotterSettings *cps = dynamic_cast<ContourPlotterSettings *>(m_settings);
    if (cps != nullptr) {
        delete m_settings;
        m_settings = nullptr;
        setPlotterSettings(new ContourPlotterSettings);
    }
}

/**
 * @brief Saves all values from Widgtes to m_settings.
 */
void PlotterSettingsDialog::saveValues()
{
    // Title Lines:
    for (int i = 0; i < 4; ++i)
        m_settings->setTitle(m_titleLines[i]->text(), i + 1);

    // Axis Labels & Spans:
    for (int i = 0; i < 3; ++i) {
        if (!m_axisTitles[i]->isHidden()) {
            m_settings->setAxisTitle(m_axisTitles[i]->text(),
                                     static_cast<PlotterSettings::Axis>(i));
        }
    }

    if (!m_axisGroupBoxes[0]->isHidden()) {
        PlotterSettings::Span xSpan = qMakePair(m_xSpinBoxes[0]->value(),
                                                m_xSpinBoxes[1]->value());
        m_settings->setAxisSpan(xSpan, PlotterSettings::X_Axis);
    }

    if (!m_axisGroupBoxes[1]->isHidden()) {
        PlotterSettings::Span ySpan = qMakePair(m_ySpinBoxes[0]->value(),
                                                m_ySpinBoxes[1]->value());
        m_settings->setAxisSpan(ySpan, PlotterSettings::Y_Axis);
    }

    if (!m_axisGroupBoxes[2]->isHidden()) {
        PlotterSettings::Span zSpan = qMakePair(m_zSpinBoxes[0]->value(),
                                                m_zSpinBoxes[1]->value());
        m_settings->setAxisSpan(zSpan, PlotterSettings::Z_Axis);
    }

    // Page & Image Dimensions:
    int page = m_pageComboBox->itemData(m_pageComboBox->currentIndex()).toInt();
    m_settings->setPage(static_cast<PlotterSettings::PageType>(page));

    auto h = m_heightSpinBox->value();
    auto w = m_widthSpinBox->value();
    m_settings->setImageSize(qMakePair(h, w));

    // Additional Options:
    m_settings->setFontIndex(m_fontComboBox->currentIndex());
    m_settings->setPageBorder(m_pageBorderCheckBox->isChecked());
}

/**
 * @brief Set and load values from @a settings.
 * @param settings
 */
void PlotterSettingsDialog::setPlotterSettings(PlotterSettings *settings)
{
    Q_ASSERT(settings != nullptr);
    Q_ASSERT(m_settings == nullptr);

    m_settings = settings;
    loadValues();
}

QGroupBox *PlotterSettingsDialog::setupAxisWidgets(char axis)
{
    QGroupBox *ret;

    int i;
    QDoubleSpinBox **spinBoxes = nullptr;
    QChar a = QChar(axis).toUpper();

    if (a == 'X') {
        i = 0;
        spinBoxes = &m_xSpinBoxes[0];
    } else if (a == 'Y') {
        i = 1;
        spinBoxes = &m_ySpinBoxes[0];
    } else if (a == 'Z') {
        i = 2;
        spinBoxes = &m_zSpinBoxes[0];
    }
    Q_ASSERT(spinBoxes != nullptr);

    m_axisTitles[i] = new QLineEdit;
    m_axisTitles[i]->setMaxLength(255); // TODO: correct max title length
    m_axisTitles[i]->setToolTip(tr("Label printed next to '%1' axis.").arg(a));

    spinBoxes[0] = new QDoubleSpinBox;
    spinBoxes[0]->setMinimum(-10.0);
    spinBoxes[0]->setMaximum(10.0);
    spinBoxes[0]->setToolTip(tr("Lower axis boundary"));

    spinBoxes[1] = new QDoubleSpinBox;
    spinBoxes[1]->setMinimum(-10.0);
    spinBoxes[1]->setMaximum(10.0);
    spinBoxes[1]->setToolTip(tr("Upper axis boundary"));

    QHBoxLayout *spanLayout = new QHBoxLayout;
    spanLayout->addWidget(spinBoxes[0]);
    spanLayout->addWidget(new QLabel(" - "));
    spanLayout->addWidget(spinBoxes[1]);
    spanLayout->setMargin(0);

    QWidget *spanWidget = new QWidget;
    spanWidget->setLayout(spanLayout);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Name:"), m_axisTitles[i]);
    formLayout->addRow(tr("Span:"), spanWidget);

    ret = new QGroupBox;
    ret->setTitle(tr("%1 Axis").arg(a));
    ret->setLayout(formLayout);

    return ret;
}

QGroupBox *PlotterSettingsDialog::setupMoreWidgets()
{
    QGroupBox *ret;

    m_fontComboBox = new QComboBox;
    m_fontComboBox->addItems(PlotterSettings::fonts());
    m_fontComboBox->setToolTip(tr("Available Postscript Fonts"));

    m_pageBorderCheckBox = new QCheckBox;
    m_pageBorderCheckBox->setText(tr("Enable"));
    m_pageBorderCheckBox->setToolTip(tr("Plots a border around the page."));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("&Font:"), m_fontComboBox);
    formLayout->addRow(tr("Page Border:"), m_pageBorderCheckBox);

    ret = new QGroupBox;
    ret->setTitle(tr("Additional Options"));
    ret->setLayout(formLayout);

    return ret;
}

QGroupBox *PlotterSettingsDialog::setupPageNimgDimWidgets()
{
    QGroupBox *ret;

    m_heightSpinBox = new QSpinBox;
    m_heightSpinBox->setToolTip(tr("Height"));
    m_heightSpinBox->setMinimum(0);
    m_heightSpinBox->setMaximum(PlotterSettings::ImageMaximumHeight);

    m_widthSpinBox = new QSpinBox;
    m_widthSpinBox->setToolTip(tr("Width"));
    m_widthSpinBox->setMinimum(0);
    m_widthSpinBox->setMaximum(PlotterSettings::ImageMaximumWidth);

    QPushButton *imageDimSwapButton = new QPushButton;
    imageDimSwapButton->setIcon(QIcon::fromTheme("view-refresh"));
    imageDimSwapButton->setToolTip(tr("Swap"));
    if (imageDimSwapButton->icon().isNull())
        imageDimSwapButton->setText(imageDimSwapButton->toolTip());
    connect(imageDimSwapButton, SIGNAL(clicked(bool)),
            this, SLOT(swapImageDimension()));

    QHBoxLayout *imageDimLayout = new QHBoxLayout;
    imageDimLayout->addWidget(m_heightSpinBox);
    imageDimLayout->addWidget(new QLabel(" x "));
    imageDimLayout->addWidget(m_widthSpinBox);
    imageDimLayout->addWidget(imageDimSwapButton);
    imageDimLayout->setMargin(0);

    QWidget *imageDimWidget = new QWidget;
    imageDimWidget->setLayout(imageDimLayout);

    m_pageComboBox = new QComboBox;
    m_pageComboBox->addItem(tr("DIN A4 Landscape"), PlotterSettings::Page_DIN_A4_Landscape);
    m_pageComboBox->addItem(tr("DIN A4 Portrait"), PlotterSettings::Page_DIN_A4_Portrait);
    connect(m_pageComboBox, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(pageChanged(QString)));

    m_fontComboBox = new QComboBox;
    m_fontComboBox->addItems(PlotterSettings::fonts());

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("&Page:"), m_pageComboBox);
    formLayout->addRow(tr("&Image Dimensions:"), imageDimWidget);

    ret = new QGroupBox;
    ret->setTitle(tr("Page & Image Dimensions"));
    ret->setLayout(formLayout);

    return ret;
}

QGroupBox *PlotterSettingsDialog::setupTitlesWidgets()
{
    QGroupBox *ret;

    QFormLayout *titlesLayout = new QFormLayout;
    for (int i = 0; i < 4; ++i) {
        m_titleLines[i] = new QLineEdit;
        m_titleLines[i]->setMaxLength(255);
        m_titleLines[i]->setToolTip(tr("Text printed as title in line %1.").arg(i+1));
        titlesLayout->addRow(tr("Line &%1:").arg(i+1), m_titleLines[i]);
    }

    ret = new QGroupBox;
    ret->setTitle(tr("Titles"));
    ret->setLayout(titlesLayout);

    return ret;
}

void PlotterSettingsDialog::swapImageDimension()
{
    int hMax = m_heightSpinBox->maximum();
    int h = m_heightSpinBox->value();
    int wMax = m_widthSpinBox->maximum();
    int w = m_widthSpinBox->value();

    m_heightSpinBox->setMaximum(wMax);
    m_heightSpinBox->setValue(w);
    m_widthSpinBox->setMaximum(hMax);
    m_widthSpinBox->setValue(h);
}

#include "plottersettingsdialog.moc"
