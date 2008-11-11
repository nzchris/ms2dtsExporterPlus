
// disable MSVC++ warning about long debug symbol names
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <algorithm>

#include "DTSUtil.h"
#include "MilkshapeMesh.h"
#include "MilkshapeBone.h"


using namespace DTS;


MilkshapeMesh::MilkshapeMesh(U32 meshIndex) : MilkshapeNode("mesh")
{
   mMeshIndex = meshIndex;
   mMat = 0;

   // set defaults
   setUserPropBool("BB", MsDefaults::bb);
   setUserPropBool("BBZ", MsDefaults::bbz);

   setUserPropBool("sorted", MsDefaults::sorted);
   setUserPropBool("SORT::Z_LAYER_UP", MsDefaults::zUp);
   setUserPropBool("SORT::Z_LAYER_DOWN", MsDefaults::zDown);
   setUserPropInt("SORT::NUM_BIG_FACES", MsDefaults::numBigFaces);
   setUserPropInt("SORT::MAX_DEPTH", MsDefaults::maxDepth);

   setUserPropInt("lod", MsDefaults::lod);

   setUserPropInt("numVisFrames", 0);
   setUserPropInt("numAutoDetails", 0);

   setUserPropBool("autoBillboard", false);
   setUserPropInt("autoBillboardSize", MsDefaults::autoBillboardSize);
   setUserPropInt("BB::EQUATOR_STEPS", MsDefaults::numEquatorSteps);
   setUserPropInt("BB::POLAR_STEPS", MsDefaults::numPolarSteps);
   setUserPropInt("BB::DL", MsDefaults::dl);
   setUserPropInt("BB::DIM", MsDefaults::dim);
   setUserPropBool("BB::INCLUDE_POLES", MsDefaults::includePoles);
   setUserPropFloat("BB::POLAR_ANGLE", MsDefaults::polarAngle);

   // read properties from comment string
   char commentString[MAX_COMMENT_LENGTH+1] = "";
   msMesh_GetComment(getMsMesh(), commentString, MAX_COMMENT_LENGTH);
   readPropertyString(commentString);

   // get mesh name and lod
   int lod;
   char meshName[MS_MAX_PATH+1];
   msMesh_GetName(getMsMesh(), meshName, MS_MAX_PATH);
   char *p = chopTrailingNumber(meshName, lod);
   if (p && (strlen(p) != strlen(meshName)))
      setUserPropInt("lod", lod);
   mName = p;
   delete [] p;
}

//--------------------------------------------------------------------------
bool MilkshapeMesh::isEqual(const MilkshapeNode *node) const
{
   const MilkshapeMesh *meshNode = dynamic_cast<const MilkshapeMesh*>(node);
   return (meshNode && (getMsMesh() == meshNode->getMsMesh()));
}

//--------------------------------------------------------------------------
S32 MilkshapeMesh::getRootBoneIndex() const
{
   // find the root bone index
   S32 rootBone = msModel_FindBoneByName(mModel, "root");
   if (rootBone < 0)
      rootBone = msModel_FindBoneByName(mModel, "Root");

   assert(rootBone >= 0 && "Could not find root bone");

   return rootBone;
}

void MilkshapeMesh::setBoneIndices()
{
   S32 rootBone = getRootBoneIndex();

   // store the indices of which bones the vertices in this
   // mesh are attached to
   mBoneIndices.clear();
   S32 numVerts = msMesh_GetVertexCount(getMsMesh());
   for (int j = 0; j < numVerts; j++)
   {
      S32 indices[MS_BONES_PER_VERTEX_EX];
      bool attached = false;

      // get the bone indices of this vertex - they are stored a bit
      // strangely to keep the milkshape file format backwards compatible.
      // the first bone is stored in a Vertex, the first weight is stored
      // in a VertexEx
      for (int k = 0; k < MS_BONES_PER_VERTEX_EX; k++)
      {
         if (k == 0)
         {
            msVertex *vtx = msMesh_GetVertexAt(getMsMesh(), j);
            indices[0] = msVertex_GetBoneIndex(vtx);
         }
         else
         {
            msVertexEx *vtx = msMesh_GetVertexExAt(getMsMesh(), j);
            indices[k] = msVertexEx_GetBoneIndices(vtx, k - 1);
         }
         if (indices[k] >= 0)
            attached = true;
      }

      // if the vertex is not attached to any bones, attach it to the root bone
      if (!attached)
      {
         // attach to root bone
         if (std::find(mBoneIndices.begin(), mBoneIndices.end(), rootBone) == mBoneIndices.end())
            mBoneIndices.push_back(rootBone);
      }
      else
      {
         // get attached bones
         for (k = 0; k < MS_BONES_PER_VERTEX_EX; k++)
         {
            S32 boneIndex = indices[k];
            if (boneIndex < 0)
               continue;

            // check if this bone has already been added
            if (std::find(mBoneIndices.begin(), mBoneIndices.end(), boneIndex) == mBoneIndices.end())
               mBoneIndices.push_back(boneIndex);
         }
      }
   }

   // mesh must be attached to at least one bone
   assert(mBoneIndices.size() > 0 && "Mesh failed to attach to the root bone");

   // rigid meshes have all their vertices attached to a single bone,
   // ie mBoneIndices.size() == 1

   // skinned meshes do not have all their vertices attached to a
   // single bone. ie mBoneIndices.size() > 1
}

