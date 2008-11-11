
// disable MSVC++ warning about long debug symbol names
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "msConfig.h"
#include "ms2DtsExporterPlus.h"


namespace DTS
{

MsConfig::MsConfig()
{
   // provide some defaults that better suit milkshape
   mEnableSequences = MsDefaults::exportAnimation;
   mExportOptimized = true;
   mAllowUnusedMeshes = true;
   mAllowCollapseTransform = false;
   mNoMipMap = false;
   mNoMipMapTranslucent = false;
   mZapBorder = true;
   mAnimationDelta = 0.0001f;
   mSameVertTOL = 0.00005f;
   mSameNormTOL = 0.005f;
   mSameTVertTOL = 0.00005f;
   mWeightThreshhold = 0.001f;
   mWeightsPerVertex = 3;
   mCyclicSequencePadding = 0;
   mAppFramesPerSec = 30.0f;
   mDumpConfig = 0xFFFFFFFF;
   mErrorString = NULL;

   clearConfigLists();

   // never export milkshape dummy nodes
   mNeverExport.push_back(strnew("base01"));
   mNeverExport.push_back(strnew("start01"));

   setupConfigParams();
}

}; // namespace DTS
