
// disable MSVC++ warning about long debug symbol names
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "msAppMesh.h"
#include "msAppNode.h"
#include "appConfig.h"
#include "appIfl.h"


namespace DTS
{
   /// Wrapper to retrieve a milkshape vertex
   ///
   /// @param mesh   Mesh to get vertex from
   /// @param idx    Index of the vertex to retrieve
   ///
   /// @return The x,y,z vertex
   Point3D getVert(msMesh *mesh, int idx)
   {
      msVertex *vtx = msMesh_GetVertexAt(mesh, idx);
      return Point3D(MilkshapePoint(vtx->Vertex));
   }

   /// Wrapper to retrieve a normal of a vertex of a milkshape triangle
   ///
   /// @param mesh       Mesh to get normal from
   /// @param triIndex   Index of the triangle to retrieve
   /// @param normIndex  Index of the normal to retrieve
   ///
   /// @return The u,v texture vertex
   Point3D getNormal(msMesh *mesh, int triIndex, int normIndex)
   {
      msTriangleEx *triEx = msMesh_GetTriangleExAt(mesh, triIndex);
      return Point3D(MilkshapePoint(triEx->Normals[normIndex]));
   }

   /// Wrapper to retrieve texture mapping from a vertex of a milkshape triangle
   ///
   /// @param mesh       Mesh to get vertex from
   /// @param triIndex   Index of the triangle to retrieve
   /// @param vertIndex  Index of the vertex to retrieve
   ///
   /// @return The u,v texture vertex
   Point2D getTVert(msMesh *mesh, int triIndex, int vertIndex)
   {
      msTriangleEx *triEx = msMesh_GetTriangleExAt(mesh, triIndex);
      return Point2D(triEx->TexCoords[vertIndex][0], triEx->TexCoords[vertIndex][1]);
   }

   //--------------------------------------------------------------------------

   MsAppMesh::MsAppMesh(MilkshapeNode *msNode, AppNode *appNode)
   {
      mAppNode = appNode;
      mMsNode = static_cast<MilkshapeMesh*>(msNode);
   }

   Matrix<4,4,F32> MsAppMesh::getMeshTransform(const AppTime & time)
   {
      assert(!mLocked && "Mesh is locked");

      S32 frame = (time.getF32() * AppConfig::AppFramesPerSec() + 0.5);
      return mMsNode->getMeshTransform(frame);
   }

   F32 MsAppMesh::getVisValue(const AppTime & time)
   {
      assert(!mLocked && "Mesh is locked");

      S32 frame = (time.getF32() * AppConfig::AppFramesPerSec() + 0.5f);
      return mMsNode->getVisibility(frame);
   }

	bool MsAppMesh::generateMaterial(MilkshapeMaterial *msMat, Material &mat)
	{
		if (!msMat)
			return false;

		// set defaults
		mat.name    = msMat->getTextureName();
		mat.flags   = 0;

		// read parameters from material
		msMat->getUserPropInt("detail", mat.detail);
		msMat->getUserPropInt("bump", mat.bump);
		msMat->getUserPropInt("reflectance", mat.reflectance);
		msMat->getUserPropFloat("detailScale", mat.detailScale);
		msMat->getUserPropFloat("reflection", mat.reflection);

		// set flags
		bool flag = false;
		msMat->getUserPropBool("SWrap", flag);
		if (flag)
			mat.flags |= Material::SWrap;

		flag = false;
		msMat->getUserPropBool("TWrap", flag);
		if (flag)
			mat.flags |= Material::TWrap;

		flag = false;
		msMat->getUserPropBool("NeverEnvMap", flag);
		if (flag)
			mat.flags |= Material::NeverEnvMap;

		flag = false;
		msMat->getUserPropBool("Translucent", flag);
		if (flag)
			mat.flags |= Material::Translucent;

		flag = false;
		msMat->getUserPropBool("Additive", flag);
		if (flag)
			mat.flags |= Material::Additive;

		flag = false;
		msMat->getUserPropBool("Subtractive", flag);
		if (flag)
			mat.flags |= Material::Subtractive;

		flag = false;
		msMat->getUserPropBool("SelfIlluminating", flag);
		if (flag)
			mat.flags |= Material::SelfIlluminating;

		flag = false;
		msMat->getUserPropBool("NoMipMap", flag);
		if (flag)
			mat.flags |= Material::NoMipMap;

		flag = false;
		msMat->getUserPropBool("MipMapZeroBorder", flag);
		if (flag)
			mat.flags |= Material::MipMapZeroBorder;

		return true;
	}

