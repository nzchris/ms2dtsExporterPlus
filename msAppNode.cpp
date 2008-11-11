
// disable MSVC++ warning about long debug symbol names
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include "appConfig.h"

#include "msAppNode.h"
#include "msAppMesh.h"
#include "DTSUtil.h"


namespace DTS
{

   MsAppNode::MsAppNode(MilkshapeNode *node, bool owner)
   {
      mMsNode = node;
      mOwnsNode = owner;
   }

   MsAppNode::~MsAppNode()
   {
      // only delete milkshape nodes that we own...
      if(mOwnsNode)
         delete mMsNode;
   }

   void MsAppNode::buildMeshList()
   {
      if(mMsNode->isMesh())
         mMeshes.push_back(new MsAppMesh(mMsNode,this));
   }

   void MsAppNode::buildChildList()
   {
      for(int i=0;i<mMsNode->getNumChildren();i++)
      {
         MilkshapeNode *child = mMsNode->getChildNode(i);
         mChildNodes.push_back(new MsAppNode(child));
      }
   }

   bool MsAppNode::isEqual(AppNode *node)
   {
      MsAppNode *appNode = dynamic_cast<MsAppNode*>(node);
      if(appNode)
         return mMsNode->isEqual(appNode->mMsNode);
      else
         return false;
   }

   Matrix<4,4,F32> MsAppNode::getNodeTransform(const AppTime & time)
   {
      // convert from seconds to frames (round to nearest)
      S32 frame = (time.getF32() * AppConfig::AppFramesPerSec() + 0.5f);
      return mMsNode->getNodeTransform(frame);
   }

   bool MsAppNode::animatesTransform(const AppSequenceData & seqData)
   {
      // check if the node transform changes during the sequence interval
      // this method is only called for ground transforms, so use groundDelta
      Matrix<4,4,F32> base = getNodeTransform(seqData.startTime);
      for (AppTime t = seqData.startTime + seqData.groundDelta; t <= seqData.endTime; t += seqData.groundDelta)
      {
         if (getNodeTransform(t) != base)
            return true;
      }
      return false;
   }

   const char *MsAppNode::getName()
   {
      if(!mName)
         mName = strnew(mMsNode->getName());
      return mName;
   }

   const char *MsAppNode::getParentName()
   {
      if(!mParentName)
         mParentName = mMsNode->getParentNode() ? strnew(mMsNode->getParentNode()->getName()) : strnew("ROOT");
      return mParentName;
   }

   bool MsAppNode::isParentRoot()
   {
      return (mMsNode->getParentNode() == NULL) || mMsNode->getParentNode()->isRootNode();
   }

   bool MsAppNode::getFloat(const char *propName, F32 &defaultVal)
   {
      return mMsNode->getUserPropFloat(propName,defaultVal);
   }

   bool MsAppNode::getInt(const char *propName, S32 &defaultVal)
   {
      return mMsNode->getUserPropInt(propName,defaultVal);
   }

   bool  MsAppNode::getBool(const char *propName, bool &defaultVal)
   {
      return mMsNode->getUserPropBool(propName,defaultVal);
   }
};
