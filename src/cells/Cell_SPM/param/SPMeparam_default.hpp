#pragma once

#include "SPMeparam.hpp"

namespace slide::param::def {

constexpr SPMeParam SPMeParam_default{};

constexpr SPMeParam SPMeParam_Kokam{
  false,       // enabled
  1000.0,      // c_elec
  1000.0,      // c_elec_init
  2.8e-10,     // electrolyte_diffusion
  1.0,         // coupling
  1.5,         // bruggeman
  1.0,         // tortuosity
  0.4,         // transference
  1.0,         // thermodynamic_factor
  0.45,        // porosity_separator
  0.30,        // porosity_pos
  0.30,        // porosity_neg
  25e-6,       // thickness_separator
  70e-6,       // thickness_pos
  73.5e-6,     // thickness_neg
  { 0.0, 0.0, 0.0 }
};

constexpr SPMeParam SPMeParam_LGChemNMC{
  false,                  // enabled
  1000.0,                 // c_elec
  1000.0,                 // c_elec_init
  2.8e-10,                // electrolyte_diffusion
  1.0,                    // coupling
  1.5,                    // bruggeman
  1.0,                    // tortuosity
  0.4,                    // transference
  1.0,                    // thermodynamic_factor
  0.45,                   // porosity_separator
  0.30,                   // porosity_pos
  0.30,                   // porosity_neg
  25e-6,                  // thickness_separator
  70e-6,                  // thickness_pos
  1.170972150305478e-4,   // thickness_neg
  { 0.0, 0.0, 0.0 }
};

} // namespace slide::param::def
