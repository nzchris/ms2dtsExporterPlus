#ifndef _MILKSHAPEMATERIAL_H_
#define _MILKSHAPEMATERIAL_H_


#include "MilkshapeNode.h"


namespace MsDefaults
{
   // material settings
   const bool sWrap              = true;
   const bool tWrap              = true;
   const bool additive           = false;
   const bool subtractive        = false;
   const bool selfIlluminating   = false;
   const bool noMipMap           = false;
   const bool mipMapZeroBorder   = false;
   const float detailScale       = 1.f;
   const bool doubleSided        = false;
};


//--------------------------------------------------------------------------
/// Wrapper class for a milkshape material
class MilkshapeMaterial : public MilkshapeNode
{
   U32 mMatIndex;                   //!< Index of the milkshape material object
   std::string mTextureName;        //!< Name of this materials texture

public:
   int mRefCount;
   MilkshapeMaterial *mDetail;
   MilkshapeMaterial *mReflectance;
   MilkshapeMaterial *mBump;

   /// Material properties are stripped from the name during construction
   MilkshapeMaterial(U32 matIndex);

   bool isMaterial() const { return true; }

   /// Get the texture name for this material
   ///
   /// @return The name of the texture
   const char *getTextureName() const { return mTextureName.data(); }

   /// Get the milkshape material
   ///
   /// @return A pointer to the milkshape material
   msMaterial *getMsMaterial() const { return msModel_GetMaterialAt(mModel, mMatIndex); }
};

#endif // _MILKSHAPEMATERIAL_H_
