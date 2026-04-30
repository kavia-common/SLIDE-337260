/*
 * running_Cell_Bucket.hpp
 *
 *  Benchmark file for Cell_Bucket
 *
 *  Created on: 07 Aug 2022
 *   Author(s): Volkan Kumtepeli
 */

#pragma once

#include "../src/slide.hpp"

#include <string>
#include <fstream>
#include <iostream>

namespace slide::benchmarks {

inline void write_spme_benchmark_header(std::ofstream &file)
{
  file << "time [s],V [V],SOC [-],T [K],time throughput [s],Ah throughput [Ah],Wh throughput [Wh],OCV [V],"
       << "SPMe enabled [-],c_e_0 [mol m-3],c_e_1 [mol m-3],c_e_2 [mol m-3],"
       << "dc_e_0 [mol m-3],dc_e_1 [mol m-3],dc_e_2 [mol m-3],"
       << "electrolyte concentration span [mol m-3],electrolyte concentration avg deviation [mol m-3],electrolyte exchange factor deviation [-]\n";
}

inline void write_spme_benchmark_row(std::ofstream &file, Cell_SPM &c, double t_now)
{
  const auto ce = c.getElectrolyteConcentrationProfile();
  const auto ce_dev = c.getElectrolyteConcentrationDeviationProfile();
  const auto diagnostics = c.getElectrolyteDiagnosticVoltages();

  file << t_now << ',' << c.V() << ',' << c.SOC() << ',' << c.T() << ','
       << c.getThroughputs().time() << ',' << c.getThroughputs().Ah() << ','
       << c.getThroughputs().Wh() << ',' << c.getOCV() << ','
       << (c.isElectrolyteGradientModelEnabled() ? 1 : 0);

  for (const auto value : ce)
    file << ',' << value;
  for (const auto value : ce_dev)
    file << ',' << value;
  for (const auto value : diagnostics)
    file << ',' << value;

  file << '\n';
}

inline void run_Cell_SPM_1(double Crate)
{
  std::string ID = "PyBAMM_1_CC_Crate"; // + std::to_string(Crate) + '_'
  auto c = Cell_SPM();

  const auto Idisch = Crate * c.Cap();

  c.getStateObj().rDCn() = 0;
  c.getStateObj().rDCp() = 0;
  c.getStateObj().rDCcc() = 0;

  std::cout << "Vbefore: " << c.V() << '\n';
  c.setBlockDegAndTherm(true);
  ThroughputData th{};

  auto cyc = Cycler(&c, ID);

  Clock clk;
  cyc.CC(Idisch, 2.7, 3600, 0.5, 2, th);
  std::cout << "Finished " << ID << " in " << clk << ".\n";

  std::cout << "Vafter: " << c.V() << '\n';

  cyc.writeData();
}

inline void run_Cell_SPM_2(double Crate, bool enable_spme = false)
{
  std::string ID = enable_spme ? "PyBAMM_2_CC_Crate_Cell_SPM_cellData_spme" : "PyBAMM_2_CC_Crate_Cell_SPM_cellData";
  auto c = Cell_SPM();

  if (enable_spme)
    c.enableElectrolyteGradientModel(true);

  const auto Idisch = 1.0; // Crate * c.Cap();

  c.getStateObj().rDCn() = 0;
  c.getStateObj().rDCp() = 0;
  c.getStateObj().rDCcc() = 0;

  std::cout << "Vbefore: " << c.V() << '\n';
  c.setBlockDegAndTherm(true);
  ThroughputData th{};

  c.setCurrent(Idisch);

  const double T_end{ 3600 }, dt{ 0.5 };
  double t_now{ 0 };
  Clock clk;

  std::ofstream file{ PathVar::results / (ID + ".csv"), std::ios::out };
  write_spme_benchmark_header(file);

  while (t_now <= T_end && c.V() > 2.7) {
    write_spme_benchmark_row(file, c, t_now);

    c.timeStep_CC(dt, 2);

    t_now += dt * 2;
  }

  std::cout << "Finished " << ID << " in " << clk << ".\n";
  std::cout << "Vafter: " << c.V() << '\n';

  file.close();
}

inline void run_Cell_SPM_spme(double Crate)
{
  run_Cell_SPM_2(Crate, true);
}

} // namespace slide::benchmarks
