/* cloudgen_core.c -- Generating stochastic fractal clouds
   This file contains the core functions
   Copyright (C) 2003 Robin Hogan <r.j.hogan@reading.ac.uk> */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cloudgen.h"
#include "random.h"

#define PI 3.14159265358979323846

/* Logarithm of the Gamma function, adapted from Numerical Recipies */
static
float
gammaln(float xx)
{
  double x, tmp, ser;
  static double cof[6] = {76.18009173, -86.50532033, 24.01409822,
			  -1.231739516, 0.120858003e-2, -0.536382e-5};
  int j;
  x = xx-1.0;
  tmp = x+5.5;
  tmp -= (x+0.5) * log(tmp);
  ser = 1.0;
  for (j = 0; j <= 5; j++) {
    x += 1.0;
    ser += cof[j]/x;
  }
  return -tmp+log(2.50662827465*ser);
}


/* Create a cloudgen field, set the size of the cloud fields to
   generate, and allocate the various arrays that are
   required. Returns NULL if there is a problem allocating the
h   memory. */
cg_field *
cg_new_multi_field(int nx, int ny, int nz,
		   real dx, real dy, real dz,
		   real x_offset, real y_offset, real z_offset,
		   int nvars)
{
  cg_field *field;
  real *kx, *ky, *kz;
  real *x, *y, *z;
  real dkx, dky, dkz; /* x and y wavenumber intervals */
  int i;

  /* We are generating a matrix of real numbers, so the length of the
     arrays is slightly larger than half that which would be required
     for a full complex fft. */
  long int len = (nx/2+1) * ny * nz;
  
  int planar_rank = 2;
  int planar_shape[] = {ny, nx};
  int planar_size_c = ny * (nx/2 + 1);
  int planar_size_r = ny * 2 * (nx / 2 + 1);

  /* Check range of nvars */
  if (nvars < 1 || nvars > CG_MAX_VARS) {
    return NULL;
  }

  /* Allocate memory for field structure */
  field = malloc(sizeof(cg_field));
  if (!field) {
    /* Out of memory */
    return NULL;
  }

  field->nvars = nvars;
  for (i = 0; i < nvars; i++) {
    int j;
    /* Allocate memory for the Fourier components */
    field->p[i] = fftw_malloc(len * sizeof(complex));
    if (!field->p[i]) {
      /* Out of memory */
      for (j = 0; j < i; j++) {
	free(field->p[j]);
      }
      free(field);
      return NULL;
    }
    /* We are using `in place' fftw functions, so the output field
       occupies the same memory as the input */
    field->field[i] = (real *) field->p[i];
  }
  for (i = nvars; i < CG_MAX_VARS; i++) {
    field->field[i] = NULL;
    field->p[i] = NULL;
  }

  fftw_plan fft_plan = fftw_plan_dft_c2r_3d(nz, ny, nx, field->p[0], field->field[0], FFTW_ESTIMATE);
  fftw_plan fft_plan_2d_1 = fftw_plan_many_dft_r2c(planar_rank, planar_shape, nz,
                                            field->field[0], NULL, 1, planar_size_r,
                                            field->p[0], NULL, 1, planar_size_c,
                                            FFTW_ESTIMATE);
  fftw_plan fft_plan_2d_2 = fftw_plan_many_dft_c2r(planar_rank, planar_shape, nz,
                                            field->p[0], NULL, 1, planar_size_c,
                                            field->field[0], NULL, 1, planar_size_r,
                                            FFTW_ESTIMATE);

  if (!fft_plan || !fft_plan_2d_1 || !fft_plan_2d_2) {
    /* Out of memory or incorrect arguments to fftw_create_plan */
    return NULL;
  }

  /* Allocate memory for x and y wavenumbers */
  kx = malloc(nx * sizeof(real));
  ky = malloc(ny * sizeof(real));
  kz = malloc(nz * sizeof(real));
  x = malloc(nx * sizeof(real));
  y = malloc(ny * sizeof(real));
  z = malloc(nz * sizeof(real));

  if (!kx || !ky || !kz || !x || !y || !z) {
    /* Out of memory */
    fftw_free(field);
    return NULL;
  }

  for (i = 0; i < nx; i++) {
    x[i] = x_offset + i*dx;
  }
  for (i = 0; i < ny; i++) {
    y[i] = y_offset + i*dy;
  }
  for (i = 0; i < nz; i++) {
    z[i] = z_offset + i*dz;
  }

  /* Fill wavenumber vectors */
  dkx = 1.0 / (nx * dx);
  for (i = 0; i <= nx/2; i++) {
    kx[i] = i * dkx;
  }
  for (; i < nx; i++) {
    kx[i] = -(nx-i) * dkx;
  }

  dky = 1.0 / (ny * dy);
  for (i = 0; i <= ny/2; i++) {
    ky[i] = i * dky;
  }
  for (; i < ny; i++) {
    ky[i] = -(ny-i) * dky;
  }

  dkz = 1.0 / (nz * dz);
  for (i = 0; i <= nz/2; i++) {
    kz[i] = i * dkz;
  }
  for (; i < nz; i++) {
    kz[i] = -(nz-i) * dkz;
  }

  field->fft_plan = fft_plan;
  field->fft_plan_2d_1 = fft_plan_2d_1;
  field->fft_plan_2d_2 = fft_plan_2d_2;

  field->kx = kx;
  field->ky = ky;
  field->kz = kz;
  field->x = x;
  field->y = y;
  field->z = z;
  field->dx = dx;
  field->dy = dy;
  field->dz = dz;
  field->dkx = dkx;
  field->dky = dky;
  field->dkz = dkz;
  field->nx = nx;
  field->ny = ny;
  field->nz = nz;

  return field;
}