//--------------------------------------------------------------------------
Matrix<4,4,F32> MilkshapeMesh::getMeshTransform(S32 frame) const
{
   /// @todo Is getMeshTransform the same as MilkshapeMesh::getNodeTransform?
   return MilkshapeNode::getNodeTransform(frame);
}

Matrix<4,4,F32> MilkshapeMesh::getNodeTransform(S32 frame) const
{
   // for rigid meshes, return the bone transform
   if ((mBoneIndices.size() == 1) && (mBoneIndices[0] >= 0))
   {
      // create a temporary bone object to retrieve the transform
      MilkshapeBone temp(mBoneIndices[0]);
      return temp.getNodeTransform(frame);
   }
   else
   {
      // for skinned meshes, return the node transform
      return MilkshapeNode::getNodeTransform(frame);
   }
}

//--------------------------------------------------------------------------
void MilkshapeMesh::clearAutoDetails()
{
   int numAutoDetails = 0;
   getUserPropInt("numAutoDetails", numAutoDetails);
   for (int i = 0; i < numAutoDetails; i++)
   {
      mIntMap.erase(DTS::avar("autoDetailSize%d", i));
      mFloatMap.erase(DTS::avar("autoDetailPercent%d", i));
   }
   setUserPropInt("numAutoDetails", 0);
}

//--------------------------------------------------------------------------
void MilkshapeMesh::clearVisFrames()
{
   int numFrames = 0;
   getUserPropInt("numVisFrames", numFrames);
   for (int i = 0; i < numFrames; i++)
   {
      mIntMap.erase(DTS::avar("visFrame%d", i));
      mFloatMap.erase(DTS::avar("visAlpha%d", i));
   }
   setUserPropInt("numVisFrames", 0);
}

F32 MilkshapeMesh::getVisibility(S32 frame) const
{
   int numFrames = 0;
   getUserPropInt("numVisFrames", numFrames);

   if (numFrames == 0)
      return 1.f;

   // get visibility frames and alpha values
   int *visFrames = new int[numFrames];
   F32 *visAlpha = new F32[numFrames];
   for (int i = 0;i < numFrames; i++)
   {
      visFrames[i] = 1;
      visAlpha[i] = 1.f;
      getUserPropInt(DTS::avar("visFrame%d", i), visFrames[i]);
      getUserPropFloat(DTS::avar("visAlpha%d", i), visAlpha[i]);
   }

   F32 alpha = 1.f;

   // check for outside bounds
   if ((numFrames == 1) || (frame <= visFrames[0]))
   {
      alpha = visAlpha[0];
   }
   else if (frame >= visFrames[numFrames-1])
   {
      alpha = visAlpha[numFrames-1];
   }
   else
   {
      // find visiblity at specified frame by interpolating between keyframes
      // either side
      float a1,a2,dt;

      for (int idx = 1; idx < numFrames; idx++)
         if (visFrames[idx] > frame)
            break;

      a1 = visAlpha[idx-1];
      a2 = visAlpha[idx];
      if ((frame == visFrames[idx-1]) || (visFrames[idx] == visFrames[idx-1]))
         dt = 0;
      else
         dt = ((F32)(frame - visFrames[idx-1])) / (visFrames[idx] - visFrames[idx-1]);

      alpha = a1 + (a2 - a1) * dt;
   }

   delete [] visFrames;
   delete [] visAlpha;

   return alpha;
}
