# This project aimed to revaluate the time require of different algorithms.
The algorithms below uesd in this project,incloud the Planchon and Darbourx algorithm (P&D), the Wang and Liu algorithm (W&L), the Zhou et al.(2017) algorithm (Zhou), the Barnes et al.(2014) algorithm (Barnes), the Bai et al.(2015) algorithm (Bai), and the Wei et al.(2019) algorithm (Wei)

This project supports floating-point GeoTIFF file format through the GDAL library. Please include GDAL library into your compilation. GDAL 1.91 was used in our experiments.

The algorithms described in the manuscript can be run using the following command line:

FillDEM AlgorithmName Input_DEM

Example: FillDEM zhou-onepass dem.tif. Use the one-pass implementation proposed in the manuscript to fill the input DEM "dem.tif" and create a filled DEM "dem_filled_zhou-onepass.tif".

The algorithms available are described briefly below and in greater detail in the manuscript.

wang: Use the variant in Wang and Liu (2006) to fill the DEM. The name of the filled dem is ended with "_filled_wang".

barnes: Use the variant in Barnes et al. (2014) to fill the DEM. The name of the filled dem is ended with "_filled_barnes".

zhou-twopass: Use the two-pass implementation proposed in the manuscript to fill the DEM. The name of the filled dem is ended with "_filled_zhou-twopass".

zhou-onepass: Use the one-pass implementation proposed in the manuscript to fill the DEM. The name of the filled dem is ended with "_filled_zhou-onepass".

zhou-direct: Use the direct implementation proposed in the manuscript to fill the DEM. The name of the filled dem is ended with "_filled_zhou-direct".

The test data used in the manuscript can be downloaded at http://www.mngeo.state.mn.us/. You need ArcGIS to convert the DEM into GeoTIFF format.
