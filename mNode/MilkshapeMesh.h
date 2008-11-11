#ifndef _MILKSHAPEMESH_H_
#define _MILKSHAPEMESH_H_


#include "MilkshapeNode.h"


#define MS_BONES_PER_VERTEX_EX   3 // SDK 1.7.7 supports up to 3 weights per vertex


namespace MsDefaults
{
   // mesh settings
   const S32 lod                 = 0;
   const bool sorted             = false;
   const bool bb                 = false;
   const bool bbz                = false;
   const bool zUp                = false;
   const bool zDown              = false;
   const S32 numBigFaces         = 4;
   const S32 maxDepth            = 2;
   const S32 autoBillboardSize   = 2;
   const S32 numEquatorSteps     = 4;
   const S32 numPolarSteps       = 0;
   const S32 dl                  = 0;
   const S32 dim                 = 64;
   const bool includePoles       = true;
   const F32 polarAngle          = 0;
};

class MilkshapeMaterial;

//--------------------------------------------------------------------------
/// Wrapper class for a milkshape mesh
class MilkshapeMesh : public MilkshapeNode
{
   MilkshapeMaterial *mMat;         //!< The material for this mesh
   U32 mMeshIndex;                  //!< Index of the msModel mesh object

public:
   enum
   {
      MaxVisFrames = 256            //!< Maximum number of visibility keyframes
   };

   MilkshapeMesh(U32 meshIndex);

   msMesh *getMsMesh() const { return msModel_GetMeshAt(mModel, mMeshIndex); }

   bool isEqual(const MilkshapeNode *node) const;
   bool isMesh() const { return true; }

   /// Set the material attached to this mesh
   ///
   /// @param mat   The material to attach to this mesh
   void setMaterial(MilkshapeMaterial *mat) { mMat = mat; }

   /// Get the material attached to this mesh
   ///
   /// @return A pointer to the material object
   MilkshapeMaterial *getMaterial() const { return mMat; }

   /// Get the index of the root bone in the milkshape model.
   ///
   /// @return The index of the root bone.
   S32 getRootBoneIndex() const;

   /// Determine which bones (if any) this mesh is attached to and store their
   /// indices. If any vertices are not attached to a bone, they will be
   /// attached to the Root bone.
   void setBoneIndices();

   /// Get the mesh transform
   ///
   /// @param frame   Index of the frame at which to retrieve the transform
   ///
   /// @return The mesh transform at the given frame
   DTS::Matrix<4,4,F32> getMeshTransform(S32 frame) const;

   /// Remove all auto-details
   void clearAutoDetails();

   /// Remove all visibility keyframes
   void clearVisFrames();

   // overridden methods
   DTS::Matrix<4,4,F32> getNodeTransform(S32 frame) const;
   F32 getVisibility(S32 frame) const;

   //--------------------------------------------------------------------------

   std::vector<S32> mBoneIndices;   //!< Indices of bones attached to this mesh
};

#endif // _MILKSHAPEMESH_H_
