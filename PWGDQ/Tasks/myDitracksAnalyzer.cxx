// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
///
/// \brief This task is an empty skeleton that fills a simple eta histogram.
///        it is meant to be a blank page for further developments.
/// \author everyone

#include <Framework/ASoA.h>
#include <fairlogger/Logger.h>
#include "Framework/runDataProcessing.h"
#include "PWGDQ/DataModel/ReducedInfoTables.h"
#include "Framework/AnalysisTask.h"

using namespace o2;
using namespace o2::framework;

using myDitracks = soa::Join<aod::Ditracks, aod::DitracksExtra>;

struct myDitracksAnalyzer {
  Configurable<float> fConfigLowMass{"cfgLowMass", 1.80, "Ditrack lower mass cut"};
  Configurable<float> fConfigHighMass{"cfgHighMass", 1.90, "Ditrack upper mass cut"};
  // Histogram registry: an object to hold your histograms
  HistogramRegistry histos{"histos", {}, OutputObjHandlingPolicy::AnalysisObject};
  // Map to track how many times an event has been encountered
  std::map<int32_t, int8_t> fEventCount;

  void init(InitContext const&)
  {
    // define axes you want to use
    const AxisSpec axisMult{300, 0., 300., "Multiplicity"};

    // create histograms
    histos.add("VtxNContribReal", "VtxNContribReal", kTH1F, {axisMult});
    histos.add("MultFT0A", "MultFT0A", kTH1F, {axisMult});
    histos.add("MultFT0C", "MultFT0C", kTH1F, {axisMult});
    histos.add("MultFV0A", "MultFV0A", kTH1F, {axisMult});
  }

  void process(myDitracks::iterator const& ditrack)
  {
    // Apply cuts
    if (ditrack.mass() < fConfigLowMass.value || ditrack.mass() > fConfigHighMass.value) {
      return;
    }

    if (fEventCount.find(ditrack.reducedeventId()) != fEventCount.end()) {
      LOGF(info, "!!! This event (%d) has been encountered %d times before", ditrack.reducedeventId(), fEventCount[ditrack.reducedeventId()]);
      fEventCount[ditrack.reducedeventId()] += 1;
    } else {
      // First time this event is encountered, fill event-level histograms
      fEventCount.insert({ditrack.reducedeventId(), 1});

      histos.get<TH1>(HIST("VtxNContribReal"))->Fill(ditrack.multNTracksPV());
      histos.get<TH1>(HIST("MultFT0A"))->Fill(ditrack.multFT0A());
      histos.get<TH1>(HIST("MultFT0C"))->Fill(ditrack.multFT0C());
      histos.get<TH1>(HIST("MultFV0A"))->Fill(ditrack.multFV0A());
    }
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  return WorkflowSpec{
    adaptAnalysisTask<myDitracksAnalyzer>(cfgc)};
}
