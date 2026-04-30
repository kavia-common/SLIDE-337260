/*
 * CellSPM_test.cpp
 *
 *  Created on: 8 Feb 2020
 *   Author(s): Jorn Reniers
 */

#include "../tests_util.hpp"
#include "../../src/slide.hpp"

#include <cassert>
#include <iostream>
#include <fstream>
#include <cmath>
#include <span>

namespace slide::tests::unit {

bool test_constructor_SPM()
{
  Cell_SPM c1;
  assert(NEAR(c1.Cap(), 16));
  assert(NEAR(c1.Vmin(), 2.7));
  assert(NEAR(c1.Vmax(), 4.2));
  assert(NEAR(c1.I(), 0));
  assert(NEAR(c1.SOC(), 0.5));
  assert(NEAR(c1.T(), settings::T_ENV));

  return true;
}

bool test_getStates_SPM()
{
  //!< bool getStates(double s[], int nin, int&nout);
  Cell_SPM c1;
  const double tol = 0.001;
  const double tol2 = tol / 100.0;

  auto s = c1.getStateObj();

  //!< 0 to nch = zp
  //!< nch to 2*nch = zn
  assert(NEAR(s.zp(0), 0, tol2));
  assert(NEAR(s.zp(1), 0, tol2));
  assert(NEAR(s.zp(2), 0, tol2));
  assert(NEAR(s.zp(3), 0.47605127273, tol2));
  assert(NEAR(s.zp(4), 0, tol2));

  assert(NEAR(s.zn(0), 0, tol2));
  assert(NEAR(s.zn(1), 0, tol2));
  assert(NEAR(s.zn(2), 0, tol2));
  assert(NEAR(s.zn(3), 0.289437188135, tol2));
  assert(NEAR(s.zn(4), 0, tol2));

  assert(EQ(s.delta(), 1e-9));
  assert(EQ(s.LLI(), 0));
  assert(EQ(s.thickp(), 70 * 1e-6));
  assert(EQ(s.thickn(), 73.5 * 1e-6));
  assert(EQ(s.ep(), 0.5));
  assert(EQ(s.en(), 0.5));
  assert(EQ(s.ap(), 1.5 / (8.5 * 1e-6)));
  assert(EQ(s.an(), 1.5 / (1.25 * 1e-5)));
  assert(NEAR(s.CS(), 0.01 * 1.5 / (1.25 * 1e-5) * 0.62 * 73.5 * 1e-6, tol));
  assert(EQ(s.Dp(), 8 * 1e-14));
  assert(EQ(s.Dn(), 7 * 1e-14));

  assert(EQ(s.rDCp(), 0.0028));
  assert(EQ(s.rDCn(), 0.0028));
  assert(EQ(s.rDCcc(), 0.0002325));

  assert(EQ(s.delta_pl(), 0.0));
  assert(EQ(s.SOC(), 0.5));
  assert(EQ(s.T(), settings::T_ENV));
  assert(EQ(s.I(), 0.0));

  assert(NEAR(c1.SOC(), 0.5));
  assert(NEAR(c1.I(), 0));
  assert(NEAR(c1.T(), settings::T_ENV));


  double cps{}, cns{};
  c1.getCSurf(cps, cns, false);

  assert(NEAR(cps, 35421.3, 0.1)); //!< allow slightly larger error since we approximated all elements of the initial array and matrix
  assert(NEAR(cns, 14644.5, 0.1)); //!< concentration ~ 10,000 so 0.1 is still a relative error of e-5
  assert(NEAR(c1.getRdc(), 0.001253, tol));

  return true;
}

bool test_getV_SPM()
{
  Cell_SPM c1;
  double tol = 0.01;

  //!< normal cell, should give no errors
  double Vini = 3.68136;
  assert(NEAR(c1.V(), Vini, tol));
  assert(NEAR(c1.V(), Vini, tol));
  assert(NEAR(c1.V(), Vini, tol));

  //!< set to charging and check the voltage has increased
  c1.setCurrent(-1);
  double V = c1.V();
  assert(V > Vini);

  //!< set to discharge
  V = c1.V();
  c1.setCurrent(1);
  assert(c1.V() < V);
  V = c1.V();

  return true;
}

bool test_setStates_SPM()
{
  Cell_SPM c1;

  //!< set valid new states
  double zp[settings::nch], zn[settings::nch];
  double T, delta, LLI, thickp, thickn, ep, en, ap, an, CS, Dp, Dn, rp, rn, rcc, delta_pl, SOC, I;
  T = 45_degC;
  delta = 2e-9;
  LLI = 1;
  thickp = 60e-6;
  thickn = 70e-6;
  ep = 0.4;
  en = 0.4;
  ap = 3 * ep / 8.5e-6;
  an = 3 * en / 1.25e-5;
  CS = 0.1;
  Dp = 5e-14;
  Dn = 6e-14;
  rp = 0.005;
  rn = 0.005;
  rcc = 0.002;
  delta_pl = 0.01;
  SOC = 0.4;
  I = -1;

  auto &st = c1.getStateObj();

  for (int i = 0; i < State_SPM::nch; i++) {
    zp[i] = st.zp(i);
    zn[i] = st.zn(i);
  }

  std::vector<double> sini(st.size()); // Also includes cumulative.

  for (int i = 0; i < State_SPM::nch; i++) {
    sini[i + State_SPM::i_zp] = zp[i];
    sini[i + State_SPM::i_zn] = zn[i];
  }

  sini[State_SPM::i_delta] = delta;
  sini[State_SPM::i_LLI] = LLI;
  sini[State_SPM::i_thickp] = thickp;
  sini[State_SPM::i_thickn] = thickn;
  sini[State_SPM::i_ep] = ep;
  sini[State_SPM::i_en] = en;
  sini[State_SPM::i_ap] = ap;
  sini[State_SPM::i_an] = an;
  sini[State_SPM::i_CS] = CS;
  sini[State_SPM::i_Dp] = Dp;
  sini[State_SPM::i_Dn] = Dn;
  sini[State_SPM::i_rDCp] = rp;
  sini[State_SPM::i_rDCn] = rn;
  sini[State_SPM::i_rDCcc] = rcc;
  sini[State_SPM::i_delta_pl] = delta_pl;
  sini[State_SPM::i_SOC] = SOC;
  sini[State_SPM::i_T] = T;
  sini[State_SPM::i_I] = I;


  std::span<double> spn(sini);
  c1.setStates(spn, true, true); //!< this checks states are valid

  assert(NEAR(st.SOC(), SOC));
  assert(NEAR(st.I(), I));
  assert(NEAR(st.T(), T));
  assert(NEAR(st.delta(), delta));
  assert(NEAR(st.CS(), CS));
  assert(NEAR(st.delta_pl(), delta_pl));
  assert(NEAR(st.LLI(), LLI));
  assert(NEAR(st.thickp(), thickp));
  assert(NEAR(st.thickn(), thickn));
  assert(NEAR(st.ep(), ep));
  assert(NEAR(st.en(), en));
  assert(NEAR(st.ap(), ap));
  assert(NEAR(st.an(), an));
  assert(NEAR(st.Dp(), Dp));
  assert(NEAR(st.Dn(), Dn));
  assert(NEAR(st.rDCp(), rp));
  assert(NEAR(st.rDCn(), rn));
  assert(NEAR(st.rDCcc(), rcc));


  // //!< set invalid states
  // //!< test with Ap != 3*e/R
  // ap = 3 * ep / 8.5 * 1e-6;
  // sini[State_SPM::i_ap] = ap;
  // auto status = c1.setStates(spn, true, true); //!< this checks states are valid


  // if (isStatusSuccessful(status)) return false; // Failed the test if it does not throw!

  // sini[State_SPM::i_an] = 3 * ep / 8.5e-6;

  // //!< test with negative uniform concentration
  // zp[3] = -zp[3];
  // for (int i = 0; i < settings::nch; i++)
  //   sini[i] = zp[i];


  // status = c1.setStates(spn, true, true);       //!< this checks states are valid
  // if (isStatusSuccessful(status)) return false; // Failed the test if it does not throw!


  return true;
}

bool test_timeStep_CC_SPM()
{
  //!< bool timeStep_CC(double dt);
  Cell_SPM c1;
  double soc = c1.SOC();

  //!< set to charging and check the voltage has increased
  c1.setCurrent(-1);
  double V = c1.V();
  c1.timeStep_CC(5);
  assert(c1.V() > V);
  assert(c1.SOC() > soc);

  //!< set to discharge
  V = c1.V();
  soc = c1.SOC();
  c1.setCurrent(1);
  assert(c1.V() < V);
  V = c1.V();
  c1.timeStep_CC(5);
  assert(c1.V() < V);
  assert(c1.SOC() < soc);

  return true;
}

bool test_spme_is_opt_in()
{
  Cell_SPM c1;
  const auto base_voltage = c1.V();

  assert(!c1.isElectrolyteGradientModelEnabled());

  const auto ce = c1.getElectrolyteConcentrationProfile();
  for (const auto ce_i : ce)
    assert(NEAR(ce_i, 1000.0));

  c1.enableElectrolyteGradientModel(true);
  assert(c1.isElectrolyteGradientModelEnabled());
  assert(NEAR(c1.V(), base_voltage, 1e-6));

  return true;
}

bool test_spme_parameter_bundle_initialises_electrolyte_state()
{
  Cell_SPM c1;

  slide::param::SPMeParam params = c1.getSPMeParameters();
  params.enabled = true;
  params.c_elec = 1234.5;
  params.c_elec_init = 1234.5;
  params.ce_deviation_init = { -10.0, 0.0, 20.0 };
  params.electrolyte_diffusion = 3.1e-10;
  params.coupling = 4.0;

  c1.setSPMeParameters(params);

  assert(c1.isElectrolyteGradientModelEnabled());

  const auto stored = c1.getSPMeParameters();
  assert(EQ(stored.enabled, params.enabled));
  assert(NEAR(stored.c_elec, params.c_elec));
  assert(NEAR(stored.electrolyte_diffusion, params.electrolyte_diffusion));
  assert(NEAR(stored.coupling, params.coupling));

  const auto ce = c1.getElectrolyteConcentrationProfile();
  const auto ce_dev = c1.getElectrolyteConcentrationDeviationProfile();
  for (size_t i = 0; i < ce.size(); i++) {
    assert(NEAR(ce_dev[i], params.ce_deviation_init[i]));
    assert(NEAR(ce[i], params.c_elec + params.ce_deviation_init[i]));
  }

  const auto diagnostics = c1.getElectrolyteDiagnosticVoltages();
  assert(NEAR(diagnostics[0], 30.0));
  assert(NEAR(diagnostics[1], 10.0 / 3.0));
  assert(NEAR(diagnostics[2], (10.0 / 3.0) / params.c_elec));

  return true;
}

bool test_spme_disabled_step_keeps_electrolyte_profile_constant()
{
  Cell_SPM c1;
  c1.setCurrent(8.0);

  const auto voltage_before = c1.V();
  const auto ce_before = c1.getElectrolyteConcentrationProfile();
  const auto ce_dev_before = c1.getElectrolyteConcentrationDeviationProfile();
  const auto diagnostics_before = c1.getElectrolyteDiagnosticVoltages();

  c1.timeStep_CC(1.0, 5);

  const auto ce_after = c1.getElectrolyteConcentrationProfile();
  const auto ce_dev_after = c1.getElectrolyteConcentrationDeviationProfile();
  const auto diagnostics_after = c1.getElectrolyteDiagnosticVoltages();

  for (size_t i = 0; i < ce_before.size(); i++) {
    assert(NEAR(ce_before[i], ce_after[i], 1e-12));
    assert(NEAR(ce_dev_before[i], ce_dev_after[i], 1e-12));
    assert(NEAR(diagnostics_before[i], diagnostics_after[i], 1e-12));
  }

  assert(!c1.isElectrolyteGradientModelEnabled());
  assert(NEAR(voltage_before, 3.68136, 0.05));

  return true;
}

bool test_spme_updates_electrolyte_profile_when_enabled()
{
  Cell_SPM c1;
  c1.enableElectrolyteGradientModel(true);
  c1.setElectrolyteGradientParameters(2.8e-10, 5.0);
  c1.setCurrent(5.0);

  const auto ce_before = c1.getElectrolyteConcentrationProfile();
  c1.timeStep_CC(1.0);
  const auto ce_after = c1.getElectrolyteConcentrationProfile();

  bool changed = false;
  for (size_t i = 0; i < ce_before.size(); i++)
    changed = changed || !NEAR(ce_before[i], ce_after[i], 1e-12);

  assert(changed);

  return true;
}

bool test_spme_enabled_step_updates_diagnostics()
{
  Cell_SPM c1;
  c1.enableElectrolyteGradientModel(true);
  c1.setElectrolyteGradientParameters(2.8e-10, 8.0);
  c1.setCurrent(10.0);

  c1.timeStep_CC(1.0, 6);

  const auto ce = c1.getElectrolyteConcentrationProfile();
  const auto ce_dev = c1.getElectrolyteConcentrationDeviationProfile();
  const auto diagnostics = c1.getElectrolyteDiagnosticVoltages();

  bool has_nonzero_deviation = false;
  for (size_t i = 0; i < ce.size(); i++) {
    has_nonzero_deviation = has_nonzero_deviation || !NEAR(ce_dev[i], 0.0, 1e-12);
    assert(NEAR(ce[i], 1000.0 + ce_dev[i], 1e-9));
  }

  assert(has_nonzero_deviation);
  assert(diagnostics[0] > 0.0);
  assert(std::abs(diagnostics[1]) > 0.0);
  assert(std::abs(diagnostics[2]) > 0.0);

  return true;
}

bool test_spme_high_rate_diverges_from_baseline_spm()
{
  Cell_SPM baseline;
  Cell_SPM spme;

  spme.enableElectrolyteGradientModel(true);
  spme.setElectrolyteGradientParameters(2.8e-10, 12.0);

  baseline.setCurrent(16.0);
  spme.setCurrent(16.0);

  baseline.timeStep_CC(1.0, 10);
  spme.timeStep_CC(1.0, 10);

  const auto baseline_ce = baseline.getElectrolyteConcentrationDeviationProfile();
  const auto spme_ce = spme.getElectrolyteConcentrationDeviationProfile();
  const auto spme_diag = spme.getElectrolyteDiagnosticVoltages();

  for (const auto value : baseline_ce)
    assert(NEAR(value, 0.0, 1e-12));

  bool any_spme_change = false;
  for (const auto value : spme_ce)
    any_spme_change = any_spme_change || !NEAR(value, 0.0, 1e-12);

  assert(any_spme_change);
  assert(spme_diag[0] > 0.0);
  assert(!NEAR(spme.V(), baseline.V(), 1e-8));
  assert(NEAR(baseline.SOC(), spme.SOC(), 1e-12));

  return true;
}

int test_all_Cell_SPM()
{
  //!< calls all test-functions
  if (!TEST(test_constructor_SPM, "test_constructor_SPM")) return 1;
  if (!TEST(test_getStates_SPM, "test_getStates_SPM")) return 2;
  if (!TEST(test_getV_SPM, "test_getV_SPM")) return 3;
  if (!TEST(test_setStates_SPM, "test_setStates_SPM")) return 4;
  if (!TEST(test_timeStep_CC_SPM, "test_timeStep_CC_SPM")) return 5;
  if (!TEST(test_spme_is_opt_in, "test_spme_is_opt_in")) return 6;
  if (!TEST(test_spme_parameter_bundle_initialises_electrolyte_state, "test_spme_parameter_bundle_initialises_electrolyte_state")) return 7;
  if (!TEST(test_spme_disabled_step_keeps_electrolyte_profile_constant, "test_spme_disabled_step_keeps_electrolyte_profile_constant")) return 8;
  if (!TEST(test_spme_updates_electrolyte_profile_when_enabled, "test_spme_updates_electrolyte_profile_when_enabled")) return 9;
  if (!TEST(test_spme_enabled_step_updates_diagnostics, "test_spme_enabled_step_updates_diagnostics")) return 10;
  if (!TEST(test_spme_high_rate_diverges_from_baseline_spm, "test_spme_high_rate_diverges_from_baseline_spm")) return 11;

  return 0;
}
} // namespace slide::tests::unit

int main() { return slide::tests::unit::test_all_Cell_SPM(); }