/* Free all the memory allocated in field */
void
cg_delete_field(cg_field *field)
{
  int i;
  if (!field) {
    return;
  }
  if (field->fft_plan) {
    fftw_destroy_plan(field->fft_plan);
  }
  for (i = 0; i < field->nvars; i++) {
    if (field->p[i]) {
      fftw_free(field->p[i]);
    }
  }
  if (field->kx) {
    free(field->kx);
  }
  if (field->ky) {
    free(field->ky);
  }
  if (field->kz) {
    free(field->kz);
  }
  free(field);
}

void
cg_delete_last_variable(cg_field *field)
{
  int i = field->nvars-1;
  if (i >= 0 && field->p[i]) {
    free(field->p[i]);
    field->p[i] = NULL;
  }
  --(field->nvars);
}

/* Set the mean spectral energy density - a power law with a scale
   break at outer_scale, a slope of "slope" at small scales and
   "outer_slope" at large scales. Maths follows Hogan and Kew. */
void
cg_power_law(cg_field *field, int ivar, real outer_scale,
	     real slope, real outer_slope)
{
  complex *p = field->p[ivar];
  real *kx = field->kx;
  real *ky = field->ky;
  real *kz = field->kz;
  real dkx = field->dkx;
  real dkz = field->dkz;
  real max_kx = field->nx * dkx *0.5;
  int i, j, k;
  int nx = field->nx;
  int ny = field->ny;
  int nz = field->nz;
  /* Locations of scale breaks: note that we use k^2 rather than k for
     efficiency */
  real gamma_factor = dkz * exp(gammaln(-0.5*slope)-gammaln(0.5-0.5*slope));
  real kk_I = 1./(outer_scale * outer_scale);
  real kk_II =  gamma_factor*gamma_factor * slope*slope * 0.25 / PI;
  real kk_III = max_kx * max_kx * 2 * (-slope) / PI;
  /* Coefficients: note that we are working in amplitude not frequency
     space, leading to the square-roots */
  real coefft_II = sqrt(1/(gamma_factor * sqrt(PI)));
  real coefft_I = coefft_II * pow(kk_I, (slope-outer_slope-1)*0.25);
  real coefft_III = sqrt(-0.5 * slope / PI);
  real coefft_IV = sqrt(0.25 / (max_kx * max_kx));

  for (k = 0; k < nz; k++) {
    for (j = 0; j < ny; j++) {
      for (i = 0; i < (nx/2+1); i++) {
	real kk = kx[i]*kx[i] + ky[j]*ky[j] + kz[k]*kz[k];
	real value;
	complex *target = &(p[i + (nx/2+1)*(j + ny*k)]);
	if (kk < kk_I) {
	  /* Region I: outer scale */
	  value = coefft_I * pow(kk, outer_slope*0.25);
	}
	else if (kk < kk_II) {
	  /* Region II: quasi-2D behaviour (x-y) */
	  value = coefft_II * pow(kk, (slope-1)*0.25);
	}
	else if (kk < kk_III) {
	  /* Region III: 3D behaviour */
	  value = coefft_III * pow(kk, (slope-2)*0.25);
	}
	else {
	  /* Region IV: quasi-1D behaviour (z) */
	  value = coefft_IV * pow(kk, slope*0.25);
	}
	*target *= value + value * I;
      }
    }
  }
  *p = 0.0 + 0.0 * I;
}

