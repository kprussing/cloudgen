/* cloudgen_layers.c -- Generating stochastic fractal clouds
   This file contains the code for operating on a field layer by layer
   Copyright (C) 2003 Robin Hogan <r.j.hogan@reading.ac.uk> */
#include <math.h>

#include "cloudgen.h"

#include <tgmath.h>
#include <stdlib.h>

#define PI 3.14159265358979323846
#define PI2 6.28318530717958647692

/* Perform forward 2D Fourier transform on each horizontal layer of
   the field. */
void
cg_transform_layers(cg_field *field)
{
  int n;
  for (n = 0; n < field->nvars; n++) {
    fftw_execute_dft_r2c(field->fft_plan_2d_1, field->field[n], field->p[n]);
  }
}

/* Perform inverse 2D Fourier transform on each horizontal layer to
   revert to real space. */
void
cg_revert_layers(cg_field *field)
{
  int n;
  for (n = 0; n < field->nvars; n++) {
    fftw_execute_dft_c2r(field->fft_plan_2d_2, field->p[n], field->field[n]);
  }
}

/* At each height in the field, change the slope of the power
   spectrum at scales smaller than outer_scale. The original slope
   is provided in old_slope and the new in the array new_slope,
   which should have field->nz elements. */
void
cg_change_slope_layers(cg_field *field, int ivar, real outer_scale,
		       real *new_slope, real old_slope)
{
  complex *p = field->p[ivar];
  real *kx = field->kx;
  real *ky = field->ky;
  int i, j, k;
  int nx = field->nx;
  int ny = field->ny;
  int nz = field->nz;

  real kk_outer = 1/(outer_scale*outer_scale);

  for (k = 0; k < nz; k++) {
    real power = 0.25*(new_slope[k]-old_slope);
    for (j = 0; j < ny; j++) { 
      for (i = 0; i < (nx/2+1); i++) {
	real kk = kx[i]*kx[i] + ky[j]*ky[j];
	if (kk > kk_outer) {
	  int index = i + (nx/2+1)*(j + ny*k);
	  real scaling = pow(kk/kk_outer, power);
	  p[index] *= scaling;
	}
      }
    }
  }
}

/* As cg_change_slope() but the slope is only changed in one
   direction, to simulate shear-induced anisotropic mixing. The
   horizontal displacements are entered to determine in which
   direction to perform the mixing. */
void
cg_anisotropic_change_slope_layers(cg_field *field, int ivar,
				   real outer_scale,
				   real *new_slope, real old_slope,
				   real *deltax, real *deltay)
{
  complex *p = field->p[ivar];
  real *kx = field->kx;
  real *ky = field->ky;
  int i, j, k;
  int nx = field->nx;
  int ny = field->ny;
  int nz = field->nz;

  real kk_outer = 1/(outer_scale*outer_scale);

  for (k = 0; k < nz; k++) {
    real theta, sin_theta, cos_theta, power;
    if (deltax[k] != 0.0 || deltay[k] != 0.0) {
      theta = atan2(deltax[k], deltay[k]);
    }
    else {
      /* Can't work out the orientation of the fall streak - use 0
	 radians*/
      theta = 0.0;
    }
    sin_theta = sin(theta);
    cos_theta = cos(theta);
    power = 0.25*(new_slope[k]-old_slope);
    for (j = 0; j < ny; j++) { 
      for (i = 0; i < (nx/2+1); i++) {
	real k_theta = kx[i]*sin_theta + ky[j]*cos_theta;
	real kk = k_theta*k_theta;
	if (kk > kk_outer) {
	  int index = i + (nx/2+1)*(j + ny*k);
	  real scaling = pow(kk/kk_outer, power);
	  p[index] *= scaling;
	}
      }
    }
  }
}
/* Translate the field horizontally at each level by the amounts
   given in the arrays deltax and deltay. */
void
cg_translate_layers(cg_field *field, real *deltax, real *deltay)
{
  real *kx = field->kx;
  real *ky = field->ky;
  int i, j, k, n;
  int nx = field->nx;
  int ny = field->ny;
  int nz = field->nz;

  for (n = 0; n < field->nvars; n++) {
    complex *p = field->p[n];
    
    for (k = 0; k < nz; k++) {
      for (j = 0; j < ny; j++) {
	for (i = 0; i < (nx/2+1); i++) {
	  int index = i + (nx/2+1)*(j + ny*k);
	  real angle = carg(p[index]);
	  real amp = fabs(p[index]);
	  angle -= PI2 * (kx[i] * deltax[k] + ky[j] * deltay[k]);
	  p[index] = amp * CMPLX(cos(angle), sin(angle));
	}
      }
    }
  }
}

/* Scale the field at each height to obtain standard deviations
   close to "std" and means close to "mean". Note that the
   calculation allows for natural vertical variations in the fractal
   field so will not match the request exactly. */
