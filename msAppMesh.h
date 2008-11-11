
#ifndef _MSAPPMESH_H_
#define _MSAPPMESH_H_


#include "./msLib/msLib.h"
#include "./msLib/msPlugIn.h"

#include "appMesh.h"
#include "appNode.h"

#include "MilkshapeMesh.h"
#include "MilkshapeMaterial.h"
#include "MilkshapeBone.h"


namespace DTS
{
   /// Wrapper for a milkshape mesh
   class MsAppMesh : public AppMesh
   {
      typedef AppMesh Parent;

   protected:

      MilkshapeMesh *mMsNode;       //!< Pointer to the milkshape mesh

      // Hold this in addition just for convenience of a few methods that
      // use app node methods
      AppNode *mAppNode;            //!< Pointer to the node that owns this mesh

      /// Generate the bone weights and vertex indices for all bones that affect
      /// this mesh.
      void getSkinData();

      ///
      bool generateMaterial(MilkshapeMaterial *msMat, Material &mat);

      /// Add a bone weighting
      /// @param  vertIndex   Index of the vertex
      /// @param  boneIndex   Index of the bone
      /// @param  weight      Bone weight
      void addBoneWeight(S32 vertIndex, S32 boneIndex, F32 weight);

   public:

      MsAppMesh(MilkshapeNode *msNode, AppNode *appNode);

      /// Get the name of this mesh
      ///
      /// @return A string containing the name of this mesh
      const char *getName() { return mAppNode->getName(); }

      //-----------------------------------------------------------------------

      /// Get a floating point property value
      ///
      /// @param propName     Name of the property to get
      /// @param defaultVal   Reference to variable to hold return value
      ///
      /// @return True if a value was set, false if not
      bool getFloat(const char *propName, F32 &defaultVal)
      {
         return mAppNode->getFloat(propName,defaultVal);
      }

      /// Get an integer property value
      ///
      /// @param propName     Name of the property to get
      /// @param defaultVal   Reference to variable to hold return value
      ///
      /// @return True if a value was set, false if not
      bool getInt(const char *propName, S32 &defaultVal)
      {
         return mAppNode->getInt(propName,defaultVal);
      }

      /// Get a boolean property value
      ///
      /// @param propName     Name of the property to get
      /// @param defaultVal   Reference to variable to hold return value
      ///
      /// @return True if a value was set, false if not
      bool getBool(const char *propName, bool &defaultVal)
      {
         return mAppNode->getBool(propName,defaultVal);
      }

      //-----------------------------------------------------------------------

      /// Sets up the material used by this mesh
      ///
      /// @param matIdx   Index of the material to get (index of the milkshape material attached to this mesh)
      /// @param mat      Material object to set up
      /// @param type     Type of material to create (normal, bump, detail etc)
      ///
      /// @return True if the material was set up correctly, false otherwise
      bool getMaterial(S32 matIdx, Material &mat, U32 type);

      /// Get the transform of this mesh at a certain time
      ///
      /// @param time   Time at which to get the transform
      ///
      /// @return The mesh transform at the specified time
      Matrix<4,4,F32> getMeshTransform(const AppTime &time);

      /// Get the visibility of this mesh at a certain time
      ///
      /// @param time   Time at which to get visibility info
      ///
      /// @return Visibility from 0 (invisible) to 1 (opaque)
      F32 getVisValue(const AppTime &time);

      /// Check if the material used by this mesh is animated
      ///
      /// @param seqData   Start/end time to check
      ///
      /// @return True if the material is animated, false if not
      bool animatesMatFrame(const AppSequenceData &seqData);

      /// Check if the mesh is animated
      ///
      /// @param seqData   Start/end time to check
      ///
      /// @return True if the mesh is animated, false if not
      bool animatesFrame(const AppSequenceData &seqData);

      /// Generate the vertex, normal and triangle data for the mesh.
      ///
      /// @param time           Time at which to generate the mesh data (ignored, as milkshape mesh data is only available at time=0)
      /// @param objectOffset   Transform to apply to the generated data (bounds transform)
      ///
      /// @return @todo Add description for this
      AppMeshLock lockMesh(const AppTime & time, const Matrix<4,4,F32> &objectOffset);

      /// Perform any application specific cleanup on the generated mesh
      void unlockMesh();
   };

};

#endif // MSAPPMESH_H_
