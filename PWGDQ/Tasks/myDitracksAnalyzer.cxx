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
#include <Framework/HistogramSpec.h>
#include <fairlogger/Logger.h>
#include "Framework/runDataProcessing.h"
#include "PWGDQ/DataModel/ReducedInfoTables.h"
#include "Framework/AnalysisTask.h"

using namespace o2;
using namespace o2::framework;

using myDitracks = soa::Join<aod::Ditracks, aod::DitracksExtra>;

 OutputObj<TH1F> massHisto{TH1F("MassOutputObj", "MassOutputObj", 500, 0., 5.),
                       OutputObjHandlingPolicy::AnalysisObject};

struct myDitracksAnalyzer {
  Configurable<float> fConfigLowMass{"cfgLowMass", 0., "Ditrack lower mass cut"};
  Configurable<float> fConfigHighMass{"cfgHighMass", 9999., "Ditrack upper mass cut"};
  Configurable<int> fConfigPairFilterBit{"cfgPairFilterBit", 0, "Which bit from the PairFilterMap to use for selection"};
  Configurable<int> fConfigPairSign{"cfgPairSign", 0, "Ditrack sum of signs"};
  // Histogram registry: an object to hold your histograms
  HistogramRegistry histos{"histos", {}, OutputObjHandlingPolicy::AnalysisObject};
  // Map to track how many times an event has been encountered
  std::map<int32_t, int8_t> fEventCount;

  void init(InitContext const&)
  {
    // define axes you want to use
    const AxisSpec axisMult{700, 0., 700., "Multiplicity"};
    const AxisSpec axisMultLow{100, 0, 100, "Multiplicity"};
    const AxisSpec axisMass{500, 0., 5., "Mass"};
    const AxisSpec axisMassD0region{140, 1.5, 2.2, "MassD0region"};
    const AxisSpec axisPt{2000, 0.0, 20., "Pt"};

    // create histograms
    histos.add("Mass_BeforeCuts", "Mass_BeforeCuts", kTH1F, {axisMass});
    histos.add("MassD0region_BeforeCuts", "MassD0region_BeforeCuts", kTH1F, {axisMassD0region});
    histos.add("Pt_BeforeCuts", "Pt_BeforeCuts", kTH1F, {axisPt});

    histos.add("Mass", "Mass", kTH1F, {axisMass});
    histos.add("MassD0region", "MassD0region", kTH1F, {axisMassD0region});
    histos.add("Pt", "Pt", kTH1F, {axisPt});
    histos.add("VtxNContribReal", "VtxNContribReal", kTH1F, {axisMult});
    histos.add("MultFT0A", "MultFT0A", kTH1F, {axisMult});
    histos.add("MultFT0C", "MultFT0C", kTH1F, {axisMult});
    histos.add("MultFV0A", "MultFV0A", kTH1F, {axisMult});
    histos.add("ND0Cand", "ND0Cand", kTH1I, {axisMultLow});
  }

  void process(myDitracks const& ditracks)
  {
    LOGF(info, "ditracks has %d entries", ditracks.size());
    fEventCount.clear();
    for (auto& ditrack : ditracks) {
      // Only process pairs with correct charge
      if (ditrack.sign() != fConfigPairSign.value) {
        return;
      }

      // Fill histograms before cuts
      histos.get<TH1>(HIST("Mass_BeforeCuts"))->Fill(ditrack.mass());
      histos.get<TH1>(HIST("MassD0region_BeforeCuts"))->Fill(ditrack.mass());
      histos.get<TH1>(HIST("Pt_BeforeCuts"))->Fill(ditrack.pt());

      // Apply cuts
      if (!ditrack.pairFilterMap_bit(fConfigPairFilterBit)) {
        return;
      }
      if (ditrack.mass() < fConfigLowMass.value || ditrack.mass() >= fConfigHighMass.value) {
        return;
      }

      // Fill pair-level histograms after cuts
      histos.get<TH1>(HIST("Mass"))->Fill(ditrack.mass());
      massHisto->Fill(ditrack.mass());
      histos.get<TH1>(HIST("MassD0region"))->Fill(ditrack.mass());
      histos.get<TH1>(HIST("Pt"))->Fill(ditrack.pt());

      if (fEventCount.find(ditrack.reducedeventId()) != fEventCount.end()) {
        LOGF(info, "!!! This event (%d) has been encountered %d times before", ditrack.reducedeventId(), fEventCount[ditrack.reducedeventId()]);
        // Remove one count from the old number
        int oldBin = histos.get<TH1>(HIST("ND0Cand"))->FindBin(fEventCount[ditrack.reducedeventId()]);
        histos.get<TH1>(HIST("ND0Cand"))->SetBinContent(oldBin, histos.get<TH1>(HIST("ND0Cand"))->GetBinContent(oldBin) - 1);
        // Update counter and histogram
        fEventCount[ditrack.reducedeventId()] += 1;
        histos.get<TH1>(HIST("ND0Cand"))->Fill(fEventCount[ditrack.reducedeventId()]);
      } else {
        // First time this event is encountered, fill event-level histograms
        fEventCount.insert({ditrack.reducedeventId(), 1});
        histos.get<TH1>(HIST("ND0Cand"))->Fill(1);

        histos.get<TH1>(HIST("VtxNContribReal"))->Fill(ditrack.multNTracksPV());
        histos.get<TH1>(HIST("MultFT0A"))->Fill(ditrack.multFT0A());
        histos.get<TH1>(HIST("MultFT0C"))->Fill(ditrack.multFT0C());
        histos.get<TH1>(HIST("MultFV0A"))->Fill(ditrack.multFV0A());
      }
    }
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  return WorkflowSpec{
    adaptAnalysisTask<myDitracksAnalyzer>(cfgc)};
}
