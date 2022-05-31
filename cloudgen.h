/* cloudgen.h -- Generating stochastic fractal clouds
   Copyright (C) 2003 Robin Hogan <r.j.hogan@reading.ac.uk> */

#ifndef _CLOUDGEN_H
#define _CLOUDGEN_H 1

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

  /* This code uses the same precision as the fftw library;
     single-precision is sufficiently accurate. */
#include <complex.h>
#include <fftw3.h>

#include "config.h"

#define CG_MAX_VARS 16
  
  /* This structure contains the cloud field information */
  typedef struct {
    fftw_plan fft_plan;         /* The initial inverse 3D transform */
    fftw_plan fft_plan_2d_1;    /* The forward 2D transforms */
    fftw_plan fft_plan_2d_2;    /* The inverse 2D transforms */
    complex *p[CG_MAX_VARS];    /* Fourier components (size (nx/2+1)*ny*nz) */
    real *field[CG_MAX_VARS];   /* Output field (points to same memory as p) */
    real *kx, *ky, *kz; /* wavenumber vectors */
    real *x, *y, *z;    /* coordinate vectors */
    real dx, dy, dz;    /* pixels sizes */
    real dkx, dky, dkz; /* wavenumber intervals */
    int nx, ny, nz;     /* number of pixels in the x, y and z directions */
    int nvars;
  } cg_field;
  
  /* FUNCTIONS IN cloudgen_core.c */
  /* The same memory is used to store the data at different stages of
     the processing, so it is important that the functions are called
     in the right order - generally the order shown here */

  /* Create a cloudgen field, set the size of the cloud fields to
     generate, and allocate the various arrays that are
     required. Returns NULL if there is a problem allocating the
     memory. */
  cg_field *cg_new_multi_field(int nx, int ny, int nz,
			       real dx, real dy, real dz,
			       real x_offset, real y_offset, real z_offset,
			       int nvars);
#define cg_new_field(nx, ny, nz, dx, dy, dz, x_offset, y_offset, z_offset) \
  cg_new_multi_field(nx, ny, nz, dx, dy, dz, x_offset, y_offset, z_offset, 1)


  /* Set the mean spectral energy density - a power law with a scale
     break at outer_scale, a slope of "slope" at small scales and
     "outer_slope" at large scales. */
  void cg_power_law(cg_field *field, int ivar, real outer_scale,
		    real slope, real outer_slope);

  /* Set the mean spectral energy density - a power law with a scale
     break at outer_scale, a slope of "slope" at small scales and
     "outer_slope" at large scales. */
  void cg_power_laws(cg_field *field, real outer_scale,
		     real *slope, real *outer_slope);

  /* Set a phase of 1+0i */
  void cg_unity_phase(cg_field *field, int ivar);

  /* Convert mean spectral energies into Fourier coefficients with a
     random phase, derived by calls to gaussian_deviate(). */
  void cg_random_phase(cg_field *field, int ivar);
  
  void cg_correlated_phase(cg_field *field, int ivar, int iorig,
			   real correlation);

  /* Perform inverse 3D Fourier transform to generate initial
     isotropic fractal field. The result is held in field as well as
     being returned. */
  real *cg_generate_fractal(cg_field *field);

  /* Replace all values below threshold with missing_value */
  void cg_threshold(cg_field *field, int ivar,
		    real threshold, real missing_value);
  
  /* Shuffle the data to remove the 2-float padding at the end of
     every row */
  void cg_squeeze(cg_field *field);

  /* Free only the last variable in field */
  void cg_delete_last_variable(cg_field *field);

  /* Free all the memory allocated in field */
  void cg_delete_field(cg_field *field);

  /* Scale the field to obtain a standard deviation of "std" and a
     mean of "mean". */
  void cg_scale(cg_field *field, int ivar, real std, real mean);

  /* As cg_scale(), but with an exponentiation of the field such that
     it conforms to a lognormal distribution. In this case, "std"
     refers to the fractional standard deviation, i.e. the standard
     deviation of the natural logarithm of the final field. "mean"
     still refers to the requested mean of the final field. */
  void cg_lognormal(cg_field *field, int ivar, real std, real mean);

  /* Dump a field to file.  This prints the components of the field in
     the order (fft_plan, fft_plan_2d_1, fft_plan_2d_2, nx, ny, nz,
     nvars, field, kx, ky, kz, x, y, x, dx, dy, dz, dkx, dky, and dkz).
     Arrays are printed on a single line separated by white space.
     Its primary use is for regression testing and debugging purposes. */
  void cg_dump_field(FILE * handle, cg_field * field);


  /* FUNCTIONS IN cloudgen_layers.c */

  /* Perform forward 2D Fourier transform on each horizontal layer of
     the field. */
  void cg_transform_layers(cg_field *field);

  /* Interpolate array "param", consisting of "n" floating point
     values at heights "height" on to the heights in "field". At
     heights outsight "height" the extreme values of "param" are
     used. A dynamically allocated array of size field->nz is
     returned, or NULL on failure.  */
  real *cg_interpolate_layers(cg_field *field, real *height,
			      real *param, int n);

  /* Calculate the horizontal displacements (m) from the
     horizontal-mean particle fall speeds (fall_speed in m/s) profile,
     the wind profile (u_wind and v_wind in m/s) and the level at
     which the fall streaks originalte (generating_level in m), based
     on Marshall's (1953) work. The result is returned in pointers to
     two dynamically allocated arrays ret_deltax and
     ret_deltay. Returns 1 on success and 0 on failure. */
  int cg_get_layer_displacements(cg_field *field, real *fall_speed,
				 real *u_wind, real *v_wind,
				 real generating_level,
				 real **ret_deltax, real **ret_deltay);

  /* Translate the field horizontally at each level by the amounts
     given in the arrays deltax and deltay. */
  void cg_translate_layers(cg_field *field, real *deltax, real *deltay);

  /* At each height in the field, change the slope of the power
     spectrum at scales smaller than outer_scale. The original slope
     is provided in old_slope and the new in the array new_slope,
     which should have field->nz elements. */
  void cg_change_slope_layers(cg_field *field, int ivar,
			      real outer_scale,
			      real *new_slope, real old_slope);

  /* As cg_change_slope() but the slope is only changed in one
     direction, to simulate shear-induced anisotropic mixing. The
     horizontal displacements are entered to determine in which
     direction to perform the mixing. */
  void cg_anisotropic_change_slope_layers(cg_field *field, int ivar, 
					  real outer_scale,
					  real *new_slope, real old_slope,
					  real *deltax, real *deltay);
  /* Perform inverse 2D Fourier transform on each horizontal layer to
     revert to real space. */
  void cg_revert_layers(cg_field *field);

  /* Scale the field at each height to obtain standard deviations
     close to "std" and means close to "mean". Note that the
     calculation allows for natural vertical variations in the fractal
     field so will not match the request exactly. */
  void cg_scale_layers(cg_field *field, int ivar, real *std, real *mean);

  /* As cg_scale_layers(), but with an exponentiation of the field
     such that it conforms to a lognormal distribution. In this case,
     "std" refers to the fractional standard deviation, i.e. the
     standard deviation of the natural logarithm of the final
     field. "mean" still refers to the requested horizontal mean of
     the final field. */
  void cg_lognormal_layers(cg_field *field, int ivar, real *std, real *mean);

  
#ifdef __cplusplus
}                               /* extern "C" */
#endif                          /* __cplusplus */

#endif
