/* main.c -- Create fractal cirrus clouds and save as NetCDF
   Copyright (C) 2003 Robin Hogan <r.j.hogan@reading.ac.uk> */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <netcdf.h>

#include "cloudgen.h"
#include "readconfig.h"
#include "random.h"
#include "nctools.h"

#include "config.h"

char verbose = 0;

/* Send a message to standard output if verbose is true.  */
static
void
chat(char *format, ...)
{
  if (verbose) {
    int status;
    va_list ap;
    va_start(ap, format);
    status = vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
  }
}

/* Check the return value from a call to a NetCDF function and quit
   semi-elegantly if an error occurred. */
int ncstatus;
#define nc_check(a) if ((ncstatus = (a)) != NC_NOERR) { \
    fprintf(stderr, "NetCDF error on line %d of %s: %s\n", __LINE__, \
      __FILE__, nc_strerror(ncstatus)); \
    exit(-1); \
}

/* Add a NetCDF dimension and a coordinate variable with axis, units
   and long_name attributes. */
static
void
add_dimension(int ncid, char *name, int n, int *dimid,
	      int *varid, char *long_name)
{
  char axis[2] = {'X', '\0'};
  nc_check(nc_def_dim(ncid, name, n, dimid));
  nc_check(nc_def_var(ncid, name, NC_FLOAT, 1, dimid, varid));
  nc_check(nc_put_att_text(ncid, *varid, "long_name",
			   strlen(long_name), long_name));
  nc_check(nc_put_att_text(ncid, *varid, "units", 1, "m"));
  axis[0] = toupper(*name);
  nc_check(nc_put_att_text(ncid, *varid, "axis", 1, axis));
}

/* Add a vector of floats with long_name and units attributes. */
static
void
add_vector(int ncid, char *name, int dimid,
	   int *varid, char *units, char *long_name)
{
  nc_check(nc_def_var(ncid, name, NC_FLOAT, 1, &dimid, varid));
  if (long_name) {
    nc_check(nc_put_att_text(ncid, *varid, "long_name",
			     strlen(long_name),long_name));
  }
  if (units) {
    nc_check(nc_put_att_text(ncid, *varid, "units",
			     strlen(units), units));
  }
}

/* Add a scalar float variable with long_name and units attributes. */
static
void
add_scalar(int ncid, char *name, int *varid, char *units, char *long_name)
{
  nc_check(nc_def_var(ncid, name, NC_FLOAT, 0, NULL, varid));
  if (long_name) {
    nc_check(nc_put_att_text(ncid, *varid, "long_name",
			     strlen(long_name),long_name));
  }
  if (units) {
    nc_check(nc_put_att_text(ncid, *varid, "units",
			     strlen(units), units));
  }
}

/* Add a scalar float variable with long_name and units attributes. */
static
void
add_scalar_int(int ncid, char *name, int *varid, char *units, char *long_name)
{
  nc_check(nc_def_var(ncid, name, NC_INT, 0, NULL, varid));
  if (long_name) {
    nc_check(nc_put_att_text(ncid, *varid, "long_name",
			     strlen(long_name),long_name));
  }
  if (units) {
    nc_check(nc_put_att_text(ncid, *varid, "units",
			     strlen(units), units));
  }
}

/* Show usage information and then quit. */
static
void
usage(char **argv)
{
  fprintf(stdout, "Usage: %s [param=value] [-param] config.dat\n", argv[0]);
  fprintf(stdout, "   Creates a stochastic cloud field and saves it as a NetCDF file based on\n"
	  "   the information in config.dat and the other arguments on the command line.\n"
	  "   Information is in the form of param-value pairs, either in the config file\n"
	  "   separated by whitespace, or on the command line as param=value or -param.\n"
	  "   Special arguments:\n"
	  "       -verbose   Report actions\n"
	  "       -version   Report the program version and quit\n"
	  "          -help   Show this message and quit\n");
  exit(0);
}