/* Set the mean spectral energy density - a power law with a scale
   break at outer_scale, a slope of "slope" at small scales and
   "outer_slope" at large scales. */
void
cg_power_laws(cg_field *field, real outer_scale,
	      real *slope, real *outer_slope)
{
  complex **p = field->p;
  real *kx = field->kx;
  real *ky = field->ky;
  real *kz = field->kz;
  real dkx = field->dkx;
  real dkz = field->dkz;
  real max_kx = field->nx * dkx *0.5;
  int i, j, k, n;
  int nx = field->nx;
  int ny = field->ny;
  int nz = field->nz;

  real gamma_factor[CG_MAX_VARS];
  real kk_I;
  real kk_II[CG_MAX_VARS];
  real kk_III[CG_MAX_VARS];
  real coefft_I[CG_MAX_VARS];
  real coefft_II[CG_MAX_VARS];
  real coefft_III[CG_MAX_VARS];
  real coefft_IV;

  kk_I = 1./(outer_scale * outer_scale);
  for (n = 0; n < field->nvars; n++) {
    /* Locations of scale breaks: note that we use k^2 rather than k for
       efficiency */
    gamma_factor[n] = dkz * exp(gammaln(-0.5*slope[n])
				-gammaln(0.5-0.5*slope[n]));
    kk_II[n] =  gamma_factor[n]*gamma_factor[n] 
      * slope[n]*slope[n] * 0.25 / PI;
    kk_III[n] = -slope[n] * max_kx * max_kx * 2 / PI;
    /* Coefficients: note that we are working in amplitude not frequency
       space, leading to the square-roots */
    coefft_II[n] = sqrt(1/(gamma_factor[n] * sqrt(PI)));
    coefft_I[n] = coefft_II[n]
      * pow(kk_I, (slope[n]-outer_slope[n]-1)*0.25);
    coefft_III[n] = sqrt(-0.5 * slope[n] / PI);
  }
  coefft_IV = sqrt(0.25 / (max_kx * max_kx));

  for (k = 0; k < nz; k++) {
    for (j = 0; j < ny; j++) {
      for (i = 0; i < (nx/2+1); i++) {
	real kk = kx[i]*kx[i] + ky[j]*ky[j] + kz[k]*kz[k];
	real value;
	int index = i + (nx/2+1)*(j + ny*k);
	for (n = 0; n < field->nvars; n++) {
	  if (kk < kk_I) {
	    /* Region I: outer scale */
	    value = coefft_I[n] * pow(kk, outer_slope[n]*0.25);
	  }
	  else if (kk < kk_II[n]) {
	    /* Region II: quasi-2D behaviour (x-y) */
	    value = coefft_II[n] * pow(kk, (slope[n]-1)*0.25);
	  }
	  else if (kk < kk_III[n]) {
	    /* Region III: 3D behaviour */
	    value = coefft_III[n] * pow(kk, (slope[n]-2)*0.25);
	  }
	  else {
	    /* Region IV: quasi-1D behaviour (z) */
	    value = coefft_IV * pow(kk, slope[n]*0.25);
	  }
	  p[n][index] *= value;
	}
      }
    }
  }
  for (n = 0; n < field->nvars; n++) {
    p[n][0] = 0.0 + 0.0 * I;
  }
}

