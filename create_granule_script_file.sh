ls -1 /media/sf_G_DRIVE/data/AMSR2/L1R/2012_10/GW*???_???D_*.h5 > temp_listing.txt #|awk -F"/" '{print $NF}'
#ls -1 /media/sf_G_DRIVE/data/AMSR2/L1R/2016_1*/GW*???_???D_*.h5 >> temp_listing.txt
#ls -1 /media/sf_G_DRIVE/data/AMSR2/L1R/2017_01/GW*???_???D_*.h5 >> temp_listing.txt
#ls -1 /media/sf_G_DRIVE/data/AMSR2/L1R/2017_02/GW*???_???D_*.h5 >> temp_listing.txt
#ls -1 /media/sf_G_DRIVE/data/AMSR2/L1R/2017_03/GW*???_???D_*.h5 >> temp_listing.txt
#ls -1 /media/sf_G_DRIVE/data/AMSR2/L1R/2017_04/GW*???_???D_*.h5 >> temp_listing.txt
#ls -1 /media/sf_G_DRIVE/data/AMSR2/L1R/2017_05/GW*???_???D_*.h5 >> temp_listing.txt
awk 'BEGIN{FS="/";}{print "./SD_SWE_SMSA_v2.0.2 /" $1 $2"/"$3"/"$4"/"$5"/"$6"/"$7"/"$8 " L2_test/" $8 ".test 1 2 3 4 5"}' temp_listing.txt > granule_script_test_Oct12.sh
rm -f temp_listing.txt