int
main(int argc, char **argv)
{
  /* Default values */
  real vertical_exponent = -2.0;
  char *output_filename = "out.nc";
  char *name = "data";
  char *long_name = "Cloud field";
  char *size_name="size";
  char *size_long_name="Particle size";
  char *units = "1";
  char *size_units = "1";
  char *comment = NULL;
  char *references = NULL;
  char *institution = NULL;
  char *user = NULL;
  char *title = NULL;
  real default_interp_height[] = {0.0};
  int n_interp = 0;
  int seed = 1;
  char *dev_random = NULL;
  real default_x_displacement[] = {0.0};
  real default_y_displacement[] = {0.0};
  real default_horizontal_exponent[] = {0.0};
  real default_std[] = {1.0};
  real default_mean[] = {1.0};
  real generating_level = -1.0e30;
  real wind_scale_factor = 1.0;

  real outer_scale = 1.0e5;
  real threshold = 0.0;
  real missing_value = 0.0;

  real *interp_height = default_interp_height;
  real *x_displacement = default_x_displacement;
  real *y_displacement = default_y_displacement;
  real *horizontal_exponent = default_horizontal_exponent;   
  real *std = default_std;
  real *mean = default_mean;
  real *u_wind = NULL;
  real *v_wind = NULL;
  real *fall_speed = NULL;

  real size_correlation = 1.0;
  real *size_mean = default_mean;
  real *size_std = default_std;

  real *grid_x_displacement = NULL;
  real *grid_y_displacement = NULL;
  real *grid_horizontal_exponent = NULL;
  real *grid_mean = NULL;
  real *grid_std = NULL;
  real *grid_u_wind = NULL;
  real *grid_v_wind = NULL;
  real *grid_fall_speed = NULL;

  real *grid_size_mean = NULL;
  real *grid_size_std = NULL;

  /* Other variables */
  rc_data *config;
  real dx;
  real dz;
  cg_field *field;
  int j, k;
  char is_lognormal = 0;
  char is_threshold = 0;
  char is_size = 0;
  int is_mean = 0;
  char *version = "Cloudgen version " PROJECT_VERSION;
  char *confstring = NULL;
  size_t start[3] = {0, 0, 0};
  size_t count[3] = {1, 1, 0};

  /* NetCDF identifiers */
  int ncid, fieldid;
  int xdimid, ydimid, zdimid;
  int xid, yid, zid;
  int meanid, stdid, deltaxid, deltayid, slopeid;
  int sizeid, size_meanid, size_stdid;
  int uwindid, vwindid, fallspeedid;
  int vertexponentid, genlevelid, outerscaleid, seedid;
  int dimids[3];

  /* Find the first config file on the command line. */
  int ifile = rc_get_file(argc, argv);

  if (!ifile) {
    /* No file given - assume command-line arguments contain all the
       information. */
    config = rc_read(NULL, stderr);
  }
  else {
    /* Read configuration information from the file. */
    config = rc_read(argv[ifile], stderr);
  }
  if (!config) {
    fprintf(stderr, "Error initializing configuration information\n");
    exit(1);
  }
  
  /* Supplement configuration information with command-line
     arguments. */
  rc_register_args(config, argc, argv);

  if (argc == 1) {
    fprintf(stderr, "No command-line arguments provided: using default parameters\n"
	    "Type \"%s -help\" for usage information\n", argv[0]);
  }

  /* See if -version or -help are on the command line. */
  if (rc_get_boolean(config, "version")) {
    fprintf(stdout, "Cloudgen " PROJECT_VERSION "\n");
    exit(0);
  }
  else if (rc_get_boolean(config, "help")) {
    usage(argv);
  }

  /* Read in variables */

  /* First whether to be verbose - if not then chat() does nothing. */
  verbose = rc_get_boolean(config, "verbose");

  /* Check precision */
#ifdef FFTW_ENABLE_FLOAT
  if (sizeof(real) != 4) {
    fprintf(stderr, "Compile error: FFTW_ENABLE_FLOAT defined but \"fftw_real\" is not equivalent to \"float\"\n");
    exit(1);
  }
  else {
    chat("Cloudgen " PROJECT_VERSION ": compiled to use single-precision internally");
  }
#else
  if (sizeof(real) != 8) {
    fprintf(stderr, "Compile error: FFTW_ENABLE_FLOAT undefined but \"fftw_real\" is not equivalent to \"double\"\n");
    exit(1);
  }
  else {
    chat("Cloudgen " PROJECT_VERSION ": compiled to use double-precision internally");
  }
#endif

  if (ifile) {
    chat("Reading configuration information from %s", argv[ifile]);
  }

  /* Read in scalars */
  rc_assign_string(config, "output_filename", &output_filename);
  rc_assign_string(config, "variable_name", &name);
  rc_assign_string(config, "long_name", &long_name);
  rc_assign_string(config, "units", &units);
  rc_assign_real(config, "vertical_exponent", &vertical_exponent);
  rc_assign_real(config, "outer_scale", &outer_scale);
  rc_assign_real(config, "generating_level", &generating_level);
  rc_assign_real(config, "wind_scale_factor", &wind_scale_factor);

  is_size = rc_get_boolean(config, "size_variable_name");
  if (is_size) {
    /* Read size parameters */
    rc_assign_string(config, "size_variable_name", &size_name);
    rc_assign_string(config, "size_long_name", &size_long_name);
    rc_assign_string(config, "size_units", &size_units);
    rc_assign_real(config, "size_correlation", &size_correlation);
  }

  /* Read strings */
  comment = rc_get_string(config, "comment");
  references = rc_get_string(config, "references");
  institution = rc_get_string(config, "institution");
  title = rc_get_string(config, "title");
  user = rc_get_string(config, "user");

  /* Do we threshold the field? */
  is_threshold = rc_assign_real(config, "threshold", &threshold);
  rc_assign_real(config, "missing_value", &missing_value);

  /* Seed the pseudo-random number generator - either with a specified seed
     or with a value taken from a Linux /dev/random type file. */
  rc_assign_int(config, "seed", &seed);
  if (rc_assign_string(config, "system_random_file", &dev_random)) {
    open_kernel_random_file(dev_random);
    seed = kernel_int_seed();
    close_kernel_random_file();
  }
  seed_random_number_generator(seed);

  /* The height dependent properties depend on the presence of interp_height. */
  n_interp = rc_assign_real_array(config, "interp_height",
					       &interp_height, 1);
  /* If interp_height is present then load in the vectors on the
     interp_height grid. */
  if (n_interp) {
    rc_assign_real_array_default(config, "x_displacement",
				  &x_displacement, n_interp, 0.0);
    rc_assign_real_array_default(config, "y_displacement", &y_displacement,
				  n_interp, 0.0);
    rc_assign_real_array_default(config, "horizontal_exponent",
				  &horizontal_exponent, n_interp, 0.0);
    rc_assign_real_array_default(config, "standard_deviation",
				  &std, n_interp, 1.0);
    is_mean = rc_assign_real_array_default(config, "mean", &mean, n_interp, 1.0);

    rc_assign_real_array(config, "u_wind", &u_wind, n_interp);
    rc_assign_real_array(config, "v_wind", &v_wind, n_interp);
    rc_assign_real_array(config, "fall_speed", &fall_speed, n_interp);
    if (is_size) {
      rc_assign_real_array_default(config, "size_standard_deviation",
				    &size_std, n_interp, 1.0);
      is_mean = rc_assign_real_array_default(config, "size_mean",
					      &size_mean, n_interp, 1.0);
    }
  }

  field = rc_generate_base_field(config);

  /* Interpolate vectors on to the field->z grid. */
  if (n_interp) {
    grid_x_displacement = cg_interpolate_layers(field, interp_height,
						x_displacement, n_interp);
    grid_y_displacement = cg_interpolate_layers(field, interp_height,
						y_displacement, n_interp);
    grid_horizontal_exponent = cg_interpolate_layers(field, interp_height,
						     horizontal_exponent,
						     n_interp);
    grid_std = cg_interpolate_layers(field, interp_height,
					  std, n_interp);
    grid_mean = cg_interpolate_layers(field, interp_height,
				      mean, n_interp);
    if (u_wind && v_wind && fall_speed) {
      int i;
      grid_u_wind = cg_interpolate_layers(field, interp_height,
					  u_wind, n_interp);
      grid_v_wind = cg_interpolate_layers(field, interp_height,
					  v_wind, n_interp);
      for (i = 0; i < field->nz; i++) {
	grid_u_wind[i] *= wind_scale_factor;
	grid_v_wind[i] *= wind_scale_factor;
      }
      grid_fall_speed = cg_interpolate_layers(field, interp_height,
					      fall_speed, n_interp);
      cg_get_layer_displacements(field, grid_fall_speed,
				 grid_u_wind, grid_v_wind,
				 generating_level, &grid_x_displacement,
				 &grid_y_displacement);
    }
    if (is_size) {
      grid_size_std = cg_interpolate_layers(field, interp_height,
					    size_std, n_interp);
      grid_size_mean = cg_interpolate_layers(field, interp_height,
					     size_mean, n_interp);
    }
  }

  /* Generate initial isotropic fractal. */
  chat("Generating random phases with seed %d", seed);
  cg_random_phase(field, 0);
  /*  cg_unity_phase(field, 0);*/

  if (is_size) {
    cg_correlated_phase(field, 1, 0, size_correlation);
  }

  chat("Calculating power law with exponent %g and outer scale %g m",
       vertical_exponent, outer_scale);
  cg_power_law(field, 0, outer_scale, vertical_exponent, 0.0);

  if (is_size) {
    cg_power_law(field, 1, outer_scale, vertical_exponent, 0.0);
  }

  chat("Generating fractal (inverse 3D Fourier transform)");
  cg_generate_fractal(field);

  /* If interp_height is present then manipulate the individual layers. */
  if (n_interp) {
    chat("Transforming individual layers (2D Fourier transforms)");
    cg_transform_layers(field);
    /* Manipulate 2D phases to simulate displacement and a different
       power spectrum */
    chat("Displacing layers horizontally");
    cg_translate_layers(field, grid_x_displacement, grid_y_displacement);
    if (rc_get_boolean(config, "anisotropic_mixing")) {
      chat("Changing spectral slope of each layer anisotropically");
      cg_anisotropic_change_slope_layers(field, 0, outer_scale,
					 grid_horizontal_exponent,
					 vertical_exponent,
					 grid_x_displacement,
					 grid_y_displacement);
      if (is_size) {
	cg_anisotropic_change_slope_layers(field, 1, outer_scale,
					   grid_horizontal_exponent,
					   vertical_exponent,
					   grid_x_displacement,
					   grid_y_displacement);
      }
    }
    else {
      chat("Changing spectral slope of each layer");
      cg_change_slope_layers(field, 0, outer_scale,
			     grid_horizontal_exponent, vertical_exponent);
      if (is_size) {
	cg_change_slope_layers(field, 1, outer_scale,
			       grid_horizontal_exponent, vertical_exponent);
      }
    }
    chat("Reverting layers (inverse 2D Fourier transforms)");
    cg_revert_layers(field);

    if (is_mean) {
      if ((is_lognormal = rc_get_boolean(config, "lognormal_distribution"))) {
	chat("Converting to lognormal distribution");
	cg_lognormal_layers(field, 0, grid_std, grid_mean);
      }
      else {
	chat("Scaling");
	cg_scale_layers(field, 0, grid_std, grid_mean);    
      }
    }
    if (is_size) {
      cg_lognormal_layers(field, 1, grid_size_std, grid_size_mean);
    }
  }
  
  /* Threshold the field. */
  if (is_threshold) {
    if (units[0] == '1' || units[1] == '\0') {
      chat("Thresholding field at %g", threshold);
    }
    else {
      chat("Thresholding field at %g %s", threshold, units);
    }
    cg_threshold(field, 0, threshold, missing_value);
  }

  /* Write a netcdf file */
  chat("Writing %s in %s", name, output_filename);
  nc_check(nc_create(output_filename, NC_CLOBBER, &ncid));

  /* Add dimensions and coordinate variables. */ 
  add_dimension(ncid, "x", field->nx, &xdimid, &xid, "Distance east");
  add_dimension(ncid, "y", field->ny, &ydimid, &yid, "Distance north");
  add_dimension(ncid, "z", field->nz, &zdimid, &zid, "Height");
  dimids[0] = zdimid; dimids[1] = ydimid; dimids[2] = xdimid;

  /* Add scalar variables. */
  add_scalar_int(ncid, "seed", &seedid, "1",
		 "Seed for random number generator");
  add_scalar(ncid, "outer_scale", &outerscaleid, "m",
	     "Horizontal scale at which the power spectrum becomes flat");
  add_scalar(ncid, "vertical_exponent", &vertexponentid, "1",
	     "Exponent of power spectrum in the vertical");
  
  if (n_interp) {
    if (u_wind && v_wind && fall_speed) {
      add_scalar(ncid, "generating_level", &genlevelid, "m",
		 "Height from which the fallstreaks originate");
    }

    /* Add height-dependent variables */
    add_vector(ncid, "x_displacement", dimids[0], &deltaxid, "m",
	       "Eastward displacement of fallstreak relative to cloud top");
    add_vector(ncid, "y_displacement", dimids[0], &deltayid, "m",
	       "Northward displacement of fallstreak relative to cloud top");
    add_vector(ncid, "horizontal_exponent", dimids[0], &slopeid, "1",
	       "Exponent of power spectrum in the horizontal");
    add_vector(ncid, "mean", dimids[0], &meanid, units,
	       "Requested horizontal mean");
    if (is_lognormal) {
      add_vector(ncid, "standard_deviation", dimids[0], &stdid, "1",
		 "Requested fractional standard deviation");
    }
    else {
      add_vector(ncid, "standard_deviation", dimids[0], &stdid, units,
		 "Requested standard deviation");
    }
    if (u_wind && v_wind) {
      add_vector(ncid, "u_wind", dimids[0], &uwindid, "m s-1",
		 "Eastward wind");
      add_vector(ncid, "v_wind", dimids[0], &vwindid, "m s-1",
		 "Northward wind");
    }
    if (fall_speed) {
      add_vector(ncid, "fall_speed", dimids[0], &fallspeedid, "m s-1",
		 "Cloud particle fall speed");
    }

    if (is_size) {
      add_vector(ncid, "size_mean", dimids[0], &size_meanid, size_units,
		 "Requested horizontal mean of particle size");
      add_vector(ncid, "size_standard_deviation", dimids[0], &size_stdid, "1",
		 "Requested fractional standard deviation of particle size");
    }
  }

  /* Add the three-dimensional cloud field variable itself. */
  nc_check(nc_def_var(ncid, name, NC_FLOAT, 3, dimids, &fieldid));
  nc_check(nc_put_att_text(ncid, fieldid, "long_name",
			   strlen(long_name), long_name));
  nc_check(nc_put_att_text(ncid, fieldid, "units", strlen(units), units));
  if (is_threshold) {
    nc_check(NC_PUT_ATT_REAL(ncid, fieldid, "missing_value",
			      NC_FLOAT, 1, &missing_value));
    nc_check(NC_PUT_ATT_REAL(ncid, fieldid, "_FillValue",
			      NC_FLOAT, 1, &missing_value));
  }

  if (is_size) {
    nc_check(nc_def_var(ncid, size_name, NC_FLOAT, 3, dimids, &sizeid));
    nc_check(nc_put_att_text(ncid, sizeid, "long_name",
			   strlen(size_long_name), size_long_name));
    nc_check(nc_put_att_text(ncid, sizeid, "units", strlen(size_units), size_units));
    if (is_threshold) {
      nc_check(NC_PUT_ATT_REAL(ncid, sizeid, "missing_value",
				NC_FLOAT, 1, &missing_value));
      nc_check(NC_PUT_ATT_REAL(ncid, sizeid, "_FillValue",
				NC_FLOAT, 1, &missing_value));
    }
  }

  /* Global attributes. */
  nct_add_history(ncid, "Generated", user);
  nct_add_command_line(ncid, argc, argv);

  if (title) {
    nc_check(nc_put_att_text(ncid, NC_GLOBAL, "title",
			     strlen(title), title));
  }
  nc_check(nc_put_att_text(ncid, NC_GLOBAL, "source",
			   strlen(version), version));
  if (institution) {
    nc_check(nc_put_att_text(ncid, NC_GLOBAL, "institution",
			     strlen(institution), institution));
  }
  if (references) {
    nc_check(nc_put_att_text(ncid, NC_GLOBAL, "references",
			     strlen(references), references));
  }
  if (comment) {
    nc_check(nc_put_att_text(ncid, NC_GLOBAL, "comment",
			     strlen(comment), comment));
  }
  if ((confstring = rc_sprint(config))) {
    nc_check(nc_put_att_text(ncid, NC_GLOBAL, "config",
			     strlen(confstring), confstring));
  }

  /* Leave define mode. */
  nc_check(nc_enddef(ncid));

  /* Assign the coordinate variables. */
  NC_PUT_VAR_REAL(ncid, xid, field->x);
  NC_PUT_VAR_REAL(ncid, yid, field->y);
  NC_PUT_VAR_REAL(ncid, zid, field->z);

  /* Assign the scalars. */
  nc_put_var_int(ncid, seedid, &seed);
  NC_PUT_VAR_REAL(ncid, outerscaleid, &outer_scale);
  NC_PUT_VAR_REAL(ncid, vertexponentid, &vertical_exponent);
  if (n_interp) {
    if (u_wind && v_wind && fall_speed) {
      NC_PUT_VAR_REAL(ncid, genlevelid, &generating_level);
    }

    /* Assign the vectors. */
    NC_PUT_VAR_REAL(ncid, deltaxid, grid_x_displacement);
    NC_PUT_VAR_REAL(ncid, deltayid, grid_y_displacement);
    NC_PUT_VAR_REAL(ncid, slopeid, grid_horizontal_exponent);

    NC_PUT_VAR_REAL(ncid, meanid, grid_mean);
    NC_PUT_VAR_REAL(ncid, stdid, grid_std);

    if (is_size) {
      NC_PUT_VAR_REAL(ncid, size_meanid, grid_size_mean);
      NC_PUT_VAR_REAL(ncid, size_stdid, grid_size_std);
    }

    if (u_wind && v_wind) {
      NC_PUT_VAR_REAL(ncid, uwindid, grid_u_wind);
      NC_PUT_VAR_REAL(ncid, vwindid, grid_v_wind);
    }
    if (fall_speed) {
      NC_PUT_VAR_REAL(ncid, fallspeedid, grid_fall_speed);
    }
  }

  /* Assign the cloud field - note that this has to be done row by row
     because there are two dummy values in each. Should use
     cg_squeeze() first and write out the whole lot in one go... */
  count[2] = field->nx;
  for (k = 0; k < field->nz; k++) {
    start[0] = k;
    for (j = 0; j < field->nx; j++) {
      start[1] = j;
      nc_check(NC_PUT_VARA_REAL(ncid, fieldid, start, count,
		 field->field[0]+(field->nx+2)*(j + field->nx*k)));
    }
  }
  if (is_size) {
    for (k = 0; k < field->nz; k++) {
      start[0] = k;
      for (j = 0; j < field->nx; j++) {
	start[1] = j;
	nc_check(NC_PUT_VARA_REAL(ncid, sizeid, start, count,
		   field->field[1]+(field->nx+2)*(j + field->nx*k)));
      }
    }
  }

  /* Close file */
  nc_check(nc_close(ncid));

  return 0;
}