/* Fill set every amplitude to 1+0i: this is useful for testing the
   power law function. */
void
cg_unity_phase(cg_field *field, int ivar)
{
  complex *p = field->p[ivar];
  long int n;
  long int len = (field->nx/2+1) * field->ny * field->nz;
  p[0] = 0.0 + 0.0 * I;

  for (n = 1; n < len; n++) {
    p[n] = 1.0 + 0.0 * I;
  }
}

/* Convert mean spectral energies into Fourier coefficients with a
   random phase, derived by calls to gaussian_deviate(). */
void
cg_random_phase(cg_field *field, int ivar)
{
  complex *p = field->p[ivar];
  long int n;
  long int len = (field->nx/2+1) * field->ny * field->nz;
  real rr, ii;

  p[0] = 0.0 + 0.0 * I;

  for (n = 1; n < len; n++) {
    rr = gaussian_deviate();
    ii = gaussian_deviate();
    p[n] = rr + ii * I;
  }
}

/* Create a field of random phases in ivar that is partially
   correlated with the random phases in iorig. */
void
cg_correlated_phase(cg_field *field, int ivar, int iorig, real correlation)
{
  complex *p = field->p[ivar];
  complex *p_orig = field->p[iorig];
  long int n;
  long int len = (field->nx/2+1) * field->ny * field->nz;
  real rr, ii;

  p[0] = 0.0 + 0.0 * I;

  if (correlation <= 0.0) {
    /* Use completely new random numbers */
    for (n = 1; n < len; n++) {
      rr = gaussian_deviate();
      ii = gaussian_deviate();
      p[n] = rr + ii * I;
    }
  }
  else if (correlation >= 1.0) {
    /* Copy values over */
    memcpy(p, p_orig, len*sizeof(complex));
  }
  else {
    /* Use weighting of new random number
       and those from another variable */
    real comp_correlation = 1.0-correlation;
    for (n = 1; n < len; n++) {
      rr = gaussian_deviate();
      ii = gaussian_deviate();
      p[n] = correlation * p_orig[n]
          + comp_correlation * (rr + ii * I);
    }
  }
}

/* Perform inverse 3D Fourier transform to generate initial
   isotropic fractal field. The result is held in field as well as
   being returned. */
real *
cg_generate_fractal(cg_field *field)
{
  int n;
  for (n = 0; n < field->nvars; n++) {
    fftw_execute_dft_c2r(field->fft_plan, field->p[n], field->field[n]);
  }

  return field->field[0];
}


/* Replace all values below threshold with missing_value */
void
cg_threshold(cg_field *field, int ivar, real threshold, real missing_value)
{
  real *data = field->field[ivar];
  int i, j, k, n;
  int nx = field->nx;
  int ny = field->ny;
  int nz = field->nz;

  for (k = 0; k < nz; k++) {
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	int index = i + (nx+2)*(j + ny*k);
	if (data[index] < threshold) {
	  for (n = 0; n < field->nvars; n++) {
	    field->field[n][index] = missing_value;
	  }
	}
      }
    }
  }
}

/* Scale the field to obtain a standard deviation of
   "std" and a mean of "mean". */
void
cg_scale(cg_field *field, int ivar, real std, real mean)
{
  real *data = field->field[ivar];
  int i, j, k;
  int nx = field->nx;
  int ny = field->ny;
  int nz = field->nz;
  double scale = 0.0;
  double sum2 = 0.0;

  for (k = 0; k < nz; k++) {
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	int index = i + (nx+2)*(j + ny*k);
	sum2 += data[index]*data[index];
      }
    }
  }
  scale = std/sqrt(sum2/(nx*ny*nz));

  for (k = 0; k < nz; k++) {
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	int index = i + (nx+2)*(j + ny*k);
	data[index] = mean + data[index]*scale;
      }
    }
  }
}

