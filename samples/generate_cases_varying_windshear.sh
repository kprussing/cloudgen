#!/bin/bash

file=19990624.dat
DATE=$(echo $file | cut -c 1-8)
z_offset=6500
z_domain_size=3000
for shear in 0.001 0.5 1 1.5 3 4.5 6 9
do
    ../cloudgen $file verbose=1 wind_scale_factor=$shear output_filename=iwc${DATE}_$shear.nc \
	z_domain_size=$z_domain_size z_offset=$z_offset x_pixels=64 x_domain_size=100000
done

file=19990717.dat
DATE=$(echo $file | cut -c 1-8)
z_offset=6000
z_domain_size=5000
for shear in 0.001 0.5 1 2 4 6 8 11
do
    ../cloudgen $file verbose=1 wind_scale_factor=$shear output_filename=iwc${DATE}_$shear.nc \
	z_domain_size=$z_domain_size z_offset=$z_offset x_pixels=64 x_domain_size=100000
done

file=19990827.dat
DATE=$(echo $file | cut -c 1-8)
z_offset=5000
z_domain_size=5000
for shear in 0.001 0.4 1 2 3 4.5 6
do
    ../cloudgen $file verbose=1 wind_scale_factor=$shear output_filename=iwc${DATE}_$shear.nc \
	z_domain_size=$z_domain_size z_offset=$z_offset x_pixels=64 x_domain_size=100000
done

file=19991227.dat
DATE=$(echo $file | cut -c 1-8)
for shear in 0.001 0.15 0.3 0.6 1 1.5 2
do
    ../cloudgen $file verbose=1 wind_scale_factor=$shear output_filename=iwc${DATE}_$shear.nc \
			 x_pixels=64 x_domain_size=100000
done

