# Milkshape DTS Exporter Plus

[MilkShape](http://www.milkshape3d.com) is a 3D modelling program from chUmbaLum sOft oriented towards building art for games. MilkShape can be used to build 3D shapes for the Torque Game Engine. MilkShape is shareware and a free 30 day trial is available. It can be registered for $20. The exporter ms2dtsExporterPlus.dll is an extension to MilkShape which allows it to export the DTS files used by the Torque Game Engine. Note that you must have at least Milkshape version 1.8.3 installed to use this exporter.

To use the exporter, just copy ms2dtsExporterPlus.dll to your Milkshape install directory. The next time you start Milkshape, it will appear under the export list as "Torque DTS Plus...". You should also copy the ms2dtsplus.chm help file to the Milkshape install directory to allow access to the documentation directly from the exporter dialogs. The new exporter can co-exist with the old exporter (ms2dtsExporter.dll). It should operate similarly to the original, and most models can be exported with very few changes. You can now view and edit meshes, materials and sequences before you export them. Changes will be applied to the model unless you hit Cancel.

This exporter is not yet completed. Some features are still missing, and it is possible that there are problems with those features that are implemented. If you find an error, you can help the development of thi tool by providing a description of the problem, and if possible, the .ms3d, .dts, and dump.html files involved. Some features are listed as *UNTESTED*, these may or may not be 100% functional.

Chris Robertson