	bool MsAppMesh::getMaterial(S32 matIdx, Material &mat, U32 type)
	{
		// get material object
		MilkshapeMaterial *msMat = mMsNode->getMaterial();

		if (!msMat)
			return false;

		// generate material by type requested
		switch (type)
		{
			case Material::DetailMap:
				if (generateMaterial(msMat->mDetail, mat))
				{
					mat.flags |= Material::DetailMap;
					return true;
				}
				return false;

			case Material::BumpMap:
				if (generateMaterial(msMat->mBump, mat))
				{
					mat.flags |= Material::BumpMap;
					return true;
				}
				return false;

			case Material::ReflectanceMap:
				if (generateMaterial(msMat->mReflectance, mat))
				{
					mat.flags |= Material::ReflectanceMap;
					return true;
				}
				return false;

			default:
			{
				// generate regular material
				generateMaterial(msMat, mat);

				// if this is an ifl, then create the ifl material if it doesn't
				// exist and mark as ifl
				char *dot = strchr(mat.name.data(),'_');
				if (dot && !stricmp(dot+1,"ifl"))
				{
					*dot = '.';
					mat.flags |= Material::IFLMaterial;
					while (mIfls.size() <= matIdx)
						mIfls.push_back(NULL);
					if (!mIfls[matIdx])
						mIfls[matIdx] = new AppIfl(mat.name.c_str());
				}

				return true;
			}
		}
	}

   bool MsAppMesh::animatesFrame(const AppSequenceData & seqData)
   {
      assert(!mLocked && "Mesh is locked");

      /// Morph animation is not supported
      return false;
   }

   bool MsAppMesh::animatesMatFrame(const AppSequenceData & seqData)
   {
      assert(!mLocked && "Mesh is locked");

      /// UV animation is not supported
      return false;
   }

   AppMeshLock MsAppMesh::lockMesh(const AppTime & time, const Matrix<4,4,F32> &objectOffset)
   {
      msMesh *mesh = mMsNode->getMsMesh();

      assert(mesh && "NULL milkshape mesh");

      S32 lastMatIdx = -1;

      // start lists empty
      mFaces.clear();
      mVerts.clear();
      mTVerts.clear();
      mIndices.clear();
      mSmooth.clear();
      mVertId.clear();

      // start out with faces and crop data allocated
      mFaces.resize(msMesh_GetTriangleCount(mesh));

      S32 vCount = msMesh_GetVertexCount(mesh);

      // Transform the vertices by the bounds and scale
      std::vector<Point3D> verts(vCount, Point3D());

      for (int i = 0; i < vCount; i++)
         verts[i] = objectOffset * (getVert(mesh, i) * mMsNode->getScale());

      int numTriangles = mFaces.size();
      for (i = 0; i < numTriangles; i++)
      {
         msTriangle *msFace = msMesh_GetTriangleAt(mesh, i);
         Primitive &tsFace = mFaces[i];

         // set faces material index
         S32 matIndex = msMesh_GetMaterialIndex(mesh);
         tsFace.type = (matIndex >= 0) ? matIndex : Primitive::NoMaterial;
         tsFace.firstElement = mIndices.size();
         tsFace.numElements = 3;
         tsFace.type |= Primitive::Triangles | Primitive::Indexed;

         // set vertex indices
         word vertIndices[3];
         msTriangle_GetVertexIndices(msFace, vertIndices);

         Point3D vert0 = verts[vertIndices[0]];
         Point3D vert1 = verts[vertIndices[1]];
         Point3D vert2 = verts[vertIndices[2]];

         Point3D norm0 = getNormal(mesh, i, 0);
         Point3D norm1 = getNormal(mesh, i, 1);
         Point3D norm2 = getNormal(mesh, i, 2);

         // set texture vertex indices
         Point2D tvert0 = getTVert(mesh, i, 0);
         Point2D tvert1 = getTVert(mesh, i, 1);
         Point2D tvert2 = getTVert(mesh, i, 2);

         // now add indices (switch order to be CW)
         mIndices.push_back(addVertex(vert0,norm0,tvert0,vertIndices[0]));
         mIndices.push_back(addVertex(vert2,norm2,tvert2,vertIndices[2]));
         mIndices.push_back(addVertex(vert1,norm1,tvert1,vertIndices[1]));

         // if the material is double-sided, add the backface as well
         if (!(tsFace.type & Primitive::NoMaterial))
         {
            bool doubleSided = false;
            MilkshapeMaterial *msMat = mMsNode->getMaterial();
            msMat->getUserPropBool("doubleSided", doubleSided);

            if (doubleSided)
            {
               Primitive backface = tsFace;
               backface.firstElement = mIndices.size();
               mFaces.push_back(backface);

               // add verts with order reversed to get the backface
               mIndices.push_back(addVertex(vert0,-norm0,tvert0,vertIndices[0]));
               mIndices.push_back(addVertex(vert1,-norm1,tvert1,vertIndices[1]));
               mIndices.push_back(addVertex(vert2,-norm2,tvert2,vertIndices[2]));
            }
         }
      }

      return Parent::lockMesh(time,objectOffset);
   }