void
cg_scale_layers(cg_field *field, int ivar, real *std, real *mean)
{
  real *data = field->field[ivar];
  int i, j, k;
  int nx = field->nx;
  int ny = field->ny;
  int nz = field->nz;

  for (k = 0; k < nz; k++) {
    double sum2 = 0.0;
    real scale;
    real offset = mean[k];
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	real value = data[i + (nx+2)*(j + ny*k)];
	sum2 += value*value;
      }
    }
    scale = std[k]/sqrt(sum2/(nx*ny));
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	real *value = data + i + (nx+2)*(j + ny*k);
	*value = *value * scale + offset;
      }
    }
  }
}

/* As cg_scale_layers(), but with an exponentiation of the field such
   that it conforms to a lognormal distribution. In this case, "std"
   refers to the fractional standard deviation, i.e. the standard
   deviation of the natural logarithm of the final field. "mean" still
   refers to the requested horizontal mean of the final field. */
void
cg_lognormal_layers(cg_field *field, int ivar, real *std, real *mean)
{
  real *data = field->field[ivar];
  int i, j, k;
  int nx = field->nx;
  int ny = field->ny;
  int nz = field->nz;

  for (k = 0; k < nz; k++) {
    double sum2 = 0.0;
    real pre_scale;
    real post_scale = mean[k]/exp(0.5*std[k]*std[k]);
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	real value = data[i + (nx+2)*(j + ny*k)];
	sum2 += value*value;
      }
    }
    pre_scale = std[k]/sqrt(sum2/(nx*ny));
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	real *value = data + i + (nx+2)*(j + ny*k);
	*value = exp(*value * pre_scale) * post_scale;
      }
    }
  }
}

/* Interpolate array "param", consisting of "n" floating point
   values at heights "height" on to the heights in "field". At
   heights outsight "height" the extreme values of "param" are
   used. A dynamically allocated array of size field->nz is
   returned, or NULL on failure.  */
real *
cg_interpolate_layers(cg_field *field, real *height, real *param, int n)
{
  real *new_param;
  int k = 0, new_k = 0;

  if (!height || !param) {
    return NULL;
  }
  if (!(new_param = fftw_malloc(field->nz * sizeof(real)))) {
    return NULL;
  }

  for (new_k = 0; new_k < field->nz; new_k++) {
    /* Increase k until field->z < height */
    while (field->z[new_k] > height[k] && k < n-1) {
      k++;
    }
    if (k == 0 || field->z[new_k] >= height[k]) {
      /* We are either outside the range of interpolation or height ==
	 field->z: in either case take an individual param value */
      new_param[new_k] = param[k];
    }
    else {
      /* Linear interpolation */
      new_param[new_k] = (param[k]*(field->z[new_k]-height[k-1])
			  +param[k-1]*(height[k]-field->z[new_k]))
	/ (height[k] - height[k-1]);
    }
  }
  return new_param;
}

/* Calculate the horizontal displacements (m) from the horizontal-mean
   particle fall speeds (fall_speed in m/s) profile, the wind profile
   (u_wind and v_wind in m/s) and the level at which the fall streaks
   originalte (generating_level in m), based on Marshall's (1953)
   work. The result is returned in pointers to two dynamically
   allocated arrays ret_deltax and ret_deltay. Returns 1 on success
   and 0 on failure. */
int
cg_get_layer_displacements(cg_field *field, real *fall_speed,
			   real *u_wind, real *v_wind,
			   real generating_level,
			   real **ret_deltax, real **ret_deltay)
{
  int k = field->nz-1;
  real u_gen_lev = u_wind[k];
  real v_gen_lev = v_wind[k];
  real *new_deltax, *new_deltay;

  if (!fall_speed) {
    return 0;
  }
  new_deltax = fftw_malloc(field->nz * sizeof(real));
  new_deltay = fftw_malloc(field->nz * sizeof(real));
  if (!new_deltax || !new_deltay) {
    return 0;
  }

  new_deltax[k] = 0.0;
  new_deltay[k] = 0.0;
  for (k = field->nz-2; k >= 0; k--) {
    if (field->z[k] > generating_level) {
      new_deltax[k] = 0.0;
      new_deltay[k] = 0.0;
      u_gen_lev = u_wind[k];
      v_gen_lev = v_wind[k];
    }
    else {
      new_deltax[k] = new_deltax[k+1]
	+ (u_wind[k+1]+u_wind[k] - 2.0*u_gen_lev)
	* (field->z[k+1]-field->z[k]) / (fall_speed[k+1]+fall_speed[k]);
      new_deltay[k] = new_deltay[k+1]
	+ (v_wind[k+1]+v_wind[k] - 2.0*v_gen_lev)
	* (field->z[k+1]-field->z[k]) / (fall_speed[k+1]+fall_speed[k]);
    }
  }
  *ret_deltax = new_deltax;
  *ret_deltay = new_deltay;
  return 1;
}
