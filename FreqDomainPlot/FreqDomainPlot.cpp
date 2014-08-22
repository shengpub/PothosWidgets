// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "FreqDomainPlot.hpp"
#include <QResizeEvent>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <QHBoxLayout>

FreqDomainPlot::FreqDomainPlot(const Pothos::DType &dtype):
    _mainPlot(new QwtPlot(this)),
    _plotGrid(new QwtPlotGrid()),
    _displayRate(1.0),
    _sampleRate(1.0),
    _sampleRateWoAxisUnits(1.0),
    _numBins(1024)
{
    //setup block
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, widget));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, setNumInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, setTitle));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, setDisplayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, setSampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, setNumFFTBins));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, numInputs));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, title));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, displayRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, sampleRate));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, numFFTBins));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, enableXAxis));
    this->registerCall(this, POTHOS_FCN_TUPLE(FreqDomainPlot, enableYAxis));
    this->setupInput(0, dtype);

    //layout
    auto layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(QMargins());
    layout->addWidget(_mainPlot);

    //setup plotter
    {
        //missing from qwt:
        qRegisterMetaType<QList<QwtLegendData>>("QList<QwtLegendData>");
        _mainPlot->setCanvasBackground(QBrush(QColor("white")));
        _mainPlot->setAxisScale(QwtPlot::yLeft, -100, 0);
    }

    //setup grid
    {
        _plotGrid->attach(_mainPlot);
        _plotGrid->setPen(QColor("#999999"), 0.5, Qt::DashLine);
    }
}

FreqDomainPlot::~FreqDomainPlot(void)
{
    return;
}

void FreqDomainPlot::setNumInputs(const size_t numInputs)
{
    for (size_t i = this->inputs().size(); i < numInputs; i++)
    {
        this->setupInput(i, this->input(0)->dtype());
    }
}

void FreqDomainPlot::setTitle(const QString &title)
{
    _mainPlot->setTitle(title);
}

void FreqDomainPlot::setDisplayRate(const double displayRate)
{
    _displayRate = displayRate;
}

void FreqDomainPlot::setSampleRate(const double sampleRate)
{
    _sampleRate = sampleRate;
    QString axisTitle("Hz");
    _sampleRateWoAxisUnits = sampleRate;
    if (sampleRate >= 2e9)
    {
        _sampleRateWoAxisUnits /= 1e9;
        axisTitle = "GHz";
    }
    else if (sampleRate >= 2e6)
    {
        _sampleRateWoAxisUnits /= 1e6;
        axisTitle = "MHz";
    }
    else if (sampleRate >= 2e3)
    {
        _sampleRateWoAxisUnits /= 1e3;
        axisTitle = "kHz";
    }
    _mainPlot->setAxisTitle(QwtPlot::xBottom, axisTitle);
    _mainPlot->setAxisScale(QwtPlot::xBottom, -_sampleRateWoAxisUnits/2, +_sampleRateWoAxisUnits/2);
}

void FreqDomainPlot::setNumFFTBins(const size_t numBins)
{
    _numBins = numBins;
    for (auto inPort : this->inputs()) inPort->setReserve(_numBins);
}

QString FreqDomainPlot::title(void) const
{
    return _mainPlot->title().text();
}

void FreqDomainPlot::enableXAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::xBottom, enb);
}

void FreqDomainPlot::enableYAxis(const bool enb)
{
    _mainPlot->enableAxis(QwtPlot::yLeft, enb);
}

void FreqDomainPlot::installLegend(void)
{
    auto legend = new QwtLegend(_mainPlot);
    legend->setDefaultItemMode(QwtLegendData::Checkable);
    connect(legend, SIGNAL(checked(const QVariant &, bool, int)), this, SLOT(handleLegendChecked(const QVariant &, bool, int)));
    _mainPlot->insertLegend(legend);
}

void FreqDomainPlot::handleLegendChecked(const QVariant &itemInfo, bool on, int)
{
    _mainPlot->infoToItem(itemInfo)->setVisible(not on);
}

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerFreqDomainPlot(
    "/widgets/freq_domain_plot", &FreqDomainPlot::make);
