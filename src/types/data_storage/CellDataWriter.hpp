/*
 * CellDataWriter.hpp
 *
 * Created on: 10 Apr 2022
 * Author(s): Jorn Reniers, Volkan Kumtepeli
 */

#pragma once

#include "cell_data.hpp"
#include "../../settings/enum_definitions.hpp"
#include "../../utility/free_functions.hpp"
#include "../../cells/Cell_SPM/Cell_SPM.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <span>
#include <cstdlib>
#include <array>
#include <span>
#include <variant>
#include <type_traits>

namespace slide {

inline void writeData(std::ofstream &file, std::span<Histogram<>> histograms)
{
  for (auto &hist : histograms)
    file << hist << "\n\n";
}

template <typename Cell_t>
void writeCellDataHeader(std::ofstream &file, const Cell_t &cell)
{
  file << "I [A],V [V],SOC [-],T [K],time [s],Ah [Ah],Wh [Wh]";

  if constexpr (std::is_base_of_v<Cell_SPM, std::remove_cvref_t<Cell_t>>) {
    file << ",SPMe enabled [-]";
    for (size_t i = 0; i < State_SPM::nce; i++)
      file << ",c_e_" << i << " [mol m-3]";
    for (size_t i = 0; i < State_SPM::nce; i++)
      file << ",dc_e_" << i << " [mol m-3]";

    file << ",electrolyte concentration span [mol m-3]"
         << ",electrolyte concentration avg deviation [mol m-3]"
         << ",electrolyte exchange factor deviation [-]";
  }

  file << '\n';
}

template <typename Cell_t>
void writeCellDataRow(std::ofstream &file, Cell_t &cell, const std::span<double> row)
{
  for (size_t i = 0; i < row.size(); i++) {
    if (i != 0)
      file << ',';
    file << row[i];
  }

  if constexpr (std::is_base_of_v<Cell_SPM, std::remove_cvref_t<Cell_t>>) {
    file << ',' << (cell.isElectrolyteGradientModelEnabled() ? 1 : 0);

    const auto ce = cell.getElectrolyteConcentrationProfile();
    for (const auto ce_i : ce)
      file << ',' << ce_i;

    const auto ce_dev = cell.getElectrolyteConcentrationDeviationProfile();
    for (const auto ce_i : ce_dev)
      file << ',' << ce_i;

    const auto diagnostics = cell.getElectrolyteDiagnosticVoltages();
    for (const auto diag_i : diagnostics)
      file << ',' << diag_i;
  }

  file << '\n';
}

inline void writeVarAndStates(std::ofstream &file, auto &cell)
{
  file << "States:,";                       // #TODO we need names for states.
  for (const auto st_i : cell.viewStates()) // Time and Throughput data is written here if available.
    file << st_i << ',';
  file << "\n\n\n";
}

template <settings::cellDataStorageLevel N>
void writeDataImpl(std::ofstream &file, auto &cell, auto &dataStorage)
{
  if constexpr (settings::data::writeCumulativeData)
    writeVarAndStates(file, cell);

  if constexpr (N >= settings::cellDataStorageLevel::storeHistogramData) {
    if constexpr (N == settings::cellDataStorageLevel::storeTimeData) {
      constexpr size_t base_width = 7;
      writeCellDataHeader(file, cell);

      for (size_t i = 0; i + base_width <= dataStorage.data.size(); i += base_width) {
        std::span<double> row(dataStorage.data.data() + i, base_width);
        writeCellDataRow(file, cell, row);
      }
    } else {
      free::write_data(file, dataStorage.data, 7);
    }
  }
  //!< else write nothing.
}

template <settings::cellDataStorageLevel N>
struct CellDataWriter
{
  /*
   * Writes cell data to a csv file.
   * The name of the csv file starts with the value of prefix,
   * after which the identification string of this cell is appended
   *
   * Depending on the value of DATASTORE_CELL, different things are written
   * 	0 	nothing
   * 	1 	general info about the cell and usage statistics in file xxx_cellStats.csv
   * 	2 	cycling data (I, V, T at every time step) in file xxx_cellData.csv
   */

  inline static void writeData(auto &cell, const std::string &prefix, auto &storage)
  {
    constexpr auto suffix = "cellData.csv";
    auto file = free::openFile(cell, PathVar::results, prefix, suffix);
    writeDataImpl<N>(file, cell, storage);
    file.close();
  }
};

} // namespace slide
