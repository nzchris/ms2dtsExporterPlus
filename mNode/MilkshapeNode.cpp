
// disable MSVC++ warning about long debug symbol names
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <algorithm>

#include <cmath>
#include "DTSUtil.h"
#include "appSequence.h"
#include "MilkshapeNode.h"

using namespace std;
using namespace DTS;


// shared milkshape model pointer
msModel *MilkshapeNode::mModel = NULL;
F32 MilkshapeNode::mScale = MsDefaults::modelScale;


MilkshapeNode::MilkshapeNode(const char *name)
{
   mName = name ? name : "Unnamed";
   mParent = NULL;
}


//--------------------------------------------------------------------------
void msMaterial_Destroy(msModel *pModel, msMaterial *pMaterial)
{
   int matIndex = pMaterial - pModel->pMaterials;
   int materialCount = msModel_GetMaterialCount(pModel);

   assert(matIndex >= 0 && matIndex < materialCount && "Invalid material index");

   // remove the material
   // @todo Do we need to free the comment string? For now, just set it empty.
   msMaterial_SetComment(pMaterial, "");

   // shift other materials in the array
   for (int i = matIndex; i < materialCount - 1; i++)
      memcpy(&pModel->pMaterials[i], &pModel->pMaterials[i+1], sizeof(msMaterial));

   pModel->nNumMaterials--;

   assert(materialCount > msModel_GetMaterialCount(pModel) && "Material not removed!");
}

//--------------------------------------------------------------------------
MilkshapeNode::~MilkshapeNode()
{
   // delete children
   for (int i = 0; i < mChildren.size(); i++)
      delete mChildren[i];
   mChildren.clear();
}

//--------------------------------------------------------------------------
void MilkshapeNode::addChildNode(MilkshapeNode *child)
{
   mChildren.push_back(child);
   child->setParentNode(this);
}

MilkshapeNode *MilkshapeNode::getChildNode(S32 idx) const
{
   assert((idx >= 0 && idx < mChildren.size()) && "Index out of range");
   return mChildren[idx];
}

Matrix<4,4,F32> MilkshapeNode::getNodeTransform(S32 frame) const
{
   return Matrix<4,4,F32>::identity();
}

//--------------------------------------------------------------------------
// @todo: This method is not safe. If there are too many properties, the comment
// string could be exceeded. Fix!
void MilkshapeNode::writePropertyString(char *buff, int maxLength) const
{
   // Print all user properties into the string as formatted text. This is by no
   // means the most space efficient method of storing the data, but it's far
   // easier to add and remove items, can be easily read back in, and there
   // really isn't that much data to store anyway.

   // each group of properties stores the number of properties in that group,
   // then each property as a "name=value" pair, separated by newlines.

   char *p = buff;

   // float properties
   p += sprintf(p, "%d\r\n", (int)mFloatMap.size());
   map<string, F32>::const_iterator itFloat = mFloatMap.begin();
   for (; itFloat != mFloatMap.end(); itFloat++)
      p += sprintf(p, "%s=%f\r\n", itFloat->first.c_str(), itFloat->second);

   // integer properties
   p += sprintf(p, "%d\r\n", (int)mIntMap.size());
   map<string, S32>::const_iterator itInt = mIntMap.begin();
   for (; itInt != mIntMap.end(); itInt++)
      p += sprintf(p, "%s=%d\r\n", itInt->first.c_str(), itInt->second);

   // boolean properties
   p += sprintf(p, "%d\r\n", (int)mBoolMap.size());
   map<string, bool>::const_iterator itBool = mBoolMap.begin();
   for (; itBool != mBoolMap.end(); itBool++)
      p += sprintf(p, "%s=%c\r\n", itBool->first.c_str(), itBool->second ? '1':'0');

   assert((p - buff) < maxLength && "Comment string exceeds max length!");
}

void MilkshapeNode::readPropertyString(char *buff)
{
   if(!buff || !strlen(buff))
      return;

   // each group of properties stores the number of properties in that group,
   // then each property as a "name=value" pair
   const char *delims = "=\r\n";

   // float properties
   int numProperties = atoi(strtok(buff, delims));
   for (int i = 0; i < numProperties; i++)
      mFloatMap[strtok(NULL, delims)] = atof(strtok(NULL, delims));

   // integer properties
   numProperties = atoi(strtok(NULL, delims));
   for (i = 0; i < numProperties; i++)
      mIntMap[strtok(NULL, delims)] = atoi(strtok(NULL, delims));

   // boolean properties
   numProperties = atoi(strtok(NULL, delims));
   for (i = 0; i < numProperties; i++)
      mBoolMap[strtok(NULL, delims)] = (*strtok(NULL, delims) == '1');
}

//--------------------------------------------------------------------------
bool MilkshapeNode::getUserPropFloat(const char *propName, F32 &val) const
{
   map<string, F32>::const_iterator it = mFloatMap.find(propName);
   if (it != mFloatMap.end())
   {
      val = it->second;
      return true;
   }
   else
      return false;
}

bool MilkshapeNode::getUserPropInt(const char *propName, S32 &val) const
{
   map<string, S32>::const_iterator it = mIntMap.find(propName);
   if (it != mIntMap.end())
   {
      val = it->second;
      return true;
   }
   else
      return false;
}

bool MilkshapeNode::getUserPropBool(const char *propName, bool &val) const
{
   map<string, bool>::const_iterator it = mBoolMap.find(propName);
   if (it != mBoolMap.end())
   {
      val = it->second;
      return true;
   }
   else
      return false;
}
