#!/bin/bash



file=19990624_nomixing.dat
DATE=$(echo $file | cut -c 1-8)
z_offset=6500
z_domain_size=3000
for shear in 0.001 1
do
    ../cloudgen $file verbose=1 wind_scale_factor=$shear output_filename=iwc${DATE}_nomixing_$shear.nc \
	z_domain_size=$z_domain_size z_offset=$z_offset x_pixels=64 x_domain_size=100000
done

#file=19990717_nomixing.dat
#DATE=$(echo $file | cut -c 1-8)
#z_offset=6000
#z_domain_size=5000
#for shear in 0.001 1
#do
#    ../cloudgen $file verbose=1 wind_scale_factor=$shear output_filename=iwc${DATE}_nomixing_$shear.nc \
#	z_domain_size=$z_domain_size z_offset=$z_offset x_pixels=64 x_domain_size=100000
#done

file=19990827_nomixing.dat
DATE=$(echo $file | cut -c 1-8)
z_offset=5000
z_domain_size=5000
for shear in 0.001 1
do
    ../cloudgen $file verbose=1 wind_scale_factor=$shear output_filename=iwc${DATE}_nomixing_$shear.nc \
	z_domain_size=$z_domain_size z_offset=$z_offset x_pixels=64 x_domain_size=100000
done

file=19991227_nomixing.dat
DATE=$(echo $file | cut -c 1-8)
for shear in 0.001 1
do
    ../cloudgen $file verbose=1 wind_scale_factor=$shear output_filename=iwc${DATE}_nomixing_$shear.nc \
			 x_pixels=64 x_domain_size=100000
done

