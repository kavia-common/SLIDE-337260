/*
 * cell_tests.hpp
 *
 * Example cell test functions;
 *
 * Created on: 04 Apr 2022
 * Author(s): Volkan Kumtepeli
 */

#pragma once

#include "../src/slide.hpp"

#include <string>
#include <memory>
#include <fstream>
#include <span>

namespace slide::examples {

inline void write_spme_trace_row(std::ofstream &out, Cell_SPM &cell, double t_now)
{
  const auto ce = cell.getElectrolyteConcentrationProfile();
  const auto ce_dev = cell.getElectrolyteConcentrationDeviationProfile();
  const auto diagnostics = cell.getElectrolyteDiagnosticVoltages();

  out << t_now << ',' << cell.I() << ',' << cell.V() << ',' << cell.SOC() << ',' << cell.T()
      << ',' << (cell.isElectrolyteGradientModelEnabled() ? 1 : 0);

  for (const auto value : ce)
    out << ',' << value;
  for (const auto value : ce_dev)
    out << ',' << value;
  for (const auto value : diagnostics)
    out << ',' << value;

  out << '\n';
}

inline auto GITT_test(bool enable_spme = false)
{
  // Note: Entropic effect must be added!
  std::string ID = enable_spme ? "temp_spme" : "temp";
  Clock clk;

  // double Tref = 21.0_degC; // Temperature at which the characterisation should be done [K]
  // Our data is between 23.5 and 25.9 with mean 24.2 C temperature. 26.44 for test data.
  if (settings::T_MODEL != 0) {
    std::cerr << "GITT_test example works with T_MODEL=0 but it is not!\n";
    throw 1234;
  }
  slide::DEG_ID deg{};

  auto c = Cell_SPM("cell_ancillary", deg, 1, 1, 1, 1);
  c.setBlockDegAndTherm(true);
  c.setT(21.0_degC);

  auto d = c; // Copy cell for discharge.

  if (enable_spme) {
    c.enableElectrolyteGradientModel(true);
    d.enableElectrolyteGradientModel(true);
  }

  auto &st = c.getStateObj();
  auto cyc = Cycler(&c, "charge");

  auto dcyc = Cycler(&d, "discharge");

  // Make cell empty!
  ThroughputData th{};
  cyc.CCCV(1, 2.7, 0.0001, 1, 0, th);
  cyc.rest(100, 1, 0, th);

  dcyc.CCCV(1, 4.2, 0.0001, 1, 0, th);
  dcyc.rest(100, 1, 0, th);

  // Start GITT test 20x0.05C pulse and 2 hr rest:
  const auto N_repeat{ 20 };     // Repeat 20 times.
  const auto t_pulse = 1 * 3600; // 1 hr pulse time.
  const auto t_rest = 2 * 3600;  // 2 hr rest time.
  const auto Crate = 0.05;
  auto current = Crate * c.Cap();

  const auto out_name = enable_spme ? "GITT_20x0.05C_1h_rest_2h_spme.csv" : "GITT_20x0.05C_1h_rest_2h.csv";
  std::ofstream out_GITT{ PathVar::results / out_name };
  out_GITT << "Time [s],Current [A],Terminal voltage [V],SOC [-],Temperature [K],SPMe enabled [-],"
           << "c_e_0 [mol m-3],c_e_1 [mol m-3],c_e_2 [mol m-3],"
           << "dc_e_0 [mol m-3],dc_e_1 [mol m-3],dc_e_2 [mol m-3],"
           << "electrolyte concentration span [mol m-3],electrolyte concentration avg deviation [mol m-3],electrolyte exchange factor deviation [-]\n";

  double t_all{};
  write_spme_trace_row(out_GITT, c, t_all);

  for (int i{}; i < N_repeat; i++) {
    c.setCurrent(-current);
    d.setCurrent(current);

    for (int j{}; j < t_pulse; j++) {
      write_spme_trace_row(out_GITT, c, t_all);

      c.timeStep_CC(1, 1);
      d.timeStep_CC(1, 1);

      t_all += 1;
    }

    c.setCurrent(0);
    d.setCurrent(0);

    for (int j{}; j < t_rest; j++) {
      write_spme_trace_row(out_GITT, c, t_all);

      c.timeStep_CC(1, 1);
      d.timeStep_CC(1, 1);

      t_all += 1;
    }
  }
  out_GITT.close();
}

inline void GITT_test_spme()
{
  GITT_test(true);
}

} // namespace slide::examples
