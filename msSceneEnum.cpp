
// disable MSVC++ warning about long debug symbol names
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <algorithm>

#include "msSceneEnum.h"
#include "msAppNode.h"


namespace DTS
{
   MsSceneEnum::MsSceneEnum(Ms2dtsExporterPlus *exp)
   {
      mExporter = exp;
   }

   int MsSceneEnum::addDetail(MilkshapeNode *start01, const char *detailName)
   {
      // check if this detail already exists
      int numChildNodes = start01->getNumChildren();
      for (int node = 0; node < numChildNodes; node++)
      {
         if (strcmp(start01->getChildNode(node)->getName(), detailName) == 0)
            return node;
      }

      // add new detail level marker
      start01->addChildNode(new MilkshapeNode(detailName));
      return numChildNodes;
   }

   void MsSceneEnum::enumScene()
   {
      // add a 'node' for each milkshape object (sequences, meshes, bones)

      // create some helper subtree nodes to organise the shape hierarchy
      // - bones are added to the 2nd level dummy node ('base01')
      // - detail levels are added to the top level dummy node ('start01')
      // - rigid meshes are added to the bone to which they are attached
      // - skinned meshes and sequences are added to the root node
      MilkshapeNode *start01 = new MilkshapeNode("start01");
      MilkshapeNode *base01 = new MilkshapeNode("base01");
      start01->addChildNode(base01);

      // add bones as children of dummy node

      // First add all bones to a temporary list, then setup parent-child
      // relationships. This removes the requirement for parent bones to exist
      // in the milkshape model before child bones, and allows bones to be
      // easily indexed by rigid meshes.
      S32 numBones = msModel_GetBoneCount(mExporter->mModel);
      std::vector<MilkshapeBone*> tempBones;
      for (int i = 0; i < numBones; i++)
         tempBones.push_back(new MilkshapeBone(i));

      for (i = 0; i < numBones; i++)
      {
         // find parent bone
         char boneName[MS_MAX_NAME+1];
         msBone *bone = msModel_GetBoneAt(mExporter->mModel, i);
         msBone_GetParentName(bone, boneName, MS_MAX_NAME);
         int parent = msModel_FindBoneByName(mExporter->mModel, boneName);

         if (parent >= 0)
         {
            assert(parent < tempBones.size() && "Invalid parent bone index");
            tempBones[parent]->addChildNode(tempBones[i]);
         }
         else
            base01->addChildNode(tempBones[i]);
      }

      // add meshes
      for (i = 0; i < mExporter->mMeshes.size(); i++)
      {
         MilkshapeNode *mesh = mExporter->mMeshes[i];

         // bounding box mesh?
         if (strStartsWith(mesh->getName(), "Bounds"))
         {
            // bounds are added to root node
            MsAppNode *msNode = new MsAppNode(mesh);
            if (!processNode(msNode))
            {
               delete msNode;
            }
            continue;
         }

         int lod = -1;
         char detailName[MS_MAX_NAME+1];
         char buff[MS_MAX_PATH+1];

         // get detail level
         mesh->getUserPropInt("lod",lod);

         if (strStartsWith(mesh->getName(), "Collision"))
         {
            // Collision Mesh

            // construct mesh name
            sprintf(buff, "Col%d", lod);
            mesh->setName(buff);

            // construct detail level name
            sprintf(detailName, "Collision%d", lod);
         }
         else if (strStartsWith(mesh->getName(), "LOSCol"))
         {
            // Line Of Sight Collision Mesh

            // construct mesh name
            sprintf(buff, "LOSCol%d", lod);
            mesh->setName(buff);

            // construct detail level name
            sprintf(detailName, "LOS%d", lod);
         }
         else
         {
            // Normal Mesh

            // construct mesh name
            bool sorted,bb,bbz;
            mesh->getUserPropBool("sorted",sorted);
            mesh->getUserPropBool("BB",bb);
            mesh->getUserPropBool("BBZ",bbz);
            sprintf(buff,"%s%s%d",  sorted?"SORT::":
                                    bb?"BB::":
                                    bbz?"BBZ::":"",
                                    mesh->getName(),lod);
            mesh->setName(buff);

            // construct detail level name
            sprintf(detailName, "detail%d", lod);
         }

         if (static_cast<MilkshapeMesh*>(mesh)->mBoneIndices.size() > 1)
         {
            // skinned meshes are added to the root node
            MsAppNode *msNode = new MsAppNode(mesh);
            if (!processNode(msNode))
            {
               delete msNode;
               continue;
            }
         }
         else
         {
            // rigid meshes are added to bone they are attached to
            int boneIndex = static_cast<MilkshapeMesh*>(mesh)->mBoneIndices[0];
            tempBones[boneIndex]->addChildNode(mesh);
         }

         // add the detail level for the mesh
         addDetail(start01, detailName);

         // add auto-detail levels
         int numAutoDetails = 0;
         mesh->getUserPropInt("numAutoDetails", numAutoDetails);
         for (int j = 0; j < numAutoDetails; j++)
         {
            int size;
            mesh->getUserPropInt(avar("autoDetailSize%d", j), size);
            addDetail(start01, avar("detail%d", size));
         }

         // add auto-billboard detail node
         bool autoBB = false;
         mesh->getUserPropBool("autoBillboard", autoBB);
         if (autoBB)
         {
            // ensure node exists for auto-billboard detail level
            S32 size;
            mesh->getUserPropInt("autoBillboardSize", size);
            S32 nodeIndex = addDetail(start01, avar("BB::detail%d", size));

            // copy properties from mesh to the auto-billboard detail node
            char buff[1024+1];
            mesh->writePropertyString(buff, sizeof(buff)-1);
            start01->getChildNode(nodeIndex)->readPropertyString(buff);
         }
      }

      // add sequences
      for (i = 0; i < mExporter->mSequences.size(); i++)
      {
         // sequence properties are extracted from name
         MilkshapeNode *seq = mExporter->mSequences[i];

         // construct sequence name
         seq->setName(DTS::avar("Sequence::%s", seq->getName()));

         // sequences are added to the root node
         MsAppNode *msNode = new MsAppNode(seq);
         if (!processNode(msNode))
            delete msNode;
      }

      // add main subtree to root node
      MsAppNode *msNode = new MsAppNode(start01, true);
      if (!processNode(msNode))
      {
         delete start01;
         delete msNode;
      }
   }

}; // namespace DTS