/* As cg_scale(), but with an exponentiation of the field such that it
   conforms to a lognormal distribution. In this case, "std" refers to
   the fractional standard deviation, i.e. the standard deviation of
   the natural logarithm of the final field. "mean" still refers to
   the requested mean of the final field. */
void
cg_lognormal(cg_field *field, int ivar, real std, real mean)
{
  real *data = field->field[ivar];
  int i, j, k;
  int nx = field->nx;
  int ny = field->ny;
  int nz = field->nz;
  int len = nx*ny*nz;
  double pre_scale = 0.0;
  double post_scale = 0.0;
  double sum2 = 0.0;

  for (k = 0; k < nz; k++) {
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	int index = i + (nx+2)*(j + ny*k);
	sum2 += data[index]*data[index];
      }
    }
  }
  pre_scale = std/sqrt(sum2/len);
  post_scale = mean/exp(0.5*std*std);

  for (k = 0; k < nz; k++) {
    for (j = 0; j < ny; j++) {
      for (i = 0; i < nx; i++) {
	int index = i + (nx+2)*(j + ny*k);
	data[index] = exp(data[index] * pre_scale) * post_scale;
      }
    }
  }
}

/* Shuffle the data to remove the 2-float padding at the end of
   every row */
void
cg_squeeze(cg_field *field)
{
  int i, j, k, v;
  int nx = field->nx;
  int ny = field->ny;
  int nz = field->nz;

  for (v = 0; v < field->nvars; v++) {
    real *data = field->field[v];
    for (k = 0; k < nz; k++) {
      for (j = 0; j < ny; j++) {
	int old_offset = (nx+2)*(j + ny*k);
	int new_offset = nx*(j + ny*k);
	for (i = 0; i < nx; i++) {
	  data[new_offset+i] = data[old_offset+i];
	}
      }
    }
  }
}

void
cg_dump_field(FILE * handle, cg_field * field) {
  int size, i, j;
  fftw_fprint_plan(field->fft_plan, handle);
  fprintf(handle, "\n");
  fftw_fprint_plan(field->fft_plan_2d_1, handle);
  fprintf(handle, "\n");
  fftw_fprint_plan(field->fft_plan_2d_2, handle);
  fprintf(handle, "\n%d\n%d\n%d\n%d", field->nx, field->ny, field->nz,
          field->nvars);
  size = 2 * (field->nx / 2 + 1) * field->ny * field->nz;
  for (j = 0; j < field->nvars; j++) {
    fprintf(handle, "\n%f", field->field[j][0]);
    for (i = 1; i < size; i++) {
      fprintf(handle, " %f", field->field[j][i]);
    }
  }

  fprintf(handle, "\n%f", field->kx[0]);
  for (i = 1; i < field->nx; i++) {
    fprintf(handle, " %f", field->kx[i]);
  }
  fprintf(handle, "\n%f", field->ky[0]);
  for (i = 1; i < field->ny; i++) {
    fprintf(handle, " %f", field->ky[i]);
  }
  fprintf(handle, "\n%f", field->kz[0]);
  for (i = 1; i < field->nz; i++) {
    fprintf(handle, " %f", field->kz[i]);
  }

  fprintf(handle, "\n%f", field->x[0]);
  for (i = 1; i < field->nx; i++) {
    fprintf(handle, " %f", field->x[i]);
  }
  fprintf(handle, "\n%f", field->y[0]);
  for (i = 1; i < field->ny; i++) {
    fprintf(handle, " %f", field->y[i]);
  }
  fprintf(handle, "\n%f", field->z[0]);
  for (i = 1; i < field->nz; i++) {
    fprintf(handle, " %f", field->z[i]);
  }

  fprintf(handle, "\n%f\n%f\n%f", field->dx, field->dy, field->dz);
  fprintf(handle, "\n%f\n%f\n%f\n", field->dkx, field->dky, field->dkz);
}
