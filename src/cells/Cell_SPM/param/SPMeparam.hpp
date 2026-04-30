#pragma once

#include <array>

namespace slide::param {

/**
 * Parameter bundle for the opt-in SPMe electrolyte extension.
 *
 * These values are intentionally lightweight and default-safe:
 * - `enabled` defaults to false so baseline SPM behavior remains unchanged.
 * - `c_elec_init` and `ce_deviation_init` define the electrolyte initial condition.
 * - transport / geometry terms are stored here so chemistry variants can provide
 *   explicit defaults without altering legacy constructor signatures.
 */
struct SPMeParam
{
  bool enabled{ false }; //!< Opt-in switch for electrolyte-gradient dynamics.

  double c_elec{ 1000.0 }; //!< Bulk electrolyte concentration [mol m-3].
  double c_elec_init{ 1000.0 }; //!< Initial electrolyte concentration baseline [mol m-3].

  double electrolyte_diffusion{ 2.8e-10 }; //!< Effective electrolyte diffusion coefficient [m2 s-1].
  double coupling{ 1.0 }; //!< Scaling factor for the simplified electrolyte source term [-].

  double bruggeman{ 1.5 }; //!< Effective transport Bruggeman coefficient [-].
  double tortuosity{ 1.0 }; //!< Effective electrolyte tortuosity factor [-].
  double transference{ 0.4 }; //!< Cation transference number [-].
  double thermodynamic_factor{ 1.0 }; //!< Thermodynamic correction factor [-].

  double porosity_separator{ 0.45 }; //!< Separator porosity [-].
  double porosity_pos{ 0.30 }; //!< Positive-electrode electrolyte porosity [-].
  double porosity_neg{ 0.30 }; //!< Negative-electrode electrolyte porosity [-].

  double thickness_separator{ 25e-6 }; //!< Separator thickness [m].
  double thickness_pos{ 70e-6 }; //!< Positive electrode thickness used by SPMe helper terms [m].
  double thickness_neg{ 73.5e-6 }; //!< Negative electrode thickness used by SPMe helper terms [m].

  std::array<double, 3> ce_deviation_init{ 0.0, 0.0, 0.0 }; //!< Initial deviation from bulk electrolyte concentration [mol m-3].
};

} // namespace slide::param