   void MsAppMesh::unlockMesh()
   {
      Parent::unlockMesh();

      // no more cleanup...but if there were some to do, we'd do it here
   }

   void MsAppMesh::getSkinData()
   {
      // only generate the skin data once
      if (mSkinDataFetched)
         return;
      mSkinDataFetched = true;

      // skinned meshes are attached to more than 1 bone
      if (mMsNode->mBoneIndices.size() <= 1)
         return;

      // need to generate an array of bones that animate this mesh
      // Milkshape supports up to 3 bone weights per vertex - all other bones
      // are set to weight 0
      msModel *model = mMsNode->getModel();

      //-----------------------------------------------------------------------
      // add all bones attached to this mesh
      S32 numBones = mMsNode->mBoneIndices.size();
      for (int i = 0; i < numBones;i++)
      {
         S32 boneIndex = mMsNode->mBoneIndices[i];

         assert(boneIndex >= 0 && "Invalid bone index");

         MilkshapeNode *node = new MilkshapeBone(boneIndex);
         mBones.push_back(new MsAppNode(node, true));
         AppConfig::PrintDump(PDPass2,avar("Adding skin object from skin \"%s\" to bone \"%s\" (%i).\r\n",mMsNode->getName(),mBones[i]->getName(),i));
      }

      //-----------------------------------------------------------------------
      // reset all weights to zero
      S32 numPoints = msMesh_GetVertexCount(mMsNode->getMsMesh());
      mWeights.resize(numBones);
      for (i = 0; i < mWeights.size(); i++)
      {
         mWeights[i] = new std::vector<F32>;
         mWeights[i]->resize(numPoints);
         for (int j=0; j<numPoints; j++)
            (*mWeights[i])[j] = 0.0f;
      }

      // set weights for bones that affect vertices in this mesh
      for (int j = 0; j < numPoints; j++)
      {
         S32 indices[MS_BONES_PER_VERTEX_EX];
         F32 weights[MS_BONES_PER_VERTEX_EX];
         bool attached = false;

         // get the bone indices/weights of this vertex - they are stored a bit
         // strangely to keep the milkshape file format backwards compatible.
         // the first bone is stored in a Vertex, the first weight is stored
         // in a VertexEx
         for (int k = 0; k < MS_BONES_PER_VERTEX_EX; k++)
         {
            msVertexEx *vtx = msMesh_GetVertexExAt(mMsNode->getMsMesh(), j);
            weights[k] = (F32)msVertexEx_GetBoneWeights(vtx, k) / 100.f;
            if (k == 0)
            {
               msVertex *v = msMesh_GetVertexAt(mMsNode->getMsMesh(), j);
               indices[0] = msVertex_GetBoneIndex(v);

               // set the weight to 1 if the VertexEx is not used
               if ((indices[0] >= 0) && (weights[0] == 0.0f))
                  weights[0] = 1.0f;
            }
            else
            {
               indices[k] = msVertexEx_GetBoneIndices(vtx, k - 1);
            }
            if (indices[k] >= 0)
               attached = true;
         }

         // if the vertex is not attached to any bones, attach it to the root bone
         if (!attached)
         {
            S32 boneIndex = mMsNode->getRootBoneIndex();
            addBoneWeight(j, boneIndex, 1.0f);
         }
         else
         {
            // add weights for each bone
            for (int k = 0; k < MS_BONES_PER_VERTEX_EX; k++)
            {
               if (indices[k] < 0)
                  continue;
               addBoneWeight(j, indices[k], weights[k]);
            }
         }
      }
   }

   void MsAppMesh::addBoneWeight(S32 vertIndex, S32 boneIndex, F32 weight)
   {
      // find bone
      for (int i = 0; i < mMsNode->mBoneIndices.size(); i++)
         if (boneIndex == mMsNode->mBoneIndices[i])
            break;

      assert(i != mMsNode->mBoneIndices.size() && "Could not find bone");

      boneIndex = i;
      AppConfig::PrintDump(-1, avar("Adding weight %f for bone %i (\"%s\")\r\n",
                           weight, boneIndex, mBones[boneIndex]->getName()));

      (*mWeights[boneIndex])[vertIndex] = weight;
   }

} // namespace DTS
