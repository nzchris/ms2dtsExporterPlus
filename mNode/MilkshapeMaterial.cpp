
// disable MSVC++ warning about long debug symbol names
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "DTSUtil.h"
#include "MilkshapeMaterial.h"


MilkshapeMaterial::MilkshapeMaterial(U32 matIndex) : MilkshapeNode("material")
{
   mMatIndex = matIndex;
   mDetail = 0;
   mBump = 0;
   mReflectance = 0;
   mRefCount = 0;

   // set default properties
   setUserPropFloat("reflection", 0.f);
   setUserPropBool("NeverEnvMap", true);

   setUserPropBool("SWrap", MsDefaults::sWrap);
   setUserPropBool("TWrap", MsDefaults::tWrap);
   setUserPropBool("Translucent", MsDefaults::additive || MsDefaults::subtractive);
   setUserPropBool("Additive", MsDefaults::additive);
   setUserPropBool("Subtractive", MsDefaults::subtractive);
   setUserPropBool("SelfIlluminating", MsDefaults::selfIlluminating);
   setUserPropBool("NoMipMap", MsDefaults::noMipMap);
   setUserPropBool("MipMapZeroBorder", MsDefaults::mipMapZeroBorder);
   setUserPropFloat("detailScale", MsDefaults::detailScale);

   setUserPropInt("detail", -1);
   setUserPropInt("bump", -1);
   setUserPropInt("reflectance", -1);

   // get the texture name
   char textureName[MS_MAX_PATH+1];
   msMaterial_GetDiffuseTexture(getMsMaterial(), textureName, MS_MAX_PATH);
   if (!textureName[0])
   {
      msMaterial_GetName(getMsMaterial(), textureName, MS_MAX_PATH);

      // if no name, create one
      if (!textureName[0])
         strcpy(textureName, "Unnamed");
   }

   // strip texture file extension
   char *ptr = textureName + strlen(textureName);
   while (ptr > textureName && *ptr != '.' && *ptr != '\\')
      ptr--;
   if (*ptr == '.')
      *ptr = '\0';

   // strip texture file path
   mTextureName = DTS::getFileBase(textureName);

   // get material name
   char name[MS_MAX_PATH+1];
   msMaterial_GetName(getMsMaterial(), name, MS_MAX_PATH);
   mName = name;

   // read properties from comment string
   char commentString[MAX_COMMENT_LENGTH+1] = "";
   msMaterial_GetComment(getMsMaterial(), commentString, MAX_COMMENT_LENGTH);
   readPropertyString(commentString);
}